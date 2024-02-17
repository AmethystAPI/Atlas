#pragma once
#include <Windows.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>
#include <amethyst/events/EventManager.h>

#include "hwinfo/utils/PCIMapper.h"
#include <hwinfo/hwinfo.h>
#include <iomanip>
#include <iostream>
#include <vector>

#define ModFunction extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

void OnStartJoinGame(ClientInstance* client);
void onRequestLeaveGame();

void onRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext);

void onUseMap(FocusImpact focus, IClientInstance clientInstance);
void onUseF3(FocusImpact focus, IClientInstance clientInstance);