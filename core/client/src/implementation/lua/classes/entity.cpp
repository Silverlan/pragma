// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "luasystem.h"

module pragma.client;


import :scripting.lua.classes.entity;
import :engine;


void Lua::Entity::Client::register_class(luabind::class_<CBaseEntity, BaseEntity> &classDef)
{
	classDef.add_static_constant("EVENT_ON_SCENE_FLAGS_CHANGED", CBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED);
	classDef.def("IsClientsideOnly", &CBaseEntity::IsClientsideOnly);
	classDef.def("GetClientIndex", &CBaseEntity::GetClientIndex);
	classDef.def("SendNetEvent", static_cast<void (*)(lua_State *, CBaseEntity &, nwm::Protocol, unsigned int, const ::NetPacket &)>(&SendNetEvent));
	classDef.def("SendNetEvent", static_cast<void (*)(lua_State *, CBaseEntity &, nwm::Protocol, unsigned int)>(&SendNetEvent));

	classDef.def("GetSceneFlags", &CBaseEntity::GetSceneFlags);
	classDef.def("AddToScene", &CBaseEntity::AddToScene);
	classDef.def(
	  "SyncScenes", +[](CBaseEntity &ent, const CBaseEntity &entOther) {
		  ent.RemoveFromAllScenes();
		  for(auto *scene : entOther.GetScenes())
			  ent.AddToScene(*scene);
	  });
	classDef.def("RemoveFromScene", &CBaseEntity::RemoveFromScene);
	classDef.def("RemoveFromAllScenes", &CBaseEntity::RemoveFromAllScenes);
	classDef.def("IsInScene", &CBaseEntity::IsInScene);

	classDef.def("AddChild", &CBaseEntity::AddChild);
}

void Lua::Entity::Client::SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, const ::NetPacket &packet)
{
	switch(protocol) {
	case nwm::Protocol::TCP:
		ent.SendNetEventTCP(eventId, const_cast<::NetPacket &>(packet));
		break;
	case nwm::Protocol::UDP:
		ent.SendNetEventUDP(eventId, const_cast<::NetPacket &>(packet));
		break;
	}
}
void Lua::Entity::Client::SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId) { SendNetEvent(l, ent, protocol, eventId, {}); }
