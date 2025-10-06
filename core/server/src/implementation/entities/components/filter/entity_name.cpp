// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.entities.components.filter.entity_name;

import pragma.server.game;

using namespace pragma;

void SFilterNameComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FilterEntityName::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFilterNameComponent>();
}
