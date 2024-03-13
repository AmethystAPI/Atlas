#include "dllmain.h"
#include "minimap/Minimap.h"

Minimap* minimap;

void OnRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext)
{
    ClientInstance* client = uiRenderContext->mClient;
    Tessellator* tes = &uiRenderContext->mScreenContext->tessellator;
    if (uiRenderContext->mClient == nullptr) return;

    // Ensure we have a minimap
    if (!minimap) {
        minimap = new Minimap(client, tes);
        minimap->UpdateChunk(ChunkPos(0, 0));
    }

    if (screenView->visualTree->mRootControlName->layerName == "hud_screen") 
    {
        minimap->Render(uiRenderContext);
    }
}

ModFunction void Initialize(AmethystContext* ctx)
{
    ctx->mEventManager.onRenderUI.AddListener(&OnRenderUi);
}