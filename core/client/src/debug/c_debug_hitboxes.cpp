// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/console/c_cvar_global_functions.h"
#include <pragma/model/model.h>
#include <pragma/console/sh_cmd.h>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

import pragma.client.debug;
import pragma.client.entities.components;

extern DLLCLIENT CGame *c_game;

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
	if(c_game == nullptr || pl == nullptr)
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
