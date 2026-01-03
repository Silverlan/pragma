// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :console.commands;

static void debug_hitboxes(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	static auto g_debugOverlayEnabled = false;
	if(g_debugOverlayEnabled) {
		g_debugOverlayEnabled = false;
		auto *game = state->GetGameState();
		if(!game)
			return;
		auto it = pragma::ecs::EntityIterator {*game};
		it.AttachFilter<TEntityIteratorFilterComponent<pragma::CDebugHitboxComponent>>();
		for(auto *ent : it)
			ent->RemoveComponent<pragma::CDebugHitboxComponent>();
		return;
	}
	if(pragma::get_cgame() == nullptr || pl == nullptr)
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	auto ents = pragma::console::find_target_entity(state, *charComponent, argv);
	if(ents.empty()) {
		Con::CWAR << "No entity targets found!" << Con::endl;
		return;
	}
	g_debugOverlayEnabled = true;
	for(auto *ent : ents)
		ent->AddComponent<pragma::CDebugHitboxComponent>();
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_hitboxes", &debug_hitboxes, pragma::console::ConVarFlags::None, "Displays the hitboxes for the entity under the crosshair, or the entity with the given name (If it was specified.).");
}
