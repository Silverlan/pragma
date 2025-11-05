// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.converters.utf8_string;

export import pragma.lua;
export import pragma.string.unicode;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<pragma::string::Utf8String> : native_converter_base<pragma::string::Utf8String> {
		enum { consumed_args = 1 };

		template<typename U>
		pragma::string::Utf8String to_cpp(lua::State *L, U u, int index);

		template<class U>
		static int match(lua::State *l, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, pragma::string::Utf8String const &x);
		void to_lua(lua::State *L, pragma::string::Utf8String *x);
	  public:
		static value_type to_cpp_deferred(lua::State *, int);
		static void to_lua_deferred(lua::State *, param_type) {}
		static int compute_score(lua::State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const pragma::string::Utf8String> : default_converter<pragma::string::Utf8String> {};

	template<>
	struct DLLNETWORK default_converter<pragma::string::Utf8String const &> : default_converter<pragma::string::Utf8String> {};

	template<>
	struct DLLNETWORK default_converter<pragma::string::Utf8String &&> : default_converter<pragma::string::Utf8String> {};
}

export namespace luabind {
	default_converter<pragma::string::Utf8String>::value_type default_converter<pragma::string::Utf8String>::to_cpp_deferred(lua::State *, int) { return {}; }

	template<typename U>
	pragma::string::Utf8String default_converter<pragma::string::Utf8String>::to_cpp(lua::State *L, U u, int index)
	{
		return {Lua::CheckString(L, index)};
	}

	template<class U>
	int default_converter<pragma::string::Utf8String>::match(lua::State *l, U u, int index)
	{
		return Lua::IsString(l, index) ? 1 : no_match;
	}
}
