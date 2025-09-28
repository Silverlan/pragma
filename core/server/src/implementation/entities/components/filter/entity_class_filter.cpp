// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "sharedutils/netpacket.hpp"

module pragma.server.entities.components.filter.entity_class;

import pragma.server.game;

using namespace pragma;

void SFilterClassComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FilterEntityClass::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFilterClassComponent>();
}
