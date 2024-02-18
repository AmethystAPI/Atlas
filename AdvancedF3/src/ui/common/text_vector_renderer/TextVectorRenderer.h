//
// Created by adria on 14/02/2024.
//
#pragma once
#include <Windows.h>
#include <amethyst/events/EventManager.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>

class TextVectorRenderer {
public:
    static void TextVectorTopLeftRenderer(ScreenView *screenView, MinecraftUIRenderContext *uiRenderContext, std::vector<std::string> data, Vec2 uiScreenSize);
    static void TextVectorTopRightRenderer(ScreenView *screenView, MinecraftUIRenderContext *uiRenderContext, std::vector<std::string> data, Vec2 uiScreenSize);

    static void TextVectorBottomLeftRenderer(ScreenView *screenView, MinecraftUIRenderContext *uiRenderContext, std::vector<std::string> data, Vec2 uiScreenSize);
    static void TextVectorBottomRightRenderer(ScreenView *screenView, MinecraftUIRenderContext *uiRenderContext, std::vector<std::string> data, Vec2 uiScreenSize);

    static void TextVectorEntireLeftRenderer(ScreenView *screenView, MinecraftUIRenderContext *uiRenderContext, std::vector<std::string> data, Vec2 uiScreenSize);
    static void TextVectorEntireRightRenderer(ScreenView *screenView, MinecraftUIRenderContext *uiRenderContext, std::vector<std::string> data, Vec2 uiScreenSize);
};
