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
		static void GetMembers(lua_State *l,SAIHandle &hEnt,std::shared_ptr<::AISquad> &squad);
	};
};

void Lua::AISquad::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<std::shared_ptr<::AISquad>>("Squad");
	classDef.def("GetMembers",&GetMembers);
	mod[classDef];
}

void Lua::AISquad::GetMembers(lua_State *l,SAIHandle&,std::shared_ptr<::AISquad> &squad)
{
	auto t = Lua::CreateTable(l);
	uint32_t idx = 1;
	for(auto &member : squad->members)
	{
		if(member.IsValid() == false)
			continue;
		Lua::PushInt(l,idx++);
		member->GetLuaObject()->push(l);
		Lua::SetTableValue(l,t);
	}
}
