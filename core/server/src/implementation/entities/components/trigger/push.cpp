// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>

module pragma.server.entities.components.triggers.push;

using namespace pragma;

void STriggerPushComponent::Initialize()
{
	BaseTriggerPushComponent::Initialize();

	SetTickPolicy(TickPolicy::Always); // TODO
}

///////

void STriggerPushComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerPush::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STriggerPushComponent>();
}
