#include "Minimap.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <set>
#include <amethyst/ui/NinesliceHelper.h>

Vec3 vertexes[6] = {
    Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f),
    Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f)
};

Minimap::Minimap(ClientInstance* client, Tessellator* tes)
    : mOutlineNineslice(30, 30, 10, 10)
{
    mClient = client;
    mTes = tes;
    mMinimapMaterial = reinterpret_cast<mce::MaterialPtr*>(SlideAddress(0x5853DB0));

    mMinimapEdgeBorder = 10.0f;
    mMinimapSize = 128.0f;
    mUnitsPerBlock = mMinimapSize / (mRenderDistance * 16 * 2);
}

std::optional<mce::Color> Minimap::GetColor(int xPos, int zPos) const
{
    BlockSource* region = mClient->getRegion();
    
    short maxY = region->getMaxHeight();
    short minY = region->getMinHeight();

    for (int y = maxY; y > minY; --y) {
        const Block* block = &region->getBlock(xPos, y, zPos);

        if (block->mLegacyBlock->mID != 0) {
            mce::Color color = block->mLegacyBlock->getMapColor(*region, BlockPos(xPos, y, zPos), *block);
            color.a = 1.0f;

            if (color.r == 0.0f && color.g == 0.0f && color.b == 0.0f && color.a == 0.0f) continue;
            return color;
        }
    }

    return std::nullopt;
}

void Minimap::UpdateChunk(ChunkPos chunkPos)
{
    mTes->begin(mce::TriangleList, 16 * 16);

    int worldX = chunkPos.x * 16;
    int worldZ = chunkPos.z * 16;

    for (int chunkX = 0; chunkX < 16; chunkX++) {
        for (int chunkZ = 0; chunkZ < 16; chunkZ++) {
            // Sample the colour of the current block
            auto colorOpt = GetColor(worldX + chunkX, worldZ + chunkZ);

            if (!colorOpt.has_value()) {
                mTes->clear();
                return;
            }

            mce::Color color = colorOpt.value();
            mTes->color(color.r, color.g, color.b, color.a);

            // Draw each block with 2 triangles
            // To do: Look into drawing with quads to reduce size in memory.
            for (auto& vert : vertexes) {
                Vec3 scaledVert = vert * Vec3(mUnitsPerBlock, mUnitsPerBlock, 1.0f);
                Vec3 transformedPos = Vec3(chunkX * mUnitsPerBlock, chunkZ * mUnitsPerBlock, 0.0f) + scaledVert;
                mTes->vertex(transformedPos);
            }
        }
    }

    mce::Mesh mesh = mTes->end(0, "Untagged Minimap Chunk", 0);
    mTes->clear();
    mChunkPosToMesh[chunkPos.packed] = mesh;
}

void Minimap::Render(MinecraftUIRenderContext* uiCtx)
{
    if (!mHasLoadedTextures) {
        ResourceLocation resource("textures/ui/minimap_border");
        mMinimapOutline = uiCtx->getTexture(&resource, true);
        mHasLoadedTextures = true;
    }

    // Save the games clipping rectangles before we add ours.
    uiCtx->saveCurrentClippingRectangle();

    // Stencil out any chunk meshes that overlap the edges of the minimap.
    Vec2 screenSize = mClient->guiData->clientUIScreenSize;
    RectangleArea rect{ screenSize.x - (mMinimapSize + mMinimapEdgeBorder), screenSize.x - mMinimapEdgeBorder, mMinimapEdgeBorder, mMinimapEdgeBorder + mMinimapSize };
    uiCtx->setClippingRectangle(rect);

    Matrix& matrix = uiCtx->mScreenContext->camera->worldMatrixStack.stack.top();
    Vec3* playerPos = uiCtx->mClient->getLocalPlayer()->getPosition();
    ChunkPos playerChunkPos = ChunkPos((int)playerPos->x / 16, (int)playerPos->z / 16);
    int chunksGeneratedThisFrame = 0;

    // chunkPos, distance to player
    std::vector<std::pair<ChunkPos, double>> mChunksToRender;

    for (int x = -mRenderDistance - 1; x <= mRenderDistance + 1; x++) {
        for (int z = -mRenderDistance - 1; z <= mRenderDistance + 1; z++) {
            ChunkPos chunkPos(x + playerChunkPos.x, z + playerChunkPos.z);

            double distance = sqrt(
                (chunkPos.x - playerChunkPos.x) * (chunkPos.x - playerChunkPos.x) + 
                (chunkPos.z - playerChunkPos.z) * (chunkPos.z - playerChunkPos.z)
            );

            mChunksToRender.push_back(std::make_pair(chunkPos, distance));
        }
    }

    // Sort the chunks in order of distance, so closer chunks are prioritised
    std::sort(mChunksToRender.begin(), mChunksToRender.end(), [](std::pair<ChunkPos, double> a, std::pair<ChunkPos, double> b) {
        return a.second < b.second;
    });

    // Render each chunk in the players minimap render distance.
    for (auto& chunk : mChunksToRender) 
    {
        ChunkPos chunkPos = chunk.first;
        auto mesh = mChunkPosToMesh.find(chunkPos.packed);
            
        // The chunk has not had a mesh generated yet
        if (mesh == mChunkPosToMesh.end()) {
            // Dont generate too much at once. Minimap doesn't need a high priority
            if (chunksGeneratedThisFrame >= mMaxChunksToGeneratePerFrame) continue;

            // Create a mesh for that chunk
            chunksGeneratedThisFrame += 1;
            UpdateChunk(chunkPos);
            continue;
        }

        float xChunkTranslation = ((chunkPos.x * 16) - playerPos->x + mRenderDistance * 16) * mUnitsPerBlock;
        float zChunkTranslation = ((chunkPos.z * 16) - playerPos->z + mRenderDistance * 16) * mUnitsPerBlock;

        xChunkTranslation += screenSize.x - (mMinimapSize + mMinimapEdgeBorder);
        zChunkTranslation += mMinimapEdgeBorder;

        // Chunks are drawn from the top left corner of the screen, so translate them to their intended position on screen
        // Then undo that translation as not to screw up minecrafts rendering, or rendering of other minimap chunks
        matrix.translate(xChunkTranslation, zChunkTranslation, 0.0f);
        mesh->second.renderMesh(uiCtx->mScreenContext, mMinimapMaterial);
        matrix.translate(-xChunkTranslation, -zChunkTranslation, 0.0f);
    }

    // Remove our clipping rectangle for the minimap renderer
    uiCtx->restoreSavedClippingRectangle();

    rect._x0 -= 1;
    rect._y0 -= 1;
    mOutlineNineslice.Draw(rect, &mMinimapOutline, uiCtx);
    HashedString flushString(0xA99285D21E94FC80, "ui_flush");
    uiCtx->flushImages(mce::Color::WHITE, 1.0f, flushString);
}

void Minimap::ClearCache()
{
    mChunkPosToMesh.clear();
}

void Minimap::onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource)
{
    ChunkPos chunkPos(pos.x / 16, pos.z / 16);
    UpdateChunk(chunkPos);
}
