// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.entities.components.prop.dynamic;

using namespace pragma;

void SPropDynamicComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PropDynamic::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPropDynamicComponent>();
}
