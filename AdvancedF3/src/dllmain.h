#pragma once
#include <Windows.h>
#include <amethyst/events/EventManager.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>

#include <iomanip>
#include <iostream>
#include <vector>

#define ModFunction extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

void onRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext);

void onUseF3(FocusImpact focus, IClientInstance clientInstance);