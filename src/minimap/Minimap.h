#pragma once
#include <unordered_map>
#include <minecraft/src/common/world/level/ChunkPos.h>
#include <minecraft/src-deps/minecraftrenderer/renderer/Mesh.h>
#include <minecraft/src-client/common/client/renderer/screen/MinecraftUIRenderContext.h>
#include <minecraft/src-client/common/client/renderer/helpers/MeshHelpers.h>
#include <minecraft/src/common/world/level/block/Block.h>
#include <minecraft/src/common/world/level/BlockSourceListener.h>
#include <minecraft/src-deps/core/resource/ResourceHelper.h>
#include <minecraft/src-client/common/client/renderer/TexturePtr.h>
#include <glm/glm.hpp>
#include <amethyst/ui/NinesliceHelper.h>

class Minimap : public BlockSourceListener {
private:
    std::unordered_map<uint64_t, mce::Mesh> mChunkPosToMesh;
    ClientInstance* mClient;
    Tessellator* mTes;
    mce::MaterialPtr* mMinimapMaterial;
    bool mHasLoadedTextures = false;

    mce::TexturePtr mMinimapOutline;
    Amethyst::NinesliceHelper mOutlineNineslice;

public:
    int mRenderDistance = 8;
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
    std::optional<mce::Color> GetColor(int xPos, int zPos) const;
};