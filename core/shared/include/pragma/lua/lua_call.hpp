// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_CALL_HPP__
#define __LUA_CALL_HPP__

#include <pragma/lua/lua_error_handling.hpp>

namespace Lua {
	inline Lua::StatusCode CallFunction(lua_State *l, const std::function<Lua::StatusCode(lua_State *)> &pushArgs, int32_t numReturnValues = 0) { return Lua::ProtectedCall(l, pushArgs, numReturnValues, Lua::HandleTracebackError); }
};

#endif
