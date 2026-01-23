// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.classes.resource;

export import pragma.shared;

export namespace Lua {
	namespace resource {
		DLLSERVER bool add_file(const std::string &res, bool stream);
		DLLSERVER bool add_file(const std::string &res);
		DLLSERVER bool add_lua_file(const std::string &f);
		DLLSERVER LuaTableObject get_list(lua::State *l);
	};
};
