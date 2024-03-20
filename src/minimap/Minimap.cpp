#include "Minimap.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

mce::Mesh Minimap::CreateChunkMesh(ChunkPos chunkPos)
{
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

    mce::Mesh mesh = mTes->end(0, "Untagged Minimap Chunk", 0);
    mTes->clear();
    return mesh;
}

void Minimap::UpdateChunk(ChunkPos chunkPos)
{
    mChunkPosToMesh[chunkPos.packed] = CreateChunkMesh(chunkPos);
}

void Minimap::Render(MinecraftUIRenderContext* uiCtx)
{
    ChunkPos chunkPos(0, 0);

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

    // Does worky but I need an mce::Mesh
    MeshHelpers::renderMeshImmediately(uiCtx->mScreenContext, mTes, mMinimapMaterial);

    // Does not worky
    /*mce::Mesh m = mTes->end(0, "Test", 0);
    m.renderMesh(uiCtx->mScreenContext, mMinimapMaterial);*/
}
