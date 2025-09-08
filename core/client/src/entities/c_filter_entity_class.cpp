// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/c_filter_entity_class.h"
#include "pragma/game/c_game.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern CGame *s_game;

void CFilterClassComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CFilterEntityClass::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFilterClassComponent>();
}
