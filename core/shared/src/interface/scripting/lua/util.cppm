// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include <string>
#include <functional>
#include <luasystem.h>

export module pragma.scripting.lua:util;

export namespace pragma::scripting::lua {
	DLLNETWORK Lua::StatusCode protected_call(lua_State *l, const std::function<Lua::StatusCode(lua_State *)> &pushFuncArgs, int32_t numResults, std::string *optOutErrMsg = nullptr);
	DLLNETWORK Lua::StatusCode protected_call(lua_State *l, int32_t numArgs = 0, int32_t numResults = 0, std::string *optOutErrMsg = nullptr);
	DLLNETWORK Lua::StatusCode run_string(lua_State *l, const std::string &str, const std::string &chunkName, int32_t numResults = 0, std::string *optOutErrMsg = nullptr);

	namespace util {
		DLLNETWORK luabind::detail::function_object *get_function_object(luabind::object const &fn);
	};
};
