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
#include <minecraft/src/common/Minecraft.hpp>

std::shared_ptr<Minimap> minimap;
bool isInWorld = false;

void AfterRenderUi(AfterRenderUIEvent& event)
{
    // Ensure the game is loaded and a minimap has been constructed
    if (!minimap) return;

    // Only render the UI on the "hud_screen" element.
    if (event.screen.visualTree->mRootControlName->mName == "hud_screen") 
    {
        minimap->Render(event.ctx);
    }
}

void DestroyMinimap() {
    ClientInstance* client = Amethyst::GetContext().mClientInstance;
    BlockSource* region = client->getRegion();

    region->removeListener(*minimap);
    minimap.reset();
}

void BeforeModShutdown(BeforeModShutdownEvent& event)
{
    DestroyMinimap();
}

void OnRequestLeaveGame(OnRequestLeaveGameEvent& event)
{
    isInWorld = false;
    DestroyMinimap();
}

void RegisterInputs(RegisterInputsEvent& event) {
    event.inputManager.RegisterNewInput("map_zoom_in", { 0xBB });
    event.inputManager.RegisterNewInput("map_zoom_out", { 0xBD });
}

void OnStartJoinGame(OnStartJoinGameEvent& event) {
    isInWorld = true;
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

void OnLevelConstructed(OnLevelConstructedEvent& event)
{
    // Only use the client side level
    if (!event.mLevel.isClientSide) return;

    minimap = std::make_shared<Minimap>();
    event.mLevel.addListener(*minimap);
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
    events.AddListener<OnLevelConstructedEvent>(&OnLevelConstructed);
} 