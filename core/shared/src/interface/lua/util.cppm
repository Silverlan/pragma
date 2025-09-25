// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module pragma.shared:lua.util;

export namespace Lua {
	DLLNETWORK void StackDump(lua_State *lua);
	DLLNETWORK std::optional<std::string> StackToString(lua_State *lua);
	DLLNETWORK std::optional<std::string> TableToString(lua_State *lua, int n = -1);
	DLLNETWORK void TableDump(lua_State *lua, int n = -1);
	DLLNETWORK void VarDump(lua_State *lua, int n = -1);
	DLLNETWORK std::optional<std::string> VarToString(lua_State *lua, int n = -1);
	class Interface;
}
