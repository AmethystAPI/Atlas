//
// Created by adria on 18/02/2024.
//

#include "NormalMode.h"
#include "../../heightmap/HeightMapCalc.h"
#include <minecraft/src/common/world/level/BlockPos.h>
#include <minecraft/src/common/world/level/block/Block.h>
#include <map>

std::map<std::pair<int, int>, mce::Color> cache;

mce::Color NormalMode::getColorFromPos(int x, int player_y, int z, BlockSource* region)
{
    int heightmap_y = HeightMapCalc::getHeightMap(x, z, region);

    BlockPos pos = BlockPos(x, heightmap_y, z);

    Block block = region->getBlock(pos);

    mce::Color color = block.mLegacyBlock->getMapColor(*region, pos, block);

    return color;
}

mce::Color NormalMode::getColorFromPosCached(int x, int player_y, int z, BlockSource* region)
{
    auto val = cache.find(std::make_pair(x, z));

    // Value wasn't found in Cache
    if (val == cache.end()){
        mce::Color color = NormalMode::getColorFromPos(x, player_y, z, region);

        std::pair<int, int> coords = std::make_pair(x, z);
        cache.insert(std::make_pair(coords, color));

        return color;

    }
    else {
        return val->second;
    }
}