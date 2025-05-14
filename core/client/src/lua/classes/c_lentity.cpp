/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/lua/classes/c_lentity.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/model/c_model.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <sharedutils/netpacket.hpp>
#include <networkmanager/interface/nwm_manager.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

extern DLLCLIENT CEngine *c_engine;

void Lua::Entity::Client::register_class(luabind::class_<CBaseEntity, BaseEntity> &classDef)
{
	classDef.add_static_constant("EVENT_ON_SCENE_FLAGS_CHANGED", CBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED);
	classDef.def("IsClientsideOnly", &CBaseEntity::IsClientsideOnly);
	classDef.def("GetClientIndex", &CBaseEntity::GetClientIndex);
	classDef.def("SendNetEvent", static_cast<void (*)(lua_State *, CBaseEntity &, nwm::Protocol, unsigned int, const NetPacket &)>(&SendNetEvent));
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

void Lua::Entity::Client::SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, const NetPacket &packet)
{
	switch(protocol) {
	case nwm::Protocol::TCP:
		ent.SendNetEventTCP(eventId, const_cast<NetPacket &>(packet));
		break;
	case nwm::Protocol::UDP:
		ent.SendNetEventUDP(eventId, const_cast<NetPacket &>(packet));
		break;
	}
}
void Lua::Entity::Client::SendNetEvent(lua_State *l, CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId) { SendNetEvent(l, ent, protocol, eventId, {}); }
