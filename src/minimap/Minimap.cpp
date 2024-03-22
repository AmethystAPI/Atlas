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
    mUnitsPerBlock = mMinimapSize / (mRenderDistance * 16);
}

mce::Mesh Minimap::CreateChunkMesh(ChunkPos chunkPos)
{
    mTes->begin(mce::TriangleList, 16 * 16);

    int worldX = chunkPos.x * 16;
    int worldZ = chunkPos.z * 16;

    for (int chunkX = 0; chunkX < 16; chunkX++) 
    {
        for (int chunkZ = 0; chunkZ < 16; chunkZ++) 
        {
            // Sample the colour of the current block
            mce::Color color((float)chunkX / 16.f, ((float)chunkZ / 16.f), 0.0f, 1.0f);
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
    return mesh;
}

void Minimap::UpdateChunk(ChunkPos chunkPos)
{
    mChunkPosToMesh[chunkPos.packed] = CreateChunkMesh(chunkPos);
}

void Minimap::Render(MinecraftUIRenderContext* uiCtx)
{
    Matrix& matrix = uiCtx->mScreenContext->camera->worldMatrixStack.stack.top();
    RectangleArea rect{ 10.0f, 100.0f, 0.0f, 230.0f };

    // Save the games clipping rectangles before we add ours.
    uiCtx->saveCurrentClippingRectangle();
    uiCtx->setClippingRectangle(rect);

    for (int x = 0; x < mRenderDistance; x++) {
        for (int z = 0; z < mRenderDistance; z++) {
            ChunkPos chunkPos(x, z);

            matrix.translate(x * 16 * mUnitsPerBlock, z * 16 * mUnitsPerBlock, 0.0f);
            mChunkPosToMesh[chunkPos.packed].renderMesh(uiCtx->mScreenContext, mMinimapMaterial);
            matrix.translate(-x * 16 * mUnitsPerBlock, -z * 16 * mUnitsPerBlock, 0.0f);
        }
    }

    // Remove our clipping rectangle for the minimap renderer
    uiCtx->restoreSavedClippingRectangle();
}
