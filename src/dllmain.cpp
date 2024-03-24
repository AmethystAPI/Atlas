#include "dllmain.h"
#include "minimap/Minimap.h"

Minimap* minimap;
bool hasBeenOpenedSinceClear = false;
int frameCount = 0;

void OnRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext)
{
    ClientInstance* client = uiRenderContext->mClient;
    Tessellator* tes = &uiRenderContext->mScreenContext->tessellator;
    if (uiRenderContext->mClient == nullptr || client->getRegion() == nullptr) return;

    frameCount++;

    // Ensure we have a minimap
    if (!minimap) {
        minimap = new Minimap(client, tes);
        client->getRegion()->addListener(*minimap);
    }

    if (screenView->visualTree->mRootControlName->layerName == "hud_screen") 
    {
        hasBeenOpenedSinceClear = true;
        minimap->Render(uiRenderContext);
    }
}

ModFunction void Initialize(AmethystContext* ctx)
{
    ctx->mEventManager.onRenderUI.AddListener(&OnRenderUi);
}