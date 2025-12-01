// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.call;

export import :scripting.lua.api;

export namespace Lua {
	DLLNETWORK Lua::StatusCode CallFunction(lua::State *l, const std::function<Lua::StatusCode(lua::State *)> &pushArgs, int32_t numReturnValues = 0);
};
