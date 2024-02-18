//
// Created by Adrian on 11/02/2024.
//

#include "minimapRenderer.h"
#include "../config/UiConfig.h"
#include <minecraft/src/common/world/level/BlockPos.h>
#include <minecraft/src/common/world/level/block/Block.h>
#include <minecraft/src/common/world/level/block/BlockLegacy.h>

void MiniMapRenderer::Renderer(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext, ClientInstance* clientInstance)
{
    Vec2 uiScreenSize = clientInstance->guiData->clientUIScreenSize;
    Vec3* playerPos = clientInstance->getLocalPlayer()->getPosition();

    BlockSource* region = clientInstance->getRegion();

    int offset = UiConfig::offset;
    mce::Color* bg_color = UiConfig::background_color;

    Vec2 size = Vec2(125, 125);

    // make smaller to get more detail
    float detail = 2;

    float start_x = uiScreenSize.x - size.x - offset;
    float start_y = static_cast<float>(offset);
    float end_x = uiScreenSize.x - offset;
    float end_y = size.y + offset;

    auto bg_area = RectangleArea(
        start_x,
        end_x,
        start_y,
        end_y);

    uiRenderContext->drawRectangle(
        &bg_area,
        bg_color,
        UiConfig::background_color_alpha,
        0);

    int max_y = static_cast<int>((size.y - static_cast<float>(offset) * 2) / detail);
    int max_x = static_cast<int>((size.x - static_cast<float>(offset) * 2) / detail);

    for (int y = 0; y < max_y; ++y) {
        for (int x = 0; x < max_x; ++x) {
            const BlockPos pos = BlockPos(
                static_cast<int>(playerPos->x) - x,
                static_cast<int>(playerPos->y)-2,
                static_cast<int>(playerPos->z) - y
            );

            const Block block = region->getBlock(pos);

            mce::Color color = block.mLegacyBlock->getMapColor(*region, pos, block);

            auto area = RectangleArea(
                start_x + static_cast<float>(offset) + static_cast<float>(x * detail),
                start_x + static_cast<float>(offset) + static_cast<float>((x + 1) * detail),
                start_y + static_cast<float>(offset) + static_cast<float>(y * detail),
                start_y + static_cast<float>(offset) + static_cast<float>((y + 1) * detail));

            uiRenderContext->drawRectangle(
                &area,
                &color,
                1.0f,
                10);
        }
    }
}