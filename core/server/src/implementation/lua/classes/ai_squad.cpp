// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.ai_squad;

import :ai;

namespace Lua {
	namespace AISquad {
		static luabind::tableT<pragma::ecs::BaseEntity> GetMembers(lua::State *l, ::AISquad &squad);
	};
};

void Lua::AISquad::register_class(lua::State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<::AISquad>("Squad");
	classDef.def("GetMembers", &GetMembers);
	mod[classDef];
}

luabind::tableT<pragma::ecs::BaseEntity> Lua::AISquad::GetMembers(lua::State *l, ::AISquad &squad)
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
