// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"
#include <sharedutils/util_string.h>

module pragma.server.entities.components.triggers.touch;

using namespace pragma;

void STouchComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerTouch::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STouchComponent>();
}
