// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.classes.entity;
import :engine;

void Lua::Entity::Client::register_class(luabind::class_<pragma::ecs::CBaseEntity, pragma::ecs::BaseEntity> &classDef)
{
	classDef.add_static_constant("EVENT_ON_SCENE_FLAGS_CHANGED", cBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED);
	classDef.def("IsClientsideOnly", &pragma::ecs::CBaseEntity::IsClientsideOnly);
	classDef.def("GetClientIndex", &pragma::ecs::CBaseEntity::GetClientIndex);
	classDef.def("SendNetEvent", static_cast<void (*)(lua::State *, pragma::ecs::CBaseEntity &, nwm::Protocol, unsigned int, const ::NetPacket &)>(&SendNetEvent));
	classDef.def("SendNetEvent", static_cast<void (*)(lua::State *, pragma::ecs::CBaseEntity &, nwm::Protocol, unsigned int)>(&SendNetEvent));

	classDef.def("GetSceneFlags", &pragma::ecs::CBaseEntity::GetSceneFlags);
	classDef.def("AddToScene", &pragma::ecs::CBaseEntity::AddToScene);
	classDef.def(
	  "SyncScenes", +[](pragma::ecs::CBaseEntity &ent, const pragma::ecs::CBaseEntity &entOther) {
		  ent.RemoveFromAllScenes();
		  for(auto *scene : entOther.GetScenes())
			  ent.AddToScene(*scene);
	  });
	classDef.def("RemoveFromScene", &pragma::ecs::CBaseEntity::RemoveFromScene);
	classDef.def("RemoveFromAllScenes", &pragma::ecs::CBaseEntity::RemoveFromAllScenes);
	classDef.def("IsInScene", &pragma::ecs::CBaseEntity::IsInScene);

	classDef.def("AddChild", &pragma::ecs::CBaseEntity::AddChild);
}

void Lua::Entity::Client::SendNetEvent(lua::State *l, pragma::ecs::CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, const ::NetPacket &packet)
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
void Lua::Entity::Client::SendNetEvent(lua::State *l, pragma::ecs::CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId) { SendNetEvent(l, ent, protocol, eventId, {}); }
