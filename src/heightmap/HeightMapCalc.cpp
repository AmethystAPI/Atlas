//
// Created by adria on 18/02/2024.
//

#include "HeightMapCalc.h"
#include <minecraft/src/common/world/level/block/Block.h>


std::optional<int> HeightMapCalc::getHeightMap(int x, int z, BlockSource* region)
{
    int max_y = region->getMaxHeight();
    int min_y = region->getMinHeight();

    for (int y = max_y; y > min_y; --y) {
        Block block = region->getBlock(x, y, z);

        if (block.mLegacyBlock->mID != 0) {
            return y;
        }
        else {
            continue;
        }

    }

    return std::nullopt;
}