//
// Created by adria on 18/02/2024.
//
#pragma once
#include <Windows.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>
#include <amethyst/events/EventManager.h>

class NormalMode {
public:
    static mce::Color getColorFromPos(int x, int player_y, int z, BlockSource* region);
};

