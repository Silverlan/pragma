// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "luasystem.h"

module pragma.server.scripting.lua.classes.ai_squad;

import pragma.server.ai;

namespace Lua {
	namespace AISquad {
		static luabind::tableT<BaseEntity> GetMembers(lua_State *l, ::AISquad &squad);
	};
};

void Lua::AISquad::register_class(lua_State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<::AISquad>("Squad");
	classDef.def("GetMembers", &GetMembers);
	mod[classDef];
}

luabind::tableT<BaseEntity> Lua::AISquad::GetMembers(lua_State *l, ::AISquad &squad)
{
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto &member : squad.members) {
		if(member.valid() == false)
			continue;
		t[idx++] = member->GetLuaObject();
	}
	return t;
}
