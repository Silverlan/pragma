// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"
#include <sharedutils/util_string.h>

module pragma.server.entities.components.triggers.hurt;

using namespace pragma;

void STriggerHurtComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerHurt::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STriggerHurtComponent>();
}
