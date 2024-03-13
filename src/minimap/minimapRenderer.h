//
// Created by Adrian on 11/02/2024.
//
#pragma once
#include <Windows.h>
#include <amethyst/runtime/AmethystContext.h>
#include <minecraft/src/common/world/level/ChunkPos.h>
#include <minecraft/src/common/world/level/BlockSourceListener.h>
#include <minecraft/src/common/world/level/BlockPos.h>
#include <minecraft/src-client/common/client/gui/ScreenView.h>
#include <array>
#include <queue>

class Minimap : public BlockSourceListener {
private:

    std::unordered_map<int64_t, std::unordered_map<int64_t, mce::Color>> mColorData;

    BlockSource* region = nullptr;
    bool hasInitializedMinimap = false;

public:
    /*
    The number of chunks to render on the map
    */
    int mRenderDistance = 6;

    /*
    The number of chunks extra that should be loaded around the current render distance
    Prevents black zones around the edges of the map, but takes extra performance
    */
    int mRenderSafeZone = 1;

    /*
    Should the transparency of blocks be considered?
    */
    bool mUseTranslucency = true;

public:
    void OnJoinGame();
    void OnLeaveGame();
    void Render(ScreenView* screenView, MinecraftUIRenderContext* ctx);

private:
    void AddChunksInRenderDistance(ClientInstance* client);
    void MapChunk(ChunkPos chunkPos, BlockSource* region);

    virtual void onBlockChanged(BlockSource& source, const BlockPos& pos, uint32_t layer, const Block& block, const Block& oldBlock, int updateFlags, const ActorBlockSyncMessage* syncMsg, BlockChangedEventTarget eventTarget, Actor* blockChangeSource) override;
};
