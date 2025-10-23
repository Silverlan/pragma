// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/core.hpp"
#include <string>

module pragma.shared;

import :scripting.lua.call;

Lua::StatusCode Lua::CallFunction(lua_State *l, const std::function<Lua::StatusCode(lua_State *)> &pushArgs, int32_t numReturnValues) { return ::pragma::scripting::lua::protected_call(l, pushArgs, numReturnValues); }
