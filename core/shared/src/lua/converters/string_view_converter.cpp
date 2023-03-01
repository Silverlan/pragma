/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "pragma/lua/converters/string_view_converter.hpp"
#include <luabind/object.hpp>

void luabind::default_converter<std::string_view>::to_lua(lua_State *L, std::string_view const &x) { luabind::object {L, std::string {x}}.push(L); }

void luabind::default_converter<std::string_view>::to_lua(lua_State *L, std::string_view *x)
{
	if(!x)
		lua_pushnil(L);
	else
		to_lua(L, *x);
}
