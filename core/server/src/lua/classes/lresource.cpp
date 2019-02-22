#include "stdafx_server.h"
#include "pragma/lua/classes/lresource.h"
#include "pragma/serverdefinitions.h"
#include "pragma/networking/resourcemanager.h"

int Lua::resource::add_file(lua_State *l)
{
	auto *res = Lua::CheckString(l,1);
	auto bStream = true;
	if(Lua::IsSet(l,2))
		bStream = Lua::CheckBool(l,2);
	Lua::PushBool(l,ResourceManager::AddResource(res,bStream));
	return 1;
}

int Lua::resource::add_lua_file(lua_State *l)
{
	auto *f = Lua::CheckString(l,1);
	auto path = Lua::GetIncludePath(f);
	Lua::PushBool(l,ResourceManager::AddResource("lua\\" +path));
	return 1;
}

int Lua::resource::get_list(lua_State *l)
{
	auto &resources = ResourceManager::GetResources();
	auto t = Lua::CreateTable(l);
	for(auto i=decltype(resources.size()){0};i<resources.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::PushString(l,resources[i].fileName);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
