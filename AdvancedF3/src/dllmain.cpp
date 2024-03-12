#include "dllmain.h"
#include "ui/f3/f3Renderer.h"
#include "recorder/ps_recorder/psRecorder.h"

bool f3_open = true;

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(HookManager* hookManager, Amethyst::EventManager* eventManager, InputManager* inputManager)
{
    // Logging from <Amethyst/Log.h>
    Log::Info("Hello from C++!");

    // Add a listener to key inputs for opening the f3 screen
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    inputManager->RegisterInput("use_f3", 0x72);
    inputManager->AddButtonDownHandler("use_f3", &onUseF3);

    // Add a listener to a built-in amethyst event
    eventManager->onRenderUI.AddListener(&onRenderUi);

    psRecorder::registerEventHandlers(eventManager);
}

void onRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext)
{
    if (screenView->visualTree->mRootControlName->layerName == "hud_screen" && f3_open) {
        f3Renderer::Renderer(screenView, uiRenderContext);
    }
}

void onUseF3(FocusImpact focus, IClientInstance clientInstance){
    f3_open = !f3_open;
}