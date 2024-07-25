#include "dllmain.h"
#include "minimap/Minimap.h"
#include <amethyst/runtime/events/GameEvents.hpp>
#include <amethyst/runtime/events/RenderingEvents.hpp>
#include <amethyst/runtime/events/ModEvents.hpp>
#include <minecraft/src/common/world/level/BlockSource.hpp>
#include <minecraft/src-client/common/client/gui/ScreenView.hpp>
#include <minecraft/src-client/common/client/gui/gui/VisualTree.hpp>
#include <minecraft/src-client/common/client/gui/gui/UIControl.hpp>
#include <amethyst/runtime/events/InputEvents.hpp>
#include <chrono>

std::shared_ptr<Minimap> minimap;
bool hasAddedRegionListener = false;

void AfterRenderUi(AfterRenderUIEvent& event)
{
    ClientInstance* client = event.ctx.mClient;

    // Ensure the game is loaded.
    if (event.ctx.mClient == nullptr || client->getRegion() == nullptr) return;

    Tessellator* tes = &event.ctx.mScreenContext->tessellator;

    // Ensure we have a minimap
    [[unlikely]]
    if (!minimap)
    {
        minimap = std::make_shared<Minimap>(event.ctx);
    }

    if (event.screen.visualTree->mRootControlName->mName == "hud_screen") 
    {
        [[unlikely]]
        if (!hasAddedRegionListener) {
            client->getRegion()->mLevel->addListener(*minimap);
            hasAddedRegionListener = true;
        }

        //auto start = std::chrono::high_resolution_clock::now();
        minimap->Render(event.ctx);
        //auto end = std::chrono::high_resolution_clock::now();

        //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        //Log::Info("Minimap::Render took: {}ms", duration.count());
    }
}

void BeforeModShutdown(BeforeModShutdownEvent& event)
{
    if (minimap == nullptr) return;
    
    ClientInstance* client = Amethyst::GetContext().mClientInstance;

    if (client != nullptr && client->getRegion() != nullptr) {
        client->getRegion()->removeListener(*minimap);
    }

    minimap.reset();
}

void OnRequestLeaveGame(OnRequestLeaveGameEvent& event)
{
    minimap->DeleteAllChunkMeshes();
    hasAddedRegionListener = false;
}

void RegisterInputs(RegisterInputsEvent& event) {
    event.inputManager.RegisterNewInput("map_zoom_in", { 0xBB });
    event.inputManager.RegisterNewInput("map_zoom_out", { 0xBD });
}

void OnStartJoinGame(OnStartJoinGameEvent& event) {
    auto& inputs = *Amethyst::GetContext().mInputManager;

    inputs.AddButtonDownHandler(
        "map_zoom_in", [](FocusImpact focus, IClientInstance& ci) {
            if (!minimap) return;
            if (minimap->mRenderDistance - 1 <= 2) return;
            minimap->mRenderDistance--;
        },
        false);

    inputs.AddButtonDownHandler(
        "map_zoom_out", [](FocusImpact focus, IClientInstance& ci) {
            minimap->mRenderDistance++;
        },
        false);
}

ModFunction void Initialize(AmethystContext& ctx)
{
    Amethyst::InitializeAmethystMod(ctx);

    Amethyst::GetContext().mFeatures->enableInputSystem = true;

    auto& events = Amethyst::GetEventBus();
    events.AddListener<OnRequestLeaveGameEvent>(&OnRequestLeaveGame);
    events.AddListener<BeforeModShutdownEvent>(&BeforeModShutdown);
    events.AddListener<AfterRenderUIEvent>(&AfterRenderUi);
    events.AddListener<RegisterInputsEvent>(&RegisterInputs);
    events.AddListener<OnStartJoinGameEvent>(&OnStartJoinGame);
} 