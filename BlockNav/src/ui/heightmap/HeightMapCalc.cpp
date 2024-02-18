//
// Created by adria on 18/02/2024.
//

#include "HeightMapCalc.h"
#include <minecraft/src/common/world/level/block/Block.h>

int HeightMapCalc::getHeightMap(int x, int z, BlockSource* region)
{
    int max_y = region->getMaxHeight();
    int min_y = region->getMinHeight();

    int diff = max_y - min_y;

    for (int y = diff; y > 0; --y) {
        Block block = region->getBlock(x, y, z);

        if (block.mLegacyBlock->mID != 0) {
            return y;
        }
        else {
            continue;
        }
    }

    return min_y;
}