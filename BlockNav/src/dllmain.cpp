#include "dllmain.h"
#include "ui/minimap/minimapRenderer.h"

Minimap minimap;
bool map_open = true;

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(HookManager* hookManager, Amethyst::EventManager* eventManager, InputManager* inputManager)
{
    /*inputManager->RegisterInput("use_map", 0x4D);
    inputManager->AddButtonDownHandler("use_map", toggleMapVisibility);*/

    // Add a listener to a built-in amethyst event
    eventManager->onRequestLeaveGame.AddListener(&onRequestLeaveGame);
    eventManager->onRenderUI.AddListener(&onRenderUi);
}

void onRequestLeaveGame()
{
    minimap.OnLeaveGame();
}

void onRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext)
{
    if (screenView->visualTree->mRootControlName->layerName == "hud_screen" && map_open) {
        minimap.Render(screenView, uiRenderContext);
    }

    minimap.Update(uiRenderContext->mClient);
}

void toggleMapVisibility(FocusImpact focus, IClientInstance clientInstance)
{
    map_open = !map_open;
}