#pragma once
#include <amethyst/ui/NinesliceHelper.hpp>
#include <glm/glm.hpp>
#include <minecraft/src-client/common/client/renderer/TexturePtr.hpp>
#include <minecraft/src-client/common/client/renderer/helpers/MeshHelpers.hpp>
#include <minecraft/src-client/common/client/renderer/screen/MinecraftUIRenderContext.hpp>
#include <minecraft/src-deps/core/resource/ResourceHelper.hpp>
#include <minecraft/src-deps/minecraftrenderer/renderer/Mesh.hpp>
#include <minecraft/src/common/world/level/chunk/LevelChunk.hpp>
#include <minecraft/src/common/world/level/LevelListener.hpp>
#include <minecraft/src/common/world/level/block/Block.hpp>
#include <unordered_map>

class Minimap : public LevelListener {
private:
    std::unordered_map<uint64_t, mce::Mesh> mChunkToMesh;
    std::vector<ChunkPos> mChunkDrawDeferList;

    Tessellator& mTes;

    mce::MaterialPtr* mMinimapMaterial;
    mce::TexturePtr mMinimapOutline;
    mce::TexturePtr mMinimapPosIcon;
    Amethyst::NinesliceHelper mOutlineNineslice;

    uint8_t mLastDimID;
    int mFramesSinceLastCull;

public:
    int mRenderDistance = 32;
    int mMaxChunksToGeneratePerFrame = 16;
    float mMinimapSize;
    float mMinimapEdgeBorder;
    float mUnitsPerBlock;

public:
    Minimap(MinecraftUIRenderContext& context);

    void TessellateChunkMesh(BlockSource& region, const ChunkPos& chunkPos);
    void Render(MinecraftUIRenderContext& uiCtx);
    void DeleteAllChunkMeshes();

    //virtual void onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource) override;

    virtual void onChunkUnloaded(LevelChunk& lc) override;
    virtual void onSubChunkLoaded(class ChunkSource& source, class LevelChunk& lc, short, bool) override;


private:
    mce::Color GetColor(BlockSource& region, int xPos, int zPos) const;
    void CullChunk(ChunkPos pos);
};