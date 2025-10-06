// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.entities.components.player_spawn;

import pragma.server.entities.components;

using namespace pragma;

void SPlayerSpawnComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void GamePlayerSpawn::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPlayerSpawnComponent>();
	AddComponent<STransformComponent>();
}
