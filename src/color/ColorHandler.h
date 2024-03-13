//
// Created by adria on 12/03/2024.
//
#pragma once
#include <Windows.h>
#include <amethyst/runtime/AmethystContext.h>

class ColorHandler {
public:
    static mce::Color getColor(BlockSource* source, int x, int z, bool translucent);
};
