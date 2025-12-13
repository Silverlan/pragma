// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.util;

export import pragma.lua;

export namespace Lua {
	DLLNETWORK void StackDump(lua::State *lua);
	DLLNETWORK std::optional<std::string> StackToString(lua::State *lua);
	DLLNETWORK std::optional<std::string> TableToString(lua::State *lua, int n = -1);
	DLLNETWORK void TableDump(lua::State *lua, int n = -1);
	DLLNETWORK void VarDump(lua::State *lua, int n = -1);
	DLLNETWORK std::optional<std::string> VarToString(lua::State *lua, int n = -1);
}

export namespace pragma::scripting::lua_core {
	DLLNETWORK Lua::StatusCode protected_call(lua::State *l, const std::function<Lua::StatusCode(lua::State *)> &pushFuncArgs, int32_t numResults, std::string *optOutErrMsg = nullptr);
	DLLNETWORK Lua::StatusCode protected_call(lua::State *l, int32_t numArgs = 0, int32_t numResults = 0, std::string *optOutErrMsg = nullptr);
	DLLNETWORK Lua::StatusCode run_string(lua::State *l, const std::string &str, const std::string &chunkName, int32_t numResults = 0, std::string *optOutErrMsg = nullptr);

	namespace util {
		DLLNETWORK luabind::detail::function_object *get_function_object(luabind::object const &fn);
	};
};

export namespace Lua {
	DLLNETWORK luabind::detail::class_rep *get_crep(luabind::object o);
	DLLNETWORK void set_ignore_include_cache(bool b);
	DLLNETWORK std::optional<std::string> find_script_file(const std::string &fileName);

	template<class T, class TCast>
	bool get_table_value(lua::State *l, const std::string &name, uint32_t t, TCast &ret, const std::function<T(lua::State *, int32_t)> &check)
	{
		PushString(l, name); /* 1 */
		GetTableValue(l, t);
		auto r = false;
		if(IsSet(l, -1) == true) {
			ret = static_cast<TCast>(check(l, -1));
			r = true;
		}
		Pop(l, 1); /* 0 */
		return r;
	}
	template<typename T>
	void get_table_values(lua::State *l, uint32_t tIdx, std::vector<T> &values, const std::function<T(lua::State *, int32_t)> &tCheck)
	{
		CheckTable(l, tIdx);
		auto numOffsets = GetObjectLength(l, tIdx);
		values.reserve(numOffsets);
		for(auto i = decltype(numOffsets) {0}; i < numOffsets; ++i) {
			PushInt(l, i + 1);
			GetTableValue(l, tIdx);

			values.push_back(tCheck(l, -1));
			Pop(l, 1);
		}
	}
	template<typename T>
	std::vector<T> get_table_values(lua::State *l, uint32_t tIdx, const std::function<T(lua::State *, int32_t)> &tCheck)
	{
		std::vector<T> values;
		get_table_values(l, tIdx, values, tCheck);
		return values;
	}
};

export namespace pragma::LuaCore {
	DLLNETWORK Lua::StatusCode protected_call(lua::State *l, const std::function<Lua::StatusCode(lua::State *)> &pushFuncArgs, int32_t numResults);
};
