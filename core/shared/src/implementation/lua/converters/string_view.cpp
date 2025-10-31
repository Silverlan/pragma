// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;


#include "pragma/lua/core.hpp"


module pragma.shared;

import :scripting.lua.converters.string_view;

void luabind::default_converter<std::string_view>::to_lua(lua_State *L, std::string_view const &x) { luabind::object {L, std::string {x}}.push(L); }

void luabind::default_converter<std::string_view>::to_lua(lua_State *L, std::string_view *x)
{
	if(!x)
		Lua::PushNil(L);
	else
		to_lua(L, *x);
}
