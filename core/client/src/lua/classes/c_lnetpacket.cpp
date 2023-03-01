/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lnetpacket.h"
#include "luasystem.h"
#include "pragma/networking/c_nwm_util.h"
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/lua/classes/lnetpacket.h>
#include <pragma/lua/lua_call.hpp>

void Lua::NetPacket::Client::register_class(luabind::class_<::NetPacket> &classDef)
{
	Lua::NetPacket::register_class(classDef);
	classDef.def("ReadUniqueEntity", &ReadUniqueEntity);
}

void Lua::NetPacket::Client::ReadUniqueEntity(lua_State *l, ::NetPacket &packet, luabind::object o)
{
	Lua::CheckFunction(l, 2);
	auto cb = FunctionCallback<void, BaseEntity *>::Create([l, o](BaseEntity *ent) {
		Lua::CallFunction(
		  l,
		  [&o, ent](lua_State *l) {
			  o.push(l);
			  ent->GetLuaObject().push(l);
			  return Lua::StatusCode::Ok;
		  },
		  0);
	});
	nwm::read_unique_entity(packet, cb);
	Lua::Push<CallbackHandle>(l, cb);
}
