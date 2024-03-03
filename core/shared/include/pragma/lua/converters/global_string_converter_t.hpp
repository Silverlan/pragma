/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#include "pragma/lua/converters/global_string_converter.hpp"
#include <luabind/object.hpp>

#ifndef __LUA_GLOBAL_STRING_CONVERTER_T_HPP__
#define __LUA_GLOBAL_STRING_CONVERTER_T_HPP__

template<typename U>
pragma::GString luabind::default_converter<pragma::GString>::to_cpp(lua_State *L, U u, int index)
{
	return {luaL_checkstring(L, index)};
}

template<class U>
int luabind::default_converter<pragma::GString>::match(lua_State *l, U u, int index)
{
	return lua_isstring(l, index) ? 1 : no_match;
}

#endif
