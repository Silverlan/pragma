// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "luasystem.h"
#include "pragma/networking/s_nwm_util.h"
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/lua/classes/lnetpacket.h>

module pragma.server.scripting.lua.classes.net_packet;

void Lua::NetPacket::Server::register_class(luabind::class_<::NetPacket> &classDef)
{
	Lua::NetPacket::register_class(classDef);
	classDef.def("WriteSoundSource", &WriteALSound);
	classDef.def("WriteUniqueEntity", static_cast<void (*)(lua_State *, ::NetPacket &, BaseEntity *)>(&WriteUniqueEntity));
	classDef.def("WriteUniqueEntity", static_cast<void (*)(lua_State *, ::NetPacket &)>(&WriteUniqueEntity));
}

void Lua::NetPacket::Server::WriteALSound(lua_State *, ::NetPacket &packet, std::shared_ptr<ALSound> snd)
{
	unsigned int idx = snd->GetIndex();
	packet->Write<unsigned int>(idx);
}

void Lua::NetPacket::Server::WriteUniqueEntity(lua_State *l, ::NetPacket &packet, BaseEntity *hEnt)
{
	//LUA_CHECK_ENTITY(l,hEnt);
	if(hEnt == nullptr) {
		nwm::write_unique_entity(packet, nullptr);
		return;
	}
	nwm::write_unique_entity(packet, hEnt);
}

void Lua::NetPacket::Server::WriteUniqueEntity(lua_State *, ::NetPacket &packet) { nwm::write_unique_entity(packet, nullptr); }
