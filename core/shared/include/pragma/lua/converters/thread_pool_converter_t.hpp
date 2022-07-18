/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "pragma/lua/converters/thread_pool_converter.hpp"
#include <luabind/object.hpp>

#ifndef __LUA_THREAD_POOL_CONVERTER_T_HPP__
#define __LUA_THREAD_POOL_CONVERTER_T_HPP__

template <typename U>
pragma::lua::LuaThreadWrapper luabind::default_converter<pragma::lua::LuaThreadWrapper>::to_cpp(lua_State* L, U u, int index)
{
	if(Lua::IsType<pragma::lua::LuaThreadTask>(L,index))
		return pragma::lua::LuaThreadWrapper{Lua::Check<std::shared_ptr<pragma::lua::LuaThreadTask>>(L,index)};
	return pragma::lua::LuaThreadWrapper{Lua::Check<pragma::lua::LuaThreadPool>(L,index)};
}

template <class U>
int luabind::default_converter<pragma::lua::LuaThreadWrapper>::match(lua_State *l, U u, int index)
{
	return Lua::IsType<pragma::lua::LuaThreadTask>(l,index) ? 1 :
		Lua::IsType<pragma::lua::LuaThreadPool>(l,index) ? 1 : no_match;
}

#endif
