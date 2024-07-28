#include "Minimap.h"
#include <amethyst/runtime/ModContext.hpp>
#include <minecraft/src-client/common/client/game/ClientInstance.hpp>
#include <minecraft/src-client/common/client/gui/gui/GuiData.hpp>
#include <minecraft/src-client/common/client/player/LocalPlayer.hpp>
#include <minecraft/src-client/common/client/renderer/Tessellator.hpp>
#include <minecraft/src/common/world/level/BlockSource.hpp>
#include <minecraft/src/common/world/level/block/BlockLegacy.hpp>
#include <minecraft/src/common/world/level/dimension/Dimension.hpp>

extern std::shared_ptr<Minimap> minimap;

Vec3 unitQuad[4] = {
    Vec3(0.0f, 0.0f, 0.0f),
    Vec3(0.0f, 1.0f, 0.0f),
    Vec3(1.0f, 1.0f, 0.0f),
    Vec3(1.0f, 0.0f, 0.0f),
};

Minimap::Minimap()
    : mOutlineNineslice(30, 30, 10, 10), mLastDimID(0)
{
    mMinimapMaterial = reinterpret_cast<mce::MaterialPtr*>(SlideAddress(0x59BD7E0));

    mMinimapEdgeBorder = 3.0f;
    mMinimapSize = 128.0f;
}

void Minimap::_LoadTextures(MinecraftUIRenderContext& ctx)
{
    /* Load any necessary textures. */
    mMinimapOutline = ctx.getTexture("textures/ui/minimap_border", true);
    mMinimapPosIcon = ctx.getTexture("textures/ui/minimap_pos_icon", true);
    mHasLoadedTextures = true;
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

#ifdef VERTEX_STATS
uint64_t vertexCount = 0;
uint64_t chunkCount = 0;
#endif

void Minimap::TessellateChunkMesh(Tessellator& mTes, BlockSource& region, const ChunkPos& chunkPos)
{
    LevelChunk* levelChunk = region.getChunk(chunkPos);
    if (levelChunk == nullptr) return;

    mTes.begin(mce::PrimitiveMode::QuadList, 16 * 16);

    int worldX = chunkPos.x * 16;
    int worldZ = chunkPos.z * 16;

    // Pre-sample block colours
    std::array<std::array<uint32_t, 16>, 16> packedColorData{};

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            packedColorData[x][z] = GetColor(region, worldX + x, worldZ + z).As32();
        }
    }

    // Greedy meshing
    std::vector<std::vector<bool>> visited(16, std::vector<bool>(16, false));

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            if (visited[x][z]) continue;

            uint32_t color = packedColorData[x][z];
            int width = 1, height = 1;

            // Determine the width
            for (int wx = x + 1; wx < 16 && packedColorData[wx][z] == color && !visited[wx][z]; wx++) {
                width++;
            }

            // Try expand height
            bool canExpand = true;
            for (int hz = z + 1; hz < 16 && canExpand; hz++) {
                for (int wx = x; wx < x + width; wx++) {
                    if (packedColorData[wx][hz] != color || visited[wx][hz]) {
                        canExpand = false;
                        break;
                    }
                }

                if (canExpand) height++;
            }

            // Mark positions as visited
            for (int wx = x; wx < x + width; wx++) {
                for (int hz = z; hz < z + height; hz++) {
                    visited[wx][hz] = true;
                }
            }

            // Draw the quad
            mTes.color(color);

            Vec3 verts[4] = {
                Vec3((float)x, (float)z, 0.0f),
                Vec3((float)x, (float)(z + height), 0.0f),
                Vec3((float)(x + width), (float)(z + height), 0.0f),
                Vec3((float)(x + width), (float)z, 0.0f),
            };

            for (auto& vert : verts) {
                Vec3 transformedPos = vert;
                mTes.vertex(transformedPos);

#ifdef VERTEX_STATS
                vertexCount += 1;
#endif
            }
        }
    }

#ifdef VERTEX_STATS
    chunkCount += 1;
    Log::Info("avg vert count {}", vertexCount / (float)chunkCount);
#endif

    // Save the chunk to the cache.
    mChunkToMesh[chunkPos.packed] = mTes.end(0, "Untagged Minimap Chunk", 0);
    mTes.clear();
}

