// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :networking.util;
import :scripting.lua.classes.net_packet;

void Lua::NetPacket::Client::register_class(luabind::class_<::NetPacket> &classDef)
{
	Lua::NetPacket::register_class(classDef);
	classDef.def("ReadUniqueEntity", &ReadUniqueEntity);
}

void Lua::NetPacket::Client::ReadUniqueEntity(lua::State *l, ::NetPacket &packet, luabind::object o)
{
	Lua::CheckFunction(l, 2);
	auto cb = FunctionCallback<void, pragma::ecs::BaseEntity *>::Create([l, o](pragma::ecs::BaseEntity *ent) {
		Lua::CallFunction(
		  l,
		  [&o, ent](lua::State *l) {
			  o.push(l);
			  ent->GetLuaObject().push(l);
			  return Lua::StatusCode::Ok;
		  },
		  0);
	});
	pragma::networking::read_unique_entity(packet, cb);
	Lua::Push<CallbackHandle>(l, cb);
}
