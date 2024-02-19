//
// Created by Adrian on 11/02/2024.
//

#include "minimapRenderer.h"
#include "../config/UiConfig.h"
#include "modes_renderer/NormalMode.h"
#include "modes_renderer/SliceDepthMode.h"
#include "modes_renderer/SliceMode.h"
#include <minecraft/src/common/world/level/BlockPos.h>
#include <minecraft/src/common/world/level/block/Block.h>
#include <minecraft/src/common/world/level/block/BlockLegacy.h>
#include <minecraft/src-client/common/client/renderer/helpers/MeshHelpers.h>

// The vertexes for two triangles on a unit square
// In clock-wise order
Vec3 vertexes[6] = {
    Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f),
    Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f)
};

void MiniMapRenderer::Renderer(ScreenView* screenView, MinecraftUIRenderContext* ctx)
{
    ClientInstance* ci = ctx->mClient;
    LocalPlayer* player = ci->getLocalPlayer();

    // Still loading into the world!
    if (player == nullptr) return;

    // Minimap options
    Vec2 minimapScreenSize = Vec2(128.0f, 128.0f);
    int widthInBlocks = 128*2;
    float pixelsPerBlock = (float)minimapScreenSize.x / widthInBlocks;

    // Placement of the minimap on the screen
    Vec2 uiScreenSize = ci->guiData->clientUIScreenSize;
    float left = uiScreenSize.x - minimapScreenSize.x - UiConfig::offset;
    float bottom = UiConfig::offset + minimapScreenSize.y;

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
        for (int y = 0; y < widthInBlocks; y++) {
            float pixelLeft = left + x * pixelsPerBlock;
            float pixelBottom = bottom - y * pixelsPerBlock;

            // This data urgently needs to be cached, this is not good
            mce::Color color = NormalMode::getColorFromPosCached(mapMinX + x, playerY, mapMinZ + y, region);
            tes->color(color.r, color.g, color.b, 1);

            for (Vec3 vertex : vertexes) {
                float vertexX = vertex.x * pixelsPerBlock + pixelLeft;
                float vertexY = vertex.y * pixelsPerBlock + pixelBottom;

                tes->vertex(vertexX, vertexY, 0);
            }
        }
    }

    // We need to find another material because this one goes black when it is showing a hover outline
    mce::MaterialPtr* mat = *reinterpret_cast<mce::MaterialPtr**>(SlideAddress(0x572A440));
    MeshHelpers::renderMeshImmediately(ctx->mScreenContext, tes, mat);
}