// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.resource;

import :networking.resource_manager;

bool Lua::resource::add_file(const std::string &res, bool stream) { return pragma::networking::ResourceManager::AddResource(res, stream); }
bool Lua::resource::add_file(const std::string &res) { return add_file(res, true); }

bool Lua::resource::add_lua_file(const std::string &f)
{
	auto path = GetIncludePath(f);
	return pragma::networking::ResourceManager::AddResource(SCRIPT_DIRECTORY_SLASH + path);
}

LuaTableObject Lua::resource::get_list(lua::State *l)
{
	auto &resources = pragma::networking::ResourceManager::GetResources();
	auto result = luabind::newtable(l);
	for(auto i = decltype(resources.size()) {0}; i < resources.size(); ++i)
		result[i] = resources[i].fileName;
	return result;
}
