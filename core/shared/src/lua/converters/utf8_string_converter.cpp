
#include "pragma/lua/luaapi.h"

// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
