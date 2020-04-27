/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lentity.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/lentity.h"
#include "luasystem.h"
#include "pragma/model/c_model.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <sharedutils/netpacket.hpp>
#include <networkmanager/interface/nwm_manager.hpp>

extern DLLCENGINE CEngine *c_engine;


void Lua::Entity::Client::register_class(luabind::class_<EntityHandle> &classDef)
{
	::Lua::Entity::register_class(classDef);
	classDef.add_static_constant("EVENT_ON_SCENE_FLAGS_CHANGED",CBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED);
	classDef.def("IsClientsideOnly",&IsClientsideOnly);
	classDef.def("GetClientIndex",&GetClientIndex);
	classDef.def("SendNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t protocol,unsigned int,const NetPacket&)>(&SendNetEvent));
	classDef.def("SendNetEvent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t protocol,unsigned int)>(&SendNetEvent));

	classDef.def("GetSceneFlags",&GetSceneFlags);
	classDef.def("AddToScene",&AddToScene);
	classDef.def("RemoveFromScene",&RemoveFromScene);
	classDef.def("IsInScene",&IsInScene);
}

void Lua::Entity::Client::IsClientsideOnly(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushBool(l,static_cast<CBaseEntity*>(hEnt.get())->IsClientsideOnly());
}

void Lua::Entity::Client::GetClientIndex(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushInt(l,static_cast<CBaseEntity*>(hEnt.get())->GetClientIndex());
}
void Lua::Entity::Client::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId,const NetPacket &packet)
{
	LUA_CHECK_ENTITY(l,hEnt);
	switch(static_cast<nwm::Protocol>(protocol))
	{
		case nwm::Protocol::TCP:
			static_cast<CBaseEntity*>(hEnt.get())->SendNetEventTCP(eventId,const_cast<NetPacket&>(packet));
			break;
		case nwm::Protocol::UDP:
			static_cast<CBaseEntity*>(hEnt.get())->SendNetEventUDP(eventId,const_cast<NetPacket&>(packet));
			break;
	}
}
void Lua::Entity::Client::SendNetEvent(lua_State *l,EntityHandle &hEnt,uint32_t protocol,unsigned int eventId) {SendNetEvent(l,hEnt,protocol,eventId,{});}
void Lua::Entity::Client::GetSceneFlags(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushInt(l,static_cast<CBaseEntity*>(hEnt.get())->GetSceneFlags());
}
void Lua::Entity::Client::AddToScene(lua_State *l,EntityHandle &hEnt,Scene &scene)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<CBaseEntity*>(hEnt.get())->AddToScene(scene);
}
void Lua::Entity::Client::RemoveFromScene(lua_State *l,EntityHandle &hEnt,Scene &scene)
{
	LUA_CHECK_ENTITY(l,hEnt);
	static_cast<CBaseEntity*>(hEnt.get())->RemoveFromScene(scene);
}
void Lua::Entity::Client::IsInScene(lua_State *l,EntityHandle &hEnt,Scene &scene)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushBool(l,static_cast<CBaseEntity*>(hEnt.get())->IsInScene(scene));
}

