/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __SH_LUA_COMPONENT_WRAPPER_HPP__
#define __SH_LUA_COMPONENT_WRAPPER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/sh_lua_component.hpp"
#include "pragma/lua/lentity_component_lua.hpp"
#include <sharedutils/datastream.h>

struct DLLNETWORK LuaBaseEntityComponentWrapper
	: public BaseLuaBaseEntityHandle,
	public luabind::wrap_base
{
public:
	LuaBaseEntityComponentWrapper(const util::WeakHandle<pragma::BaseEntityComponent> &hComponent);
	LuaBaseEntityComponentWrapper();

	void Initialize() {}
	static void default_Initialize(lua_State *l,LuaBaseEntityComponentWrapper &hComponent) {}

	void OnTick(double dt) {}
	static void default_OnTick(lua_State *l,LuaBaseEntityComponentWrapper &hComponent,double dt) {}

	void OnRemove() {}
	static void default_OnRemove(lua_State *l,LuaBaseEntityComponentWrapper &hComponent) {}

	void OnEntitySpawn() {}
	static void default_OnEntitySpawn(lua_State *l,LuaBaseEntityComponentWrapper &hComponent) {}

	void OnAttachedToEntity() {}
	static void default_OnAttachedToEntity(lua_State *l,LuaBaseEntityComponentWrapper &hComponent) {}

	void OnDetachedToEntity() {}
	static void default_OnDetachedToEntity(lua_State *l,LuaBaseEntityComponentWrapper &hComponent) {}

	void HandleEvent(uint32_t eventId) {}
	static void default_HandleEvent(lua_State *l,LuaBaseEntityComponentWrapper &hComponent,uint32_t eventId) {}

	void Save(DataStream ds) {}
	static void default_Save(lua_State *l,LuaBaseEntityComponentWrapper &hComponent,DataStream ds) {}

	void Load(DataStream ds,uint32_t version) {}
	static void default_Load(lua_State *l,LuaBaseEntityComponentWrapper &hComponent,DataStream ds,uint32_t version) {}
};

#endif
