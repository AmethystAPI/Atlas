#include "Minimap.h"
#include <amethyst/ui/NinesliceHelper.hpp>
#include <minecraft/src/common/world/level/block/Block.hpp>
#include <minecraft/src/common/world/level/block/BlockLegacy.hpp>

Vec3 vertexes[4] = {
    Vec3(0.0f, 0.0f, 0.0f),
    Vec3(0.0f, 1.0f, 0.0f),
    Vec3(1.0f, 1.0f, 0.0f),
    Vec3(1.0f, 0.0f, 0.0f),
};

Minimap::Minimap(ClientInstance* client, Tessellator* tes)
    : mOutlineNineslice(30, 30, 10, 10)
{
    mClient = client;
    mTes = tes;
    mMinimapMaterial = reinterpret_cast<mce::MaterialPtr*>(SlideAddress(0x5853DB0));

    mMinimapEdgeBorder = 3.0f;
    mMinimapSize = 128.0f;
    mUnitsPerBlock = mMinimapSize / (mRenderDistance * 16 * 2);
}

int countBlockNeighbors(const BlockSource* region, int xPos, int yPos, int zPos)
{
    int count = 0;

    const Block* block = &region->getBlock(xPos - 1, yPos, zPos);
    if (block->mLegacyBlock->mID != 0) count += 1;

    block = &region->getBlock(xPos, yPos, zPos - 1);
    if (block->mLegacyBlock->mID != 0) count += 1;

    block = &region->getBlock(xPos + 1, yPos, zPos);
    if (block->mLegacyBlock->mID != 0) count += 1;

    block = &region->getBlock(xPos, yPos, zPos + 1);
    if (block->mLegacyBlock->mID != 0) count += 1;

    return count;
}

