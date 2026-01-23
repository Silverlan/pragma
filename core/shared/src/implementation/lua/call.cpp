// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.call;

Lua::StatusCode Lua::CallFunction(lua::State *l, const std::function<StatusCode(lua::State *)> &pushArgs, int32_t numReturnValues) { return pragma::scripting::lua_core::protected_call(l, pushArgs, numReturnValues); }
