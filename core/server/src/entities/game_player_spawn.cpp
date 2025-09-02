// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/game_player_spawn.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

import pragma.server.entities.components;

using namespace pragma;

LINK_ENTITY_TO_CLASS(game_player_spawn, GamePlayerSpawn);

void SPlayerSpawnComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void GamePlayerSpawn::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPlayerSpawnComponent>();
	AddComponent<STransformComponent>();
}