mce::Color Minimap::GetColor(int xPos, int zPos) const
{
    BlockSource* region = mClient->getRegion();

    short maxY = region->getMaxHeight();
    short minY = region->getMinHeight();

    for (int y = maxY; y >= minY; --y) {
        const Block* block = &region->getBlock(xPos, y, zPos);

        if (block->mLegacyBlock->mID != 0) {
            // Get the blocks map color
            mce::Color color = block->getMapColor(*region, BlockPos(xPos, y, zPos));

            if (color.a == 0.0f) continue;
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

void Minimap::UpdateChunk(ChunkPos chunkPos)
{
    BlockSource* region = mClient->getRegion();

    mTes->begin(mce::QuadList, 16 * 16);

    int worldX = chunkPos.x * 16;
    int worldZ = chunkPos.z * 16;

    [[unlikely]]
    if (!region->areChunksFullyLoaded(BlockPos(chunkPos.x * 16, 0, chunkPos.z * 16), 1)) {
        mTes->clear();
        return;
    }

    for (int chunkX = 0; chunkX < 16; chunkX++) {
        for (int chunkZ = 0; chunkZ < 16; chunkZ++) {
            // Sample the colour of the current block
            auto color = GetColor(worldX + chunkX, worldZ + chunkZ);

            mTes->color(color.r, color.g, color.b, color.a);

            // Draw each block with 2 triangles
            // To do: Look into drawing with quads to reduce size in memory.
            for (auto& vert : vertexes) {
                Vec3 scaledVert = vert * Vec3(mUnitsPerBlock, mUnitsPerBlock, 1.0f);

                if (chunkX == 15 && vert.x == 1.0f) scaledVert = scaledVert + Vec3(0.1f, 0.0f, 0.0f);
                if (chunkZ == 15 && vert.y == 1.0f) scaledVert = scaledVert + Vec3(0.0f, 0.1f, 0.0f);

                Vec3 transformedPos = Vec3(chunkX * mUnitsPerBlock, chunkZ * mUnitsPerBlock, 0.0f) + scaledVert;

                mTes->vertex(transformedPos);
            }
        }
    }

    mce::Mesh mesh = mTes->end(0, "Untagged Minimap Chunk", 0);
    mTes->clear();

    mChunkToMesh[chunkPos.packed] = mesh;
}

void Minimap::Render(MinecraftUIRenderContext* uiCtx)
{
    BlockSource* region = mClient->getRegion();
    uint8_t dimId = region->getDimensionConst().mId;

    [[unlikely]]
    // If the player entered another dimension clear the cache
    if (mLastDimID != dimId) {
        mLastDimID = dimId;
        this->ClearCache();
    }

    [[unlikely]]
    if (!mHasLoadedTextures) {
        ResourceLocation outlineResource("textures/ui/minimap_border");
        mMinimapOutline = uiCtx->getTexture(&outlineResource, true);

        ResourceLocation posIconResource("textures/ui/minimap_pos_icon");
        mMinimapPosIcon = uiCtx->getTexture(&posIconResource, true);

        mHasLoadedTextures = true;
    }

    // Save the games clipping rectangles before we add ours.
    uiCtx->saveCurrentClippingRectangle();

    // Stencil out any chunk meshes that overlap the edges of the minimap.
    Vec2 screenSize = mClient->guiData->clientUIScreenSize;

    RectangleArea rect{ screenSize.x - (mMinimapSize + mMinimapEdgeBorder), screenSize.x - mMinimapEdgeBorder, mMinimapEdgeBorder, mMinimapEdgeBorder + mMinimapSize };
    uiCtx->setClippingRectangle(rect);

    Matrix& matrix = uiCtx->mScreenContext->camera->worldMatrixStack.stack.top();
    Matrix originalMatrix = matrix;

    Vec3* playerPos = uiCtx->mClient->getLocalPlayer()->getPosition();
    ChunkPos playerChunkPos = ChunkPos((int)playerPos->x / 16, (int)playerPos->z / 16);

    [[unlikely]]
    if (mFramesSinceLastCull >= mFrameCullInterval) {
        mFramesSinceLastCull = 0;
        this->CullChunks(playerChunkPos);
    }

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

        auto mesh = mChunkToMesh.find(chunkPos.packed);

        // The chunk in the dimension has not had a mesh generated yet
        [[unlikely]]
        if (mesh == mChunkToMesh.end()) {
            // Dont generate too much at once. Minimap doesn't need a high priority
            if (chunksGeneratedThisFrame >= mMaxChunksToGeneratePerFrame) continue;

            // Create a mesh for that chunk
            chunksGeneratedThisFrame += 1;
            UpdateChunk(chunkPos);
            continue;
        }

        float xChunkTranslation = (((chunkPos.x * 16) - playerPos->x + mRenderDistance * 16)) * mUnitsPerBlock;
        float zChunkTranslation = (((chunkPos.z * 16) - playerPos->z + mRenderDistance * 16)) * mUnitsPerBlock;

        xChunkTranslation += screenSize.x - (mMinimapSize + mMinimapEdgeBorder);
        zChunkTranslation += mMinimapEdgeBorder;

        // Chunks are drawn from the top left corner of the screen, so translate them to their intended position on screen
        // Then undo that translation as not to screw up minecrafts rendering, or rendering of other minimap chunks
        matrix.translate(xChunkTranslation, zChunkTranslation, 0.0f);
        mesh->second.renderMesh(uiCtx->mScreenContext, mMinimapMaterial);
        matrix = originalMatrix;
    }

    // Remove our clipping rectangle for the minimap renderer
    uiCtx->restoreSavedClippingRectangle();

    // Draw minimap border
    // The stenciling in MinecraftUIRenderContext has an off by 1 error
    // Also crashes on world leave
    rect._x0 -= 1;
    rect._y0 -= 1;

    mOutlineNineslice.Draw(rect, &mMinimapOutline, uiCtx);
    HashedString flushString(0xA99285D21E94FC80, "ui_flush");
    uiCtx->flushImages(mce::Color::WHITE, 1.0f, flushString);

    // Draw player position icon
    float midX = (rect._x0 + 1 + rect._x1) / 2.0f;
    float midY = (rect._y0 + 1 + rect._y1) / 2.0f;

    ActorRotationComponent* playerRotation = uiCtx->mClient->getLocalPlayer()->tryGetComponent<ActorRotationComponent>();

    mTes->begin(mce::PrimitiveMode::QuadList, 4);

    for (auto& vert : vertexes) {
        float size = 15;

        mTes->color(vert.x, vert.y, vert.z, 1.0f);

        Vec3 transformedVert = vert * Vec3(size, size, 1.0f);

        transformedVert.x += midX - size / 2;
        transformedVert.y += midY - size / 2;

        transformedVert.rotateAroundPointDegrees(
            Vec3(midX, midY, 1.0f),
            Vec3(0.0f, 0.0f, playerRotation->mHeadRotPrev.y)
            );

        mTes->vertexUV(transformedVert, vert.x, vert.y);
    }

    mce::Mesh mesh = mTes->end(0, "player_pos_icon", 0);
    mTes->clear();

    mesh.renderMesh(uiCtx->mScreenContext, mMinimapMaterial);
}

void Minimap::CullChunks(ChunkPos playerChunkPos) {
    int radius = (mRenderDistance + mCullingExemptDistance) * 2;
    int radiusSquared = radius * radius;

    int culledChunks = 0;

    auto it = mChunkToMesh.begin();
    while (it != mChunkToMesh.end()) {
        [[unlikely]]
        if (culledChunks >= mMaxChunksToCullPerCull) {
            return;
        }

        ChunkPos chunkPos((int64_t)it->first);

        int dx = chunkPos.x - playerChunkPos.x;
        int dz = chunkPos.z - playerChunkPos.z;

        // Calculate squared distance to avoid sqrt for performance
        int distanceSquared = dx * dx + dz * dz;

        // Erase the chunk if it's beyond the render distance
        if (distanceSquared > radiusSquared) {
            it = mChunkToMesh.erase(it);
            culledChunks++;
        } else {
            ++it;
        }
    }
}

void Minimap::ClearCache()
{
    mChunkToMesh.clear();
}

void Minimap::onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource)
{
    ChunkPos chunkPos(pos.x / 16, pos.z / 16);
    UpdateChunk(chunkPos);
}
