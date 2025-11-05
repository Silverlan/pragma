// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

module pragma.server;
import :entities.components.filter.entity_class;

import :game;

using namespace pragma;

void SFilterClassComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FilterEntityClass::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFilterClassComponent>();
}
