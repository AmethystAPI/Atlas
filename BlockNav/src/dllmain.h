#pragma once
#include <Windows.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>
#include <amethyst/events/EventManager.h>

#define ModFunction extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

void onStartJoinGame(ClientInstance* client);
void onRequestLeaveGame();

void onRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext);

void toggleMapVisibility(FocusImpact focus, IClientInstance clientInstance);