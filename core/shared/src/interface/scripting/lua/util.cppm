// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include <string>
#include <functional>
#include <luasystem.h>

export module pragma.scripting.lua.util;

export namespace pragma::scripting::lua {
    DLLNETWORK Lua::StatusCode protected_call(lua_State *l, const std::function<Lua::StatusCode(lua_State *)> &pushFuncArgs, int32_t numResults);
};
