#pragma once
#include <Windows.h>
#include <amethyst/runtime/AmethystContext.h>

#define ModFunction extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}