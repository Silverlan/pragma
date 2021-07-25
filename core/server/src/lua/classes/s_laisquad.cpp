/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/s_laisquad.h"
#include "luasystem.h"
#include "pragma/ai/ai_squad.h"
#include "pragma/lua/s_lentity_handles.hpp"

namespace Lua
{
	namespace AISquad
	{
		static void GetMembers(lua_State *l,SAIHandle &hEnt,::AISquad &squad);
	};
};

void Lua::AISquad::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<::AISquad>("Squad");
	classDef.def("GetMembers",&GetMembers);
	mod[classDef];
}

void Lua::AISquad::GetMembers(lua_State *l,SAIHandle&,::AISquad &squad)
{
	auto t = Lua::CreateTable(l);
	uint32_t idx = 1;
	for(auto &member : squad.members)
	{
		if(member.valid() == false)
			continue;
		Lua::PushInt(l,idx++);
		member->GetLuaObject()->push(l);
		Lua::SetTableValue(l,t);
	}
}
