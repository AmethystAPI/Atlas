#pragma once
#include <unordered_map>
#include <minecraft/src/common/world/level/ChunkPos.h>
#include <minecraft/src-deps/minecraftrenderer/renderer/Mesh.h>
#include <minecraft/src-client/common/client/renderer/screen/MinecraftUIRenderContext.h>
#include <minecraft/src-client/common/client/renderer/helpers/MeshHelpers.h>
#include <glm/glm.hpp>

class Minimap {
private:
    std::unordered_map<uint64_t, mce::Mesh> mChunkPosToMesh;
    ClientInstance* mClient;
    Tessellator* mTes;
    mce::MaterialPtr* mMinimapMaterial;

public:
    int mRenderDistance = 2;
    float mMinimapSize;
    float mUnitsPerBlock;

public:
    Minimap(ClientInstance* client, Tessellator* tes);
    mce::Mesh CreateChunkMesh(ChunkPos chunkPos);
    void UpdateChunk(ChunkPos chunkPos);
    void Render(MinecraftUIRenderContext* uiCtx);
};