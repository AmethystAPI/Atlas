//
// Created by adria on 18/02/2024.
//
#pragma once
#include <Windows.h>
#include <optional>
#include <minecraft/src/common/world/level/BlockSource.h>

class HeightMapCalc {
public:
    static std::optional<int> getHeightMap(int x, int z, BlockSource* region);
};

