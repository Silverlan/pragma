/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_debug_hitbox_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debugoverlay.h"
#include <pragma/model/model.h>
#include <pragma/console/sh_cmd.h>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

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
