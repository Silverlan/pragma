// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/lua/classes/lresource.h"
#include "pragma/serverdefinitions.h"
#include "pragma/networking/resourcemanager.h"

bool Lua::resource::add_file(const std::string &res, bool stream) { return ResourceManager::AddResource(res, stream); }
bool Lua::resource::add_file(const std::string &res) { return add_file(res, true); }

bool Lua::resource::add_lua_file(const std::string &f)
{
	auto path = Lua::GetIncludePath(f);
	return ResourceManager::AddResource(Lua::SCRIPT_DIRECTORY_SLASH + path);
}

LuaTableObject Lua::resource::get_list(lua_State *l)
{
	auto &resources = ResourceManager::GetResources();
	auto result = luabind::newtable(l);
	for(auto i = decltype(resources.size()) {0}; i < resources.size(); ++i)
		result[i] = resources[i].fileName;
	return result;
}
