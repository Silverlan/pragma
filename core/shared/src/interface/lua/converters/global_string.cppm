// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.converters.global_string;

export import pragma.lua;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<pragma::util::GString> : native_converter_base<pragma::util::GString> {
		enum { consumed_args = 1 };

		template<typename U>
		pragma::util::GString to_cpp(lua::State *L, U u, int index);

		template<class U>
		static int match(lua::State *l, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, pragma::util::GString const &x);
		void to_lua(lua::State *L, pragma::util::GString *x);
	  public:
		static value_type to_cpp_deferred(lua::State *, int) { return {}; }
		static void to_lua_deferred(lua::State *, param_type) {}
		static int compute_score(lua::State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const pragma::util::GString> : default_converter<pragma::util::GString> {};

	template<>
	struct DLLNETWORK default_converter<pragma::util::GString const &> : default_converter<pragma::util::GString> {};

	template<>
	struct DLLNETWORK default_converter<pragma::util::GString &&> : default_converter<pragma::util::GString> {};
}

export namespace luabind {
	template<typename U>
	pragma::util::GString default_converter<pragma::util::GString>::to_cpp(lua::State *L, U u, int index)
	{
		return {Lua::CheckString(L, index)};
	}

	template<class U>
	int default_converter<pragma::util::GString>::match(lua::State *l, U u, int index)
	{
		return Lua::IsString(l, index) ? 1 : no_match;
	}
}
