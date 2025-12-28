#pragma once
#include <amethyst/ui/NinesliceHelper.hpp>
#include <glm/glm.hpp>
#include <mc/src-client/common/client/renderer/TexturePtr.hpp>
#include <mc/src-client/common/client/renderer/helpers/MeshHelpers.hpp>
#include <mc/src-client/common/client/renderer/screen/MinecraftUIRenderContext.hpp>
#include <mc/src-deps/core/resource/ResourceHelper.hpp>
#include <mc/src-deps/minecraftrenderer/renderer/Mesh.hpp>
#include <mc/src/common/world/level/LevelListener.hpp>
#include <mc/src/common/world/level/block/Block.hpp>
#include <mc/src/common/world/level/chunk/LevelChunk.hpp>
#include <unordered_map>
#include <unordered_set>

class Minimap : public LevelListener {
private:
	std::unordered_map<uint64_t, mce::Mesh> mChunkToMesh;
	std::unordered_set<uint64_t> mChunkDrawDeferList;

	Tessellator* mTes;

	mce::TexturePtr mMinimapOutline;
	mce::TexturePtr mMinimapPosIcon;
	bool mHasLoadedTextures = false;

	mce::MaterialPtr* mMinimapMaterial;
	Amethyst::NinesliceHelper mOutlineNineslice;

	uint8_t mLastDimID;

public:
	int mRenderDistance = 3;
	int mMaxChunksToGeneratePerFrame = 32;
	float mMinimapSize;
	float mMinimapEdgeBorder;

public:
	Minimap();

	void TessellateChunkMesh(Tessellator& mTes, BlockSource& region, const ChunkPos& chunkPos);
	void Render(MinecraftUIRenderContext& uiCtx);
	void DeleteAllChunkMeshes();

	virtual void onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource) override;
	virtual void onChunkUnloaded(LevelChunk& lc) override;
	virtual void onSubChunkLoaded(class ChunkSource& source, class LevelChunk& lc, short, bool) override;
	virtual void onChunkLoaded(class ChunkSource& source, class LevelChunk& lc) override;

private:
	mce::Color GetColor(BlockSource& region, int xPos, int zPos) const;
	void CullChunk(const ChunkPos& pos);
	void _LoadTextures(MinecraftUIRenderContext& ctx);
};