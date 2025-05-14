/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan */

#include "pragma/lua/converters/utf8_string_converter.hpp"
#include <luabind/object.hpp>

#ifndef __LUA_UTF8_STRING_CONVERTER_T_HPP__
#define __LUA_UTF8_STRING_CONVERTER_T_HPP__

luabind::default_converter<pragma::string::Utf8String>::value_type luabind::default_converter<pragma::string::Utf8String>::to_cpp_deferred(lua_State *, int)
{
	return {};
}

template<typename U>
pragma::string::Utf8String luabind::default_converter<pragma::string::Utf8String>::to_cpp(lua_State *L, U u, int index)
{
	return {luaL_checkstring(L, index)};
}

template<class U>
int luabind::default_converter<pragma::string::Utf8String>::match(lua_State *l, U u, int index)
{
	return lua_isstring(l, index) ? 1 : no_match;
}

#endif
