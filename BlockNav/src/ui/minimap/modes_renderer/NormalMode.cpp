//
// Created by adria on 18/02/2024.
//

#include "NormalMode.h"
#include "../../heightmap/HeightMapCalc.h"
#include <minecraft/src/common/world/level/BlockPos.h>
#include <minecraft/src/common/world/level/block/Block.h>

mce::Color NormalMode::getColorFromPos(int x, int player_y, int z, BlockSource* region)
{
    int heightmap_y = HeightMapCalc::getHeightMap(x, z, region);

    BlockPos pos = BlockPos(x, heightmap_y, z);

    Block block = region->getBlock(pos);

    mce::Color color = block.mLegacyBlock->getMapColor(*region, pos, block);

    return color;
}
