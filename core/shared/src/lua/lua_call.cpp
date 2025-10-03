
#include "pragma/lua/lua_call.hpp"

// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/lua/luaapi.h"

import pragma.shared;

Lua::StatusCode Lua::CallFunction(lua_State *l, const std::function<Lua::StatusCode(lua_State *)> &pushArgs, int32_t numReturnValues) { return pragma::scripting::lua::protected_call(l, pushArgs, numReturnValues); }
