// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.converters.utf8_string;

void luabind::default_converter<pragma::string::Utf8String>::to_lua(lua::State *L, pragma::string::Utf8String const &x) { object {L, std::string {x}}.push(L); }

void luabind::default_converter<pragma::string::Utf8String>::to_lua(lua::State *L, pragma::string::Utf8String *x)
{
	if(!x)
		Lua::PushNil(L);
	else
		to_lua(L, *x);
}
