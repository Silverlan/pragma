// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <functional>

export module pragma.shared:scripting.lua.call;

export import :scripting.lua.api;

export namespace Lua {
	DLLNETWORK Lua::StatusCode CallFunction(lua_State *l, const std::function<Lua::StatusCode(lua_State *)> &pushArgs, int32_t numReturnValues = 0);
};