void Minimap::Render(MinecraftUIRenderContext& ctx)
{
    ClientInstance& client = *Amethyst::GetContext().mClientInstance;
    BlockSource* region = client.getRegion();
    Tessellator& mTes = ctx.mScreenContext->tessellator;

    if (!mHasLoadedTextures) {
        _LoadTextures(ctx);
    }

    // Game is still loading...
    if (region == nullptr) return;

    int chunksGeneratedThisFrame = 0;
    std::vector<uint64_t> chunksDrawn;

    for (auto& chunkPos : mChunkDrawDeferList) {
        if (chunksGeneratedThisFrame >= this->mMaxChunksToGeneratePerFrame) {
            break;
        }

        ChunkPos unpacked(chunkPos);
        TessellateChunkMesh(mTes, *region, unpacked);
        chunksGeneratedThisFrame++;

        chunksDrawn.push_back(chunkPos);
    }

    for (auto& chunkPos : chunksDrawn) {
        mChunkDrawDeferList.erase(chunkPos);
    }

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

    RectangleArea rect{screenSize.x - (mMinimapSize + mMinimapEdgeBorder), screenSize.x - mMinimapEdgeBorder, mMinimapEdgeBorder, mMinimapEdgeBorder + mMinimapSize};
    ctx.setClippingRectangle(rect);

    // Draw the black background
    ctx.fillRectangle(rect, mce::Color::BLACK, 1.0);

    Matrix& matrix = ctx.mScreenContext->camera->worldMatrixStack.stack.top();
    Matrix originalMatrix = matrix;

    Vec3* playerPos = ctx.mClient->getLocalPlayer()->getPosition();
    ChunkPos playerChunkPos = ChunkPos((int)playerPos->x / 16, (int)playerPos->z / 16);

    float unitsPerBlock = mMinimapSize / (mRenderDistance * 16 * 2);

    for (int x = -mRenderDistance - 1; x <= mRenderDistance + 1; x++) {
        for (int z = -mRenderDistance - 1; z <= mRenderDistance + 1; z++) {
            ChunkPos chunkPos(x + playerChunkPos.x, z + playerChunkPos.z);

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
    }

    // Log::Info("Vertex count: {}, drawing {} chunks. average: {}", vertexCount, chunksCount, vertexCount / (float)vertexCount);

    // Remove our clipping rectangle for the minimap renderer
    ctx.restoreSavedClippingRectangle();

    // Draw minimap border
    // The stenciling in MinecraftUIRenderContext has an off by 1 error
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

    for (auto& vert : unitQuad) {
        float size = 10;

        Vec3 transformedVert = vert * Vec3(size, size, 1.0f);

        transformedVert.x += midX - size / 2;
        transformedVert.y += midY - size / 2;

        transformedVert.rotateAroundPointDegrees(
            Vec3(midX, midY, 0.0f),
            Vec3(0.0f, 0.0f, headRot->y + 180.0f));

        mTes.vertexUV(transformedVert, vert.x, vert.y);
    }

    mce::Mesh mesh = mTes.end(0, "player_pos_icon", 0);
    mTes.clear();

    mesh.renderMesh(*ctx.mScreenContext, *mMinimapMaterial, mMinimapPosIcon);
}

void Minimap::CullChunk(const ChunkPos& pos)
{
    this->mChunkToMesh.erase(pos.packed);
}

void Minimap::DeleteAllChunkMeshes()
{
    mChunkToMesh.clear();
}

void Minimap::onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource)
{
    ChunkPos chunkPos(pos.x / 16, pos.z / 16);
    mChunkDrawDeferList.insert(chunkPos.packed);
}

void Minimap::onChunkUnloaded(LevelChunk& lc)
{
    CullChunk(lc.mPosition);
}

void Minimap::onSubChunkLoaded(ChunkSource& source, LevelChunk& lc, short, bool)
{
    if (!minimap) return;
    mChunkDrawDeferList.insert(lc.mPosition.packed);
}

// Some server softwares don't support sending in SubChunks, so this fixes the edge case where they send full chunks at once.
void Minimap::onChunkLoaded(ChunkSource& source, LevelChunk& lc)
{
    if (!minimap) return;
    mChunkDrawDeferList.insert(lc.mPosition.packed);
}
