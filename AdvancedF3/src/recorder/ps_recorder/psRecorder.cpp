//
// Created by Adrian on 12/02/2024.
//

#include "psRecorder.h"
#include <chrono>
int ttt = 0;

int tick = 0;
int update = 0;
int render_cycle = 0;
std::chrono::time_point<std::chrono::system_clock> last_reset;

void psRecorder::registerEventHandlers(Amethyst::EventManager *eventManager) {
    last_reset = std::chrono::system_clock::now();

    // to be added later ~ we don't have a tick func anymore
    eventManager->update.AddListener(onTick);
    eventManager->onRenderUI.AddListener(onRenderUI);
    eventManager->update.AddListener(onUpdate);
}

void psRecorder::onTick() {
    if (ttt >= 12){
        tick++;
        ttt = 0;
    }
    else {
        ttt++;
    }
}

void psRecorder::onRenderUI(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext) {
    if (screenView->visualTree->mRootControlName->layerName == "hud_screen"){
        render_cycle++;
    }
}

void psRecorder::onUpdate(){
    update++;
}

Vec3 psRecorder::getData() {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    auto duration = now - last_reset;
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    Log::Info("TICK: {}; RENDER-CYCLES: {}; MILLIS: {}; SECONDS: {}", tick, render_cycle, millis, millis / 1000);

    float tps = static_cast<float>(tick) / (static_cast<float>(millis) / 1000);
    float fps = static_cast<float>(render_cycle) / (static_cast<float>(millis) / 1000);
    float ups = static_cast<float>(update) / (static_cast<float>(millis) / 1000);

    tick = 0;
    render_cycle = 0;
    update = 0;

    last_reset = std::chrono::system_clock::now();

    return {tps, fps, ups};
}

