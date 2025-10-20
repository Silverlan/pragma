// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <luabind/detail/conversion_policies/native_converter.hpp>
#include <string_view>

export module pragma.shared:scripting.lua.converters.utf8_string;

import pragma.string.unicode;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<pragma::string::Utf8String> : native_converter_base<pragma::string::Utf8String> {
		enum { consumed_args = 1 };

		template<typename U>
		pragma::string::Utf8String to_cpp(lua_State *L, U u, int index);

		template<class U>
		static int match(lua_State *l, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}

		void to_lua(lua_State *L, pragma::string::Utf8String const &x);
		void to_lua(lua_State *L, pragma::string::Utf8String *x);
	  public:
		static value_type to_cpp_deferred(lua_State *, int);
		static void to_lua_deferred(lua_State *, param_type) {}
		static int compute_score(lua_State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const pragma::string::Utf8String> : default_converter<pragma::string::Utf8String> {};

	template<>
	struct DLLNETWORK default_converter<pragma::string::Utf8String const &> : default_converter<pragma::string::Utf8String> {};

	template<>
	struct DLLNETWORK default_converter<pragma::string::Utf8String &&> : default_converter<pragma::string::Utf8String> {};
}

export {
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
}
