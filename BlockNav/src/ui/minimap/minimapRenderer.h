//
// Created by Adrian on 11/02/2024.
//
#pragma once
#include <Windows.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>
#include <amethyst/events/EventManager.h>

class MiniMapRenderer {
public:
    static void Renderer(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext, ClientInstance* clientInstance);
};
