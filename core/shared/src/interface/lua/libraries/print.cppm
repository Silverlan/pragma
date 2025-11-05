// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.print;

export import pragma.lua;

export {
	namespace Lua {
		DLLNETWORK bool lua_value_to_string(lua::State *L, int arg, int *r, std::string *val);
	};
	namespace Lua::console {
		DLLNETWORK int print(lua::State *l);
		DLLNETWORK int print_table(lua::State *l, std::string tab, int idx = 1);
		DLLNETWORK int print_table(lua::State *l);
		DLLNETWORK int msg(lua::State *l, int st);
		DLLNETWORK int msg(lua::State *l);
		DLLNETWORK int msgn(lua::State *l);
		DLLNETWORK int msgc(lua::State *l);
		DLLNETWORK int msgw(lua::State *l);
		DLLNETWORK int msge(lua::State *l);
	};

	namespace Lua::log {
		DLLNETWORK int info(lua::State *l);
		DLLNETWORK int warn(lua::State *l);
		DLLNETWORK int error(lua::State *l);
		DLLNETWORK int critical(lua::State *l);
		DLLNETWORK int debug(lua::State *l);
		DLLNETWORK int color(lua::State *l);
		DLLNETWORK int register_logger(lua::State *l);
	};

	namespace Lua::debug {
		DLLNETWORK int print(lua::State *l);
	};
};
