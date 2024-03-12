//
// Created by adria on 18/02/2024.
//
#pragma once
#include <Windows.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>
#include <amethyst/events/EventManager.h>
#include <optional>

class HeightMapCalc {
public:
    static std::optional<int> getHeightMap(int x, int z, BlockSource* region);
};

