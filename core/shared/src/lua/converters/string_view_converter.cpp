// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
