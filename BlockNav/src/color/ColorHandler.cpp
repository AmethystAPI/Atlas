//
// Created by adria on 12/03/2024.
//

#include "ColorHandler.h"
#include "../heightmap/HeightMapCalc.h"
#include <minecraft/src/common/world/level/BlockPos.h>
#include <minecraft/src/common/world/level/block/Block.h>
#include <optional>

mce::Color ColorHandler::getColor(BlockSource* region, int x, int z, bool useTranslucency)
{
    std::optional<int> int_result = HeightMapCalc::getHeightMap(x, z, region);
    int worldY = 0;

    if (int_result.has_value()) {
        worldY = int_result.value();
    }
    else {
        return {0, 0, 0, 1};
    }

    BlockPos pos(x, worldY, z);

    const Block block = region->getBlock(pos);
    mce::Color color = block.mLegacyBlock->getMapColor(*region, pos, block);
    float translucency = block.mLegacyBlock->mMaterial.mTranslucency;

    // While we haven't found an opaque block
    while (useTranslucency && translucency > 0.0f) {
        worldY--;

        if (worldY < region->getMinHeight()) break;

        // Get the colour and translucency of the below block
        BlockPos belowPos = pos.below();
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

    return color;
}