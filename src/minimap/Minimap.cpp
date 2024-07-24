#include "Minimap.h"
#include <minecraft/src/common/world/level/dimension/Dimension.hpp>
#include <minecraft/src/common/world/level/block/BlockLegacy.hpp>
#include <minecraft/src/common/world/level/BlockSource.hpp>
#include <minecraft/src-client/common/client/renderer/Tessellator.hpp>
#include <minecraft/src-client/common/client/game/ClientInstance.hpp>
#include <minecraft/src-client/common/client/player/LocalPlayer.hpp>
#include <minecraft/src-client/common/client/gui/gui/GuiData.hpp>
#include <amethyst/runtime/ModContext.hpp>

Vec3 vertexes[4] = {
    Vec3(0.0f, 0.0f, 0.0f),
    Vec3(0.0f, 1.0f, 0.0f),
    Vec3(1.0f, 1.0f, 0.0f),
    Vec3(1.0f, 0.0f, 0.0f),
};

Minimap::Minimap(MinecraftUIRenderContext& ctx)
    : mOutlineNineslice(30, 30, 10, 10), mTes(ctx.mScreenContext->tessellator)
{
    mMinimapMaterial = reinterpret_cast<mce::MaterialPtr*>(SlideAddress(0x59BD7E0));

    mMinimapEdgeBorder = 3.0f;
    mMinimapSize = 128.0f;

    /* Load any necessary textures. */
    mMinimapOutline = ctx.getTexture("textures/ui/minimap_border", true);
    mMinimapPosIcon = ctx.getTexture("textures/ui/minimap_pos_icon", true);
}

int countBlockNeighbors(const BlockSource& region, int xPos, int yPos, int zPos)
{
    int count = 0;

    const Block* block = &region.getBlock(xPos - 1, yPos, zPos);
    if (block->mLegacyBlock->mID != 0) count += 1;

    block = &region.getBlock(xPos, yPos, zPos - 1);
    if (block->mLegacyBlock->mID != 0) count += 1;

    block = &region.getBlock(xPos + 1, yPos, zPos);
    if (block->mLegacyBlock->mID != 0) count += 1;

    block = &region.getBlock(xPos, yPos, zPos + 1);
    if (block->mLegacyBlock->mID != 0) count += 1;

    return count;
}

mce::Color Minimap::GetColor(BlockSource& region, int xPos, int zPos) const
{
    short maxY = region.getMaxHeight();
    short minY = region.getMinHeight();

    for (int y = maxY; y >= minY; --y) {
        const Block& block = region.getBlock(xPos, y, zPos);

        if (block.mLegacyBlock->mID != 0) {
            // Get the blocks map color
            mce::Color color = block.getMapColor(region, BlockPos(xPos, y, zPos));

            if (color.r == 0 && color.g == 0 && color.b == 0 && color.a == 0.0f) continue;
            color.a = 1.0f;

            // If the block has little neighbors, its higher than surrounding blocks, shade bright
            // If has many neighbors shade normally.
            float blocksAbove = (float)countBlockNeighbors(region, xPos, y + 1, zPos) / 4;

            float start = 0.9f;
            float end = 0.7f;
            float darkening = start + blocksAbove * (end - start);

            color.r *= darkening;
            color.g *= darkening;
            color.b *= darkening;

            return color;
        }
    }

    return {0, 0, 0, 0xFF};
}

void Minimap::TessellateChunkMesh(BlockSource& region, const ChunkPos& chunkPos)
{
    mTes.begin(mce::PrimitiveMode::QuadList, 16 * 16);

    int worldX = chunkPos.x * 16;
    int worldZ = chunkPos.z * 16;

    for (int chunkX = 0; chunkX < 16; chunkX++) {
        for (int chunkZ = 0; chunkZ < 16; chunkZ++) {
            // Sample the colour of the current block
            auto color = GetColor(region, worldX + chunkX, worldZ + chunkZ);

            mTes.color(color.r, color.g, color.b, color.a);

            for (auto& vert : vertexes) {
                Vec3 scaledVert = vert;

                if (chunkX == 15 && vert.x == 1.0f) scaledVert = scaledVert + Vec3(0.1f, 0.0f, 0.0f);
                if (chunkZ == 15 && vert.y == 1.0f) scaledVert = scaledVert + Vec3(0.0f, 0.1f, 0.0f);

                Vec3 transformedPos = Vec3((float)chunkX, (float)chunkZ, 0.0f) + scaledVert;

                mTes.vertex(transformedPos);
            }
        }
    }

    // Save the chunk to the cache.
    mChunkToMesh[chunkPos.packed] = mTes.end(0, "Untagged Minimap Chunk", 0);
    mTes.clear();
}

void Minimap::Render(MinecraftUIRenderContext& ctx)
{
    ClientInstance& client = *Amethyst::GetContext().mClientInstance;
    BlockSource* region = client.getRegion();

    // Game is still loading...
    if (region == nullptr) return;

    for (auto& chunkPos : mChunkDrawDeferList) {
        TessellateChunkMesh(*region, chunkPos);
    }

    mChunkDrawDeferList.clear();

    uint8_t dimId = region->getDimensionConst().mId.runtimeID;

    // If the player entered another dimension clear the cache
    if (mLastDimID != dimId) {
        mLastDimID = dimId;
        DeleteAllChunkMeshes();
    }

    // Save the games clipping rectangles before we add ours.
    ctx.saveCurrentClippingRectangle();

    // Stencil out any chunk meshes that overlap the edges of the minimap.
    Vec2 screenSize = client.guiData->clientUIScreenSize;

    RectangleArea rect{ screenSize.x - (mMinimapSize + mMinimapEdgeBorder), screenSize.x - mMinimapEdgeBorder, mMinimapEdgeBorder, mMinimapEdgeBorder + mMinimapSize };
    ctx.setClippingRectangle(rect);

    Matrix& matrix = ctx.mScreenContext->camera->worldMatrixStack.stack.top();
    Matrix originalMatrix = matrix;

    Vec3* playerPos = ctx.mClient->getLocalPlayer()->getPosition();
    ChunkPos playerChunkPos = ChunkPos((int)playerPos->x / 16, (int)playerPos->z / 16);

    std::vector<ChunkPos> mChunksToRender;

    for (int x = -mRenderDistance - 1; x <= mRenderDistance + 1; x++) {
        for (int z = -mRenderDistance - 1; z <= mRenderDistance + 1; z++) {
            ChunkPos chunkPos(x + playerChunkPos.x, z + playerChunkPos.z);
            mChunksToRender.push_back(chunkPos);
        }
    }

    float unitsPerBlock = mMinimapSize / (mRenderDistance * 16 * 2);

    // Render each chunk in the players minimap render distance.
    for (auto& chunkPos : mChunksToRender)
    {
        // Attempt to find a mesh for this chunk
        auto mesh = mChunkToMesh.find(chunkPos.packed);
        if (mesh == mChunkToMesh.end()) continue;

        float xChunkTranslation = (((chunkPos.x * 16) - playerPos->x + mRenderDistance * 16)) * unitsPerBlock;
        float zChunkTranslation = (((chunkPos.z * 16) - playerPos->z + mRenderDistance * 16)) * unitsPerBlock;

        xChunkTranslation += screenSize.x - (mMinimapSize + mMinimapEdgeBorder);
        zChunkTranslation += mMinimapEdgeBorder;

        // Chunks are drawn from the top left corner of the screen, so translate them to their intended position on screen
        // Then undo that translation as not to screw up minecrafts rendering, or rendering of other minimap chunks
        

        matrix.translate(xChunkTranslation, zChunkTranslation, 0.0f);
        matrix.scale(unitsPerBlock, unitsPerBlock, unitsPerBlock);
        mesh->second.renderMesh(*ctx.mScreenContext, *mMinimapMaterial);
        matrix = originalMatrix;
    }

    // Remove our clipping rectangle for the minimap renderer
    ctx.restoreSavedClippingRectangle();

    // Draw minimap border
    // The stenciling in MinecraftUIRenderContext has an off by 1 error
    // Also crashes on world leave
    rect._x0 -= 1;
    rect._y0 -= 1;

    mOutlineNineslice.Draw(rect, &mMinimapOutline, &ctx);
    HashedString flushString(0xA99285D21E94FC80, "entity_alphablend");
    ctx.flushImages(mce::Color::WHITE, 1.0f, flushString);

    // Draw player position icon
    float midX = (rect._x0 + 1 + rect._x1) / 2.0f;
    float midY = (rect._y0 + 1 + rect._y1) / 2.0f;

    Vec2* headRot = ctx.mClient->getLocalPlayer()->getHeadRot();

    mTes.begin(mce::PrimitiveMode::QuadList, 4);

    for (auto& vert : vertexes) {
        float size = 10;

        Vec3 transformedVert = vert * Vec3(size, size, 1.0f);

        transformedVert.x += midX - size / 2;
        transformedVert.y += midY - size / 2;

        transformedVert.rotateAroundPointDegrees(
            Vec3(midX, midY, 0.0f),
            Vec3(0.0f, 0.0f, headRot->y + 180.0f)
        );

        mTes.vertexUV(transformedVert, vert.x, vert.y);
    }

    mce::Mesh mesh = mTes.end(0, "player_pos_icon", 0);
    mTes.clear();
   
    mesh.renderMesh(*ctx.mScreenContext, *mMinimapMaterial, mMinimapPosIcon);
}

void Minimap::CullChunk(ChunkPos pos) {
    this->mChunkToMesh.erase(pos.packed);
}

void Minimap::DeleteAllChunkMeshes()
{
    mChunkToMesh.clear();
}

//void Minimap::onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource)
//{
//    ChunkPos chunkPos(pos.x / 16, pos.z / 16);
//    Log::Info("[CHA] POS: ({}; {}) {}", chunkPos.x, chunkPos.z, (int)lc.mLoadState);
//    TessellateChunkMesh(source, chunkPos);
//}

void Minimap::onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource)
{
    ChunkPos chunkPos(pos.x / 16, pos.z / 16);
    mChunkDrawDeferList.insert(chunkPos);
}

void Minimap::onChunkUnloaded(LevelChunk& lc)
{
    CullChunk(lc.mPosition);
}

void Minimap::onSubChunkLoaded(ChunkSource& source, LevelChunk& lc, short, bool)
{
    mChunkDrawDeferList.insert(lc.mPosition);
}
