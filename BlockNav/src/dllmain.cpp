#include "dllmain.h"
#include "ui/minimap/minimapRenderer.h"

bool map_open = true;

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(HookManager* hookManager, Amethyst::EventManager* eventManager, InputManager* inputManager)
{

    // Add a listener to key inputs
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

    // Inputs don't seem to work with amethysts hot reloading
    // Removing for now... will add back later!
    /*inputManager->RegisterInput("use_map", 0x4D);
    inputManager->AddButtonDownHandler("use_map", toggleMapVisibility);*/

    // Add a listener to a built-in amethyst event
    eventManager->onRequestLeaveGame.AddListener(onRequestLeaveGame);
    eventManager->onRenderUI.AddListener(&onRenderUi);
}

void onRequestLeaveGame()
{
    map_open = false;
}

void onRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext)
{
    if (screenView->visualTree->mRootControlName->layerName == "hud_screen" && map_open) {
        MiniMapRenderer::Renderer(screenView, uiRenderContext);
    }
}

void toggleMapVisibility(FocusImpact focus, IClientInstance clientInstance)
{
    map_open = !map_open;
}