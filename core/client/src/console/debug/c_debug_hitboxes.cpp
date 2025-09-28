// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"

import pragma.client;


void Console::commands::debug_hitboxes(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	static auto g_debugOverlayEnabled = false;
	if(g_debugOverlayEnabled) {
		g_debugOverlayEnabled = false;
		auto *game = state->GetGameState();
		if(!game)
			return;
		auto it = EntityIterator {*game};
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
	auto ents = command::find_target_entity(state, *charComponent, argv);
	if(ents.empty()) {
		Con::cwar << "No entity targets found!" << Con::endl;
		return;
	}
	g_debugOverlayEnabled = true;
	for(auto *ent : ents)
		ent->AddComponent<pragma::CDebugHitboxComponent>();
}
