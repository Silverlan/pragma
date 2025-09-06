// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/entity_component_system_t.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.server.entities.components.gamemode;

using namespace pragma;

void SGamemodeComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void SGamemode::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SGamemodeComponent>();
}
