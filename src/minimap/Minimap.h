#pragma once
#include <unordered_map>
#include <minecraft/src/common/world/level/ChunkPos.hpp>
#include <minecraft/src-deps/minecraftrenderer/renderer/Mesh.hpp>
#include <minecraft/src-client/common/client/renderer/screen/MinecraftUIRenderContext.hpp>
#include <minecraft/src-client/common/client/renderer/helpers/MeshHelpers.hpp>
#include <minecraft/src/common/world/level/block/Block.hpp>
#include <minecraft/src/common/world/level/BlockSourceListener.hpp>
#include <minecraft/src-deps/core/resource/ResourceHelper.hpp>
#include <minecraft/src-client/common/client/renderer/TexturePtr.hpp>
#include <glm/glm.hpp>
#include <amethyst/ui/NinesliceHelper.hpp>

class Minimap : public BlockSourceListener {
private:
    std::unordered_map<uint64_t, mce::Mesh> mChunkToMesh;
    Tessellator* mTes;
    mce::MaterialPtr* mMinimapMaterial;
    bool mHasLoadedTextures = false;

    mce::TexturePtr mMinimapOutline;
    mce::TexturePtr mMinimapPosIcon;
    Amethyst::NinesliceHelper mOutlineNineslice;

    uint8_t mLastDimID;

public:
    ClientInstance* mClient;
    int mRenderDistance = 6;
    int mCullingExemptDistance = 3;
    int mMaxChunksToGeneratePerFrame = 16;
    float mMinimapSize;
    float mMinimapEdgeBorder;
    float mUnitsPerBlock;

public:
    Minimap(ClientInstance* client, Tessellator* tes);
    void UpdateChunk(ChunkPos chunkPos);
    void Render(MinecraftUIRenderContext* uiCtx);
    void ClearCache();
    virtual void onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource) override;

private:
    mce::Color GetColor(int xPos, int zPos) const;
    void CollectNotLoadedChunks(ChunkPos playerChunkPos);
};