/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_CALL_HPP__
#define __LUA_CALL_HPP__

#include <pragma/lua/lua_error_handling.hpp>

namespace Lua
{
	inline Lua::StatusCode CallFunction(lua_State *l,const std::function<Lua::StatusCode(lua_State*)> &pushArgs,int32_t numReturnValues=0)
	{
		return Lua::ProtectedCall(l,pushArgs,numReturnValues,Lua::HandleTracebackError);
	}
};

#endif
