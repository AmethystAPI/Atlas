//
// Created by adria on 15/02/2024.
//
#pragma once
#include <Windows.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>
#include <amethyst/events/EventManager.h>
#include <minecraft/src-client/common/client/renderer/helpers/MeshHelpers.h>

void SimpleLineRenderer(MinecraftUIRenderContext* ctx, Vec2 point_1, Vec2 point_2);
void VectorLineRenderer(MinecraftUIRenderContext* ctx, const std::vector<Vec2>& points);