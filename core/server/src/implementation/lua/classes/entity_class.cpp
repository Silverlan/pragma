// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.entity;

import :entities;
import pragma.util;

void Lua::Entity::Server::register_class(luabind::class_<SBaseEntity, pragma::ecs::BaseEntity> &classDef)
{
	classDef.def("IsShared", &SBaseEntity::IsShared);
	classDef.def("SetShared", &SBaseEntity::SetShared);
	classDef.def("SendNetEvent", static_cast<void (*)(lua::State *, SBaseEntity &, nwm::Protocol, unsigned int, ::NetPacket, pragma::networking::TargetRecipientFilter &)>(&SendNetEvent));
	classDef.def("BroadcastNetEvent", static_cast<void (*)(lua::State *, SBaseEntity &, nwm::Protocol, unsigned int, ::NetPacket)>(&SendNetEvent));
	classDef.def("SendNetEvent", static_cast<void (*)(lua::State *, SBaseEntity &, nwm::Protocol, unsigned int, pragma::networking::TargetRecipientFilter &)>(&SendNetEvent));
	classDef.def("BroadcastNetEvent", static_cast<void (*)(lua::State *, SBaseEntity &, nwm::Protocol, unsigned int)>(&SendNetEvent));
	classDef.def("IsSynchronized", &SBaseEntity::IsSynchronized);
	classDef.def("SetSynchronized", &SBaseEntity::SetSynchronized);
	classDef.def("SetSnapshotDirty", &SBaseEntity::MarkForSnapshot);
	classDef.def("AddNetworkedComponent", &SBaseEntity::AddNetworkedComponent);
}

void Lua::Entity::Server::SendNetEvent(lua::State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, ::NetPacket packet, pragma::networking::TargetRecipientFilter &rp)
{
	switch(static_cast<nwm::Protocol>(protocol)) {
	case nwm::Protocol::TCP:
		static_cast<SBaseEntity &>(ent).SendNetEvent(eventId, packet, pragma::networking::Protocol::SlowReliable, rp);
		break;
	case nwm::Protocol::UDP:
		static_cast<SBaseEntity &>(ent).SendNetEvent(eventId, packet, pragma::networking::Protocol::FastUnreliable, rp);
		break;
	}
}
void Lua::Entity::Server::SendNetEvent(lua::State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, ::NetPacket packet)
{
	switch(static_cast<nwm::Protocol>(protocol)) {
	case nwm::Protocol::TCP:
		static_cast<SBaseEntity &>(ent).SendNetEvent(eventId, packet, pragma::networking::Protocol::SlowReliable);
		break;
	case nwm::Protocol::UDP:
		static_cast<SBaseEntity &>(ent).SendNetEvent(eventId, packet, pragma::networking::Protocol::FastUnreliable);
		break;
	}
}
void Lua::Entity::Server::SendNetEvent(lua::State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, pragma::networking::TargetRecipientFilter &rp) { SendNetEvent(l, ent, protocol, eventId, {}, rp); }
void Lua::Entity::Server::SendNetEvent(lua::State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId) { SendNetEvent(l, ent, protocol, eventId, {}); }
