#include "dllmain.h"
#include "recorder/ps_recorder/psRecorder.h"
#include "ui/f3/f3Renderer.h"
#include "ui/minimap/minimapRenderer.h"

ClientInstance* client = nullptr;
bool map_open = false;
bool f3_open = false;

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(HookManager* hookManager, Amethyst::EventManager* eventManager, InputManager* inputManager)
{
    // Logging from <Amethyst/Log.h>
    Log::Info("Hello from C++!");

    // Add a listener to key inputs
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    inputManager->RegisterInput("use_map", 0x4D);
    inputManager->AddButtonDownHandler("use_map", onUseMap);

    inputManager->RegisterInput("use_f3", 0x72);
    inputManager->AddButtonDownHandler("use_f3", &onUseF3);

    //// Add a listener to a built-in amethyst event
    eventManager->onStartJoinGame.AddListener(&OnStartJoinGame);
    eventManager->onRequestLeaveGame.AddListener(onRequestLeaveGame);

    eventManager->onRenderUI.AddListener(&onRenderUi);

    psRecorder::registerEventHandlers(eventManager);
}

// Subscribed to amethysts on start join game event in Initialize
void OnStartJoinGame(ClientInstance* clientInstance)
{
    client = clientInstance;

    Log::Info("The player has joined the game!");
}

void onRequestLeaveGame()
{
    Log::Info("The player has top_left the game!");

    map_open = false;
    f3_open = false;
}

void onRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext)
{
    if (screenView->visualTree->mRootControlName->layerName == "hud_screen") {
        if (map_open) {
            MiniMapRenderer::Renderer(screenView, uiRenderContext, client);
        }

        if (f3_open) {
            f3Renderer::Renderer(screenView, uiRenderContext, client);
        }
    }
}

void onUseMap(FocusImpact focus, IClientInstance clientInstance)
{
    map_open = !map_open;
    Log::Info("Map");
}

void onUseF3(FocusImpact focus, IClientInstance clientInstance)
{
    f3_open = !f3_open;
    Log::Info("F3");
}