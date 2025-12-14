// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.converters.global_string;

void luabind::default_converter<pragma::GString>::to_lua(lua::State *L, pragma::GString const &x)
{
	std::string str = x;
	object {L, str}.push(L);
}

void luabind::default_converter<pragma::GString>::to_lua(lua::State *L, pragma::GString *x)
{
	if(!x)
		Lua::PushNil(L);
	else {
		std::string str = *x;
		to_lua(L, str);
	}
}
