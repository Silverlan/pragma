// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/lua/converters/global_string_converter.hpp"
#include <luabind/object.hpp>

void luabind::default_converter<pragma::GString>::to_lua(lua_State *L, pragma::GString const &x)
{
	std::string str = x;
	luabind::object {L, str}.push(L);
}

void luabind::default_converter<pragma::GString>::to_lua(lua_State *L, pragma::GString *x)
{
	if(!x)
		lua_pushnil(L);
	else {
		std::string str = *x;
		to_lua(L, str);
	}
}
