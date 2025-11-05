// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.converters.string_view;

export import pragma.lua;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<std::string_view> : native_converter_base<std::string_view> {
		enum { consumed_args = 1 };

		template<typename U>
		std::string_view to_cpp(lua::State *L, U u, int index);

		template<class U>
		static int match(lua::State *l, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, std::string_view const &x);
		void to_lua(lua::State *L, std::string_view *x);
	  public:
		static value_type to_cpp_deferred(lua::State *, int) { return {}; }
		static void to_lua_deferred(lua::State *, param_type) {}
		static int compute_score(lua::State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const std::string_view> : default_converter<std::string_view> {};

	template<>
	struct DLLNETWORK default_converter<std::string_view const &> : default_converter<std::string_view> {};

	template<>
	struct DLLNETWORK default_converter<std::string_view &&> : default_converter<std::string_view> {};
}

export namespace luabind {
	template<typename U>
	std::string_view default_converter<std::string_view>::to_cpp(lua::State *L, U u, int index)
	{
		return {Lua::CheckString(L, index)};
	}

	template<class U>
	int default_converter<std::string_view>::match(lua::State *l, U u, int index)
	{
		return Lua::IsString(l, index) ? 1 : no_match;
	}
}
