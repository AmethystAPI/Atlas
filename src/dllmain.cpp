#include "dllmain.h"
#include "minimap/minimapRenderer.h"

Minimap minimap;
bool map_open = true;

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(AmethystContext* ctx)
{
    /*inputManager->RegisterInput("use_map", 0x4D);
    inputManager->AddButtonDownHandler("use_map", toggleMapVisibility);*/

    // Add a listener to a built-in amethyst event
    ctx->mEventManager.onRenderUI.AddListener(&onRenderUi);
    ctx->mEventManager.onStartJoinGame.AddListener(&onStartJoinGame);
    ctx->mEventManager.onRequestLeaveGame.AddListener(&onRequestLeaveGame);
}

void onStartJoinGame(ClientInstance* client) {
    minimap.OnJoinGame();
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
}

void toggleMapVisibility(FocusImpact focus, IClientInstance clientInstance)
{
    map_open = !map_open;
}