//
// Created by Adrian on 11/02/2024.
//

#include "minimapRenderer.h"
#include "../config/UiConfig.h"
#include <minecraft/src/common/world/level/BlockPos.h>
#include <minecraft/src/common/world/level/block/Block.h>
#include <minecraft/src/common/world/level/block/BlockLegacy.h>
#include <minecraft/src-client/common/client/renderer/helpers/MeshHelpers.h>
#include "../heightmap/HeightMapCalc.h"
#include <algorithm>

int frameCounter = 0;

void Minimap::Update(ClientInstance* ci) {
    if (ci->getLocalPlayer() == nullptr) return;

    frameCounter++;

    // Only add new chunks every so often
    if (frameCounter >= 30) {
        frameCounter = 0;
        _AddChunksInRenderDistance(ci);
    };

    int chunksMappedThisFrame = 0;
    
    while (this->mChunksToMap.size() > 0 && chunksMappedThisFrame < mMaxChunksPerFrame) {
        auto it = this->mChunksToMap.begin();
        int64_t packed = *it;
        this->mChunksToMap.erase(it);

        ChunkPos chunkPos(packed);
        this->_MapChunk(chunkPos, ci->getRegion());
    }
}

void Minimap::_MapChunk(ChunkPos chunkPos, BlockSource* region) {
    MinimapChunkData chunkData;

    for (int localX = 0; localX < 16; localX++) {
        for (int localZ = 0; localZ < 16; localZ++) {
            int worldX = chunkPos.x * 16 + localX;
            int worldZ = chunkPos.z * 16 + localZ;
            int worldY = HeightMapCalc::getHeightMap(worldX, worldZ, region);

            BlockPos pos = BlockPos(worldX, worldY, worldZ);
            const Block block = region->getBlock(pos);
            mce::Color color = block.mLegacyBlock->getMapColor(*region, pos, block);
            float translucency = block.mLegacyBlock->mMaterial.mTranslucency;

            // While we haven't found an opaque block
            while (mUseTranslucency && translucency > 0.0f) {
                worldY--;

                if (worldY < region->getMinHeight()) break;

                // Get the colour and translucency of the below block
                BlockPos belowPos(worldX, worldY, worldZ);
                const Block belowBlock = region->getBlock(belowPos);
                mce::Color belowColor = belowBlock.mLegacyBlock->getMapColor(*region, belowPos, belowBlock);
                float belowTranslucency = belowBlock.mLegacyBlock->mMaterial.mTranslucency;

                if (belowBlock.mLegacyBlock->mMaterial.mType == MaterialType::Water) {
                    belowTranslucency = 0.4f;
                }

                // If a block is fully transparent below other blocks than that would 
                // result in a *0, so just skip this from happening
                if (belowTranslucency == 1.0f) {
                    continue;
                }

                // Calculate blending factors based on translucency
                float blendFactor = 1.0f - translucency;
                float belowBlendFactor = translucency * (1.0f - belowTranslucency);

                // Blend colors
                color.r = color.r * blendFactor + belowColor.r * belowBlendFactor;
                color.g = color.g * blendFactor + belowColor.g * belowBlendFactor;
                color.b = color.b * blendFactor + belowColor.b * belowBlendFactor;

                // Update translucency for the next block
                translucency = belowTranslucency;
            }

            color.a = 1.0f;

            chunkData.colors[localX][localZ] = color;
        }
    }

    this->mChunkData[chunkPos.packed] = chunkData;
}

void Minimap::_AddChunksInRenderDistance(ClientInstance* ci) {
    LocalPlayer* player = ci->getLocalPlayer();
    if (player == nullptr) return;

    Vec3* pos = player->getPosition();
    int chunkX = (int)pos->x / 16;
    int chunkZ = (int)pos->z / 16;

    for (int x = chunkX - mRenderDistance - mRenderSafeZone; x < chunkX + mRenderDistance + mRenderSafeZone; x++) {
        for (int z = chunkZ - mRenderDistance - mRenderSafeZone; z < chunkZ + mRenderDistance + mRenderSafeZone; z++) {
            ChunkPos chunkPos(x, z);
            auto it = this->mChunkData.find(chunkPos.packed);

            if (it == this->mChunkData.end()) {
                this->mChunksToMap.insert(chunkPos.packed);
            }
        }
    }

    if (this->mChunksToMap.size() < this->mMaxChunksPerFrame) {
        ChunkPos originChunk(chunkX, chunkZ);
        this->mChunksToMap.insert(originChunk.packed);
    }
}

// The vertexes for two triangles on a unit square
// In clock-wise order
Vec3 vertexes[6] = {
    Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f),
    Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f)
};

void Minimap::Render(ScreenView* screenView, MinecraftUIRenderContext* ctx)
{
    ClientInstance* ci = ctx->mClient;
    LocalPlayer* player = ci->getLocalPlayer();

    // Still loading into the world!
    if (player == nullptr) return;

    Vec2 uiScreenSize = ci->guiData->clientUIScreenSize;

    // Minimap options
    Vec2 minimapScreenSize = Vec2(135.0f, 135.0f);
    int widthInBlocks = mRenderDistance * 2 * 16;
    float pixelsPerBlock = (float)minimapScreenSize.x / widthInBlocks;

    // Placement of the minimap on the screen
    float left = uiScreenSize.x - minimapScreenSize.x - UiConfig::offset * 2;
    float bottom = UiConfig::offset * 2 + minimapScreenSize.y;

    RectangleArea bg_area = RectangleArea(
        uiScreenSize.x - UiConfig::offset * 3 - minimapScreenSize.x,
        uiScreenSize.x - UiConfig::offset,
        UiConfig::offset,
        UiConfig::offset * 3 + minimapScreenSize.y
        );

    //Draw Background of Minimap
    ctx->drawRectangle(
        &bg_area,
        UiConfig::background_color,
        UiConfig::background_color_alpha,
        0
        );

    // Position the player in the center of the map
    Vec3* playerPos = player->getPosition();
    int mapMinX = (int)playerPos->x - widthInBlocks / 2;
    int mapMinZ = (int)playerPos->z - widthInBlocks / 2;
    int playerY = (int)playerPos->y;

    BlockSource* region = ci->getRegion();
    Tessellator* tes = &ctx->mScreenContext->tessellator;

    tes->begin(mce::TriangleList, widthInBlocks * widthInBlocks);
    tes->mNoColor = false;

    for (int x = 0; x < widthInBlocks; x++) {
        for (int z = 0; z < widthInBlocks; z++) {
            float pixelLeft = left + x * pixelsPerBlock;
            float pixelBottom = bottom - z * pixelsPerBlock;

            int chunkX = (mapMinX + x) / 16;
            int chunkZ = (mapMinZ + z) / 16;

            mce::Color color(0.0f, 0.0f, 0.0f, 1.0f);

            ChunkPos chunkPos(chunkX, chunkZ);
            auto it = mChunkData.find(chunkPos.packed);

            if (mChunkData.find(chunkPos.packed) != mChunkData.end()) {
                int localX = abs((mapMinX + x) % 16);
                int localZ = abs((mapMinZ + z) % 16);

                color = it->second.colors[localX][localZ];
            }

            tes->color(color.r, color.g, color.b, color.a);

            for (Vec3 vertex : vertexes) {
                float vertexX = vertex.x * pixelsPerBlock + pixelLeft;
                float vertexY = vertex.y * pixelsPerBlock + pixelBottom;

                tes->vertex(vertexX, vertexY, 0);
            }
        }
    }

    // We need to find another material because this one goes black when it is showing a hover outline
    HashedString hashedMaterialName("entity_static");
    mce::MaterialPtr material(*mce::RenderMaterialGroup::switchable, hashedMaterialName);

    MeshHelpers::renderMeshImmediately(ctx->mScreenContext, tes, &material);
}

void Minimap::OnLeaveGame() {
    mChunkData.clear();
    mChunksToMap.clear();
}