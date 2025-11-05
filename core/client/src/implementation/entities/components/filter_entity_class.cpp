// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

module pragma.client;

import :entities.components.filter_entity_class;
using namespace pragma;

void CFilterClassComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CFilterEntityClass::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFilterClassComponent>();
}
