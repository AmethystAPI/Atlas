#include "dllmain.h"
#include "minimap/Minimap.h"

Minimap* minimap;

void OnRenderUi(ScreenView* screenView, MinecraftUIRenderContext* uiRenderContext)
{
    ClientInstance* client = uiRenderContext->mClient;
    Tessellator* tes = &uiRenderContext->mScreenContext->tessellator;
    if (uiRenderContext->mClient == nullptr || client->getRegion() == nullptr) return;

    // Ensure we have a minimap
    if (!minimap) {
        minimap = new Minimap(client, tes);

        for (int x = 0; x < minimap->mRenderDistance; x++) {
            for (int z = 0; z < minimap->mRenderDistance; z++) {
                minimap->UpdateChunk(ChunkPos(x, z));
            }
        }
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