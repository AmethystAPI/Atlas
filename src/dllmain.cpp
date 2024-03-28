#include "dllmain.h"
#include "minimap/Minimap.h"

Minimap* minimap;
bool hasAddedRegionListener = false;

void OnRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext)
{
    ClientInstance* client = uiRenderContext->mClient;
    Tessellator* tes = &uiRenderContext->mScreenContext->tessellator;
    if (uiRenderContext->mClient == nullptr || client->getRegion() == nullptr) return;

    // Ensure we have a minimap
    if (!minimap) 
    {
        minimap = new Minimap(client, tes);
    }

    if (screenView->visualTree->mRootControlName->layerName == "hud_screen") 
    {
        if (!hasAddedRegionListener) {
            client->getRegion()->addListener(*minimap);
            hasAddedRegionListener = true;
        }

        minimap->Render(uiRenderContext);
    }
}

void BeforeModShutdown() {
    if (minimap == nullptr) return;
    ClientInstance* client = minimap->mClient;

    if (client != nullptr && client->getRegion() != nullptr) {
        client->getRegion()->removeListener(*minimap);
    }

    delete minimap;
    minimap = nullptr;
}

void OnRequestLeaveGame() {
    minimap->ClearCache();
    hasAddedRegionListener = false;
}

ModFunction void Initialize(AmethystContext* ctx)
{
    ctx->mEventManager.onRenderUI.AddListener(&OnRenderUi);
    ctx->mEventManager.beforeModShutdown.AddListener(&BeforeModShutdown);
    ctx->mEventManager.onRequestLeaveGame.AddListener(&OnRequestLeaveGame);
}