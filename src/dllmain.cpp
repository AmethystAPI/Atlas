#include "dllmain.hpp"
#include "minimap/Minimap.hpp"
#include <amethyst/runtime/events/GameEvents.hpp>
#include <amethyst/runtime/events/InputEvents.hpp>
#include <amethyst/runtime/events/ModEvents.hpp>
#include <amethyst/runtime/events/RenderingEvents.hpp>
#include <chrono>
#include <mc/src-client/common/client/gui/ScreenView.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-client/common/client/gui/gui/VisualTree.hpp>
#include <mc/src/common/Minecraft.hpp>
#include <mc/src/common/world/level/BlockSource.hpp>

std::shared_ptr<Minimap> minimap;
bool isInWorld = false;

void AfterRenderUi(AfterRenderUIEvent& event)
{
	// Ensure the game is loaded and a minimap has been constructed
	if (!minimap) return;

	// Only render the UI on the "hud_screen" element.
	if (event.screen.visualTree->mRootControlName->mName == "hud_screen") {
		minimap->Render(event.ctx);
	}
}

void DestroyMinimap()
{
	ClientInstance* client = Amethyst::GetContext().mClientCtx->mClientInstance;
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

void RegisterInputs(RegisterInputsEvent& event)
{
	Amethyst::InputAction& zoom_in = event.inputManager.RegisterNewInput("map_zoom_in", { 0xBB });
	Amethyst::InputAction& zoom_out = event.inputManager.RegisterNewInput("map_zoom_out", { 0xBD });

	zoom_in.addButtonDownHandler(
		[](FocusImpact focus, ClientInstance& ci) {
			if (!minimap || !isInWorld || minimap->mRenderDistance - 1 <= 2) return Amethyst::InputPassthrough::Passthrough;

			minimap->mRenderDistance--;
			return Amethyst::InputPassthrough::ModOnly;
		});

	zoom_out.addButtonDownHandler(
		[](FocusImpact focus, ClientInstance& ci) {
			if (!minimap || !isInWorld || minimap->mRenderDistance >= 20) return Amethyst::InputPassthrough::Passthrough;

			minimap->mRenderDistance++;
			return Amethyst::InputPassthrough::ModOnly;
		});
}

void OnStartJoinGame(OnStartJoinGameEvent& event)
{
	isInWorld = true;
}

void OnLevelConstructed(OnLevelConstructedEvent& event)
{
	// Only use the client side level
	if (!event.mLevel.isClientSide) return;

	minimap = std::make_shared<Minimap>();
	event.mLevel.addListener(*minimap);
}

ModFunction void Initialize(AmethystContext& ctx, const Amethyst::Mod& mod)
{
	Amethyst::InitializeAmethystMod(ctx, mod);

	auto& events = Amethyst::GetEventBus();
	events.AddListener<OnRequestLeaveGameEvent>(&OnRequestLeaveGame);
	events.AddListener<BeforeModShutdownEvent>(&BeforeModShutdown);
	events.AddListener<AfterRenderUIEvent>(&AfterRenderUi);
	events.AddListener<RegisterInputsEvent>(&RegisterInputs);
	events.AddListener<OnStartJoinGameEvent>(&OnStartJoinGame);
	events.AddListener<OnLevelConstructedEvent>(&OnLevelConstructed);
}