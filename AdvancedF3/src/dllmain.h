#pragma once
#include <Windows.h>
#include <amethyst/events/EventManager.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>

#include <hwinfo/hwinfo.h>
#include <iomanip>
#include <iostream>
#include <vector>
#include "hwinfo/utils/PCIMapper.h"

#define ModFunction extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

void OnStartJoinGame(ClientInstance* client);
void onRequestLeaveGame();

void onRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext);

void onUseF3(FocusImpact focus, IClientInstance clientInstance);