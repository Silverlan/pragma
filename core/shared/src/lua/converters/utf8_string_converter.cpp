/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan */

#include "pragma/lua/converters/utf8_string_converter.hpp"
#include <luabind/object.hpp>

void luabind::default_converter<pragma::string::Utf8String>::to_lua(lua_State *L, pragma::string::Utf8String const &x) { luabind::object {L, std::string {x}}.push(L); }

void luabind::default_converter<pragma::string::Utf8String>::to_lua(lua_State *L, pragma::string::Utf8String *x)
{
	if(!x)
		lua_pushnil(L);
	else
		to_lua(L, *x);
}
