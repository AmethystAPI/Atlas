#include "Minimap.h"

Vec3 vertexes[6] = {
    Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f),
    Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f)
};

Minimap::Minimap(ClientInstance* client, Tessellator* tes)
{
    mClient = client;
    mTes = tes;
    mMinimapMaterial = reinterpret_cast<mce::MaterialPtr*>(SlideAddress(0x5853DB0));

    mMinimapSize = 256.0f;
    mUnitsPerBlock = mMinimapSize / (mRenderDistance * 16 * 2);
}

void Minimap::UpdateChunk(ChunkPos chunkPos)
{
    BlockSource* region = mClient->getRegion();
    mTes->begin(mce::TriangleList, 16 * 16);

    int worldX = chunkPos.x * 16;
    int worldZ = chunkPos.z * 16;

    int testWidth = mRenderDistance * 16 * 2;

    for (int chunkX = 0; chunkX < testWidth; chunkX++) {
        for (int chunkZ = 0; chunkZ < testWidth; chunkZ++) {

            // Sample block color
            mce::Color color((float)chunkX / 16.f, ((float)chunkZ / 16.f), 0.0f, 1.0f);
            mTes->color(color.r, color.g, color.b, color.a);

            for (auto& vert : vertexes) {
                Vec3 transformedPos = Vec3(chunkX * mUnitsPerBlock, chunkZ * mUnitsPerBlock, 0.0f) + vert;
                mTes->vertex(transformedPos);
            }
        }
    }

    mChunkPosToMesh[chunkPos.packed] = mTes->end(0, "Untagged Minimap Chunk", 0);
    mTes->clear();
}

void Minimap::Render(MinecraftUIRenderContext* uiCtx)
{
    ChunkPos chunkPos(0, 0);

    auto it = mChunkPosToMesh.find(chunkPos.packed);
    if (it == mChunkPosToMesh.end()) return;

    // Step 1: Change the position of the mesh on screen using matrixes
    // Step 2: Profit
    it->second.renderMesh(uiCtx->mScreenContext, mMinimapMaterial);
}
