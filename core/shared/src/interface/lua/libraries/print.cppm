// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

export module pragma.client:scripting.lua.libraries.print;

export {
	namespace Lua {
		DLLNETWORK bool lua_value_to_string(lua_State *L, int arg, int *r, std::string *val);
	};
	namespace Lua::console {
		DLLNETWORK int print(lua_State *l);
		DLLNETWORK int print_table(lua_State *l, std::string tab, int idx = 1);
		DLLNETWORK int print_table(lua_State *l);
		DLLNETWORK int msg(lua_State *l, int st);
		DLLNETWORK int msg(lua_State *l);
		DLLNETWORK int msgn(lua_State *l);
		DLLNETWORK int msgc(lua_State *l);
		DLLNETWORK int msgw(lua_State *l);
		DLLNETWORK int msge(lua_State *l);
	};

	namespace Lua::log {
		DLLNETWORK int info(lua_State *l);
		DLLNETWORK int warn(lua_State *l);
		DLLNETWORK int error(lua_State *l);
		DLLNETWORK int critical(lua_State *l);
		DLLNETWORK int debug(lua_State *l);
		DLLNETWORK int color(lua_State *l);
		DLLNETWORK int register_logger(lua_State *l);
	};

	namespace Lua::debug {
		DLLNETWORK int print(lua_State *l);
	};
};
