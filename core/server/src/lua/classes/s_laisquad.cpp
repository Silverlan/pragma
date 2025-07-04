// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/s_laisquad.h"
#include "luasystem.h"
#include "pragma/ai/ai_squad.h"

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
