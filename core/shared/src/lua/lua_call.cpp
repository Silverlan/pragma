// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/lua/lua_call.hpp"
#include <interface/scripting/lua/lua.hpp>

// import pragma.scripting.lua;

Lua::StatusCode Lua::CallFunction(lua_State *l, const std::function<Lua::StatusCode(lua_State *)> &pushArgs, int32_t numReturnValues) { return pragma::scripting::lua::protected_call(l, pushArgs, numReturnValues); }
