//
// Created by adria on 12/02/2024.
//
#pragma once
#include <Windows.h>
#include <amethyst/events/EventManager.h>
#include <amethyst/HookManager.h>
#include <amethyst/InputManager.h>

class f3DataBuilder {
public:
    static std::vector<std::string> BuildInfoData(ClientInstance* clientInstance);
    static std::vector<std::string> BuildControlData(ClientInstance *clientInstance);
};


