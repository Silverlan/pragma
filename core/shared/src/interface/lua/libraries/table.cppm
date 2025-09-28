// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.shared:scripting.lua.libraries.table;

export namespace Lua {
	namespace table {
		DLLNETWORK int has_value(lua_State *l);
		DLLNETWORK int random(lua_State *l);
		DLLNETWORK int is_empty(lua_State *l);
		DLLNETWORK int count(lua_State *l);
	};
};
