/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/classes/s_lentity.h"
#include "pragma/entities/s_baseentity.h"
#include "luasystem.h"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/audio/alsound_type.h>
#include <pragma/networking/enums.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>

void Lua::Entity::Server::register_class(luabind::class_<SBaseEntity, BaseEntity> &classDef)
{
	classDef.def("IsShared", &SBaseEntity::IsShared);
	classDef.def("SetShared", &SBaseEntity::SetShared);
	classDef.def("SendNetEvent", static_cast<void (*)(lua_State *, SBaseEntity &, nwm::Protocol, unsigned int, ::NetPacket, pragma::networking::TargetRecipientFilter &)>(&SendNetEvent));
	classDef.def("BroadcastNetEvent", static_cast<void (*)(lua_State *, SBaseEntity &, nwm::Protocol, unsigned int, ::NetPacket)>(&SendNetEvent));
	classDef.def("SendNetEvent", static_cast<void (*)(lua_State *, SBaseEntity &, nwm::Protocol, unsigned int, pragma::networking::TargetRecipientFilter &)>(&SendNetEvent));
	classDef.def("BroadcastNetEvent", static_cast<void (*)(lua_State *, SBaseEntity &, nwm::Protocol, unsigned int)>(&SendNetEvent));
	classDef.def("IsSynchronized", &SBaseEntity::IsSynchronized);
	classDef.def("SetSynchronized", &SBaseEntity::SetSynchronized);
	classDef.def("SetSnapshotDirty", &SBaseEntity::MarkForSnapshot);
	classDef.def("AddNetworkedComponent", &SBaseEntity::AddNetworkedComponent);
}

void Lua::Entity::Server::SendNetEvent(lua_State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, ::NetPacket packet, pragma::networking::TargetRecipientFilter &rp)
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
void Lua::Entity::Server::SendNetEvent(lua_State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, NetPacket packet)
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
void Lua::Entity::Server::SendNetEvent(lua_State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, pragma::networking::TargetRecipientFilter &rp) { SendNetEvent(l, ent, protocol, eventId, {}, rp); }
void Lua::Entity::Server::SendNetEvent(lua_State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId) { SendNetEvent(l, ent, protocol, eventId, {}); }
