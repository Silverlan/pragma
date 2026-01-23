// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.net_packet;

import :networking.util;

void Lua::NetPacket::Server::register_class(luabind::class_<::NetPacket> &classDef)
{
	NetPacket::register_class(classDef);
	classDef.def("WriteSoundSource", &WriteALSound);
	classDef.def("WriteUniqueEntity", static_cast<void (*)(lua::State *, ::NetPacket &, pragma::ecs::BaseEntity *)>(&WriteUniqueEntity));
	classDef.def("WriteUniqueEntity", static_cast<void (*)(lua::State *, ::NetPacket &)>(&WriteUniqueEntity));
}

void Lua::NetPacket::Server::WriteALSound(lua::State *, ::NetPacket &packet, std::shared_ptr<pragma::audio::ALSound> snd)
{
	unsigned int idx = snd->GetIndex();
	packet->Write<unsigned int>(idx);
}

void Lua::NetPacket::Server::WriteUniqueEntity(lua::State *l, ::NetPacket &packet, pragma::ecs::BaseEntity *hEnt)
{
	//LUA_CHECK_ENTITY(l,hEnt);
	if(hEnt == nullptr) {
		pragma::networking::write_unique_entity(packet, nullptr);
		return;
	}
	pragma::networking::write_unique_entity(packet, hEnt);
}

void Lua::NetPacket::Server::WriteUniqueEntity(lua::State *, ::NetPacket &packet) { pragma::networking::write_unique_entity(packet, nullptr); }
