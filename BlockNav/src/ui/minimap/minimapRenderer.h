//
// Created by Adrian on 11/02/2024.
//
#pragma once
#include <Windows.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>
#include <amethyst/events/EventManager.h>
#include <minecraft/src/common/world/level/ChunkPos.h>
#include <array>
#include <queue>

class Minimap {
private:
    struct MinimapChunkData {
        std::array<std::array<mce::Color, 16>, 16> colors;
    };

    std::unordered_map<int64_t, MinimapChunkData> mChunkData;
    std::unordered_set<int64_t> mChunksToMap;

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
    The max number of new chunks that should be read per frame
    */
    int mMaxChunksPerFrame = 1;

    /*
    Should the transparency of blocks be considered?
    */
    bool mUseTranslucency = true;

public:
    void Update(ClientInstance* client);
    void Render(ScreenView* screenView, MinecraftUIRenderContext* ctx);
    void OnLeaveGame();

private:
    void _AddChunksInRenderDistance(ClientInstance* client);
    void _MapChunk(ChunkPos chunkPos, BlockSource* region);
};
