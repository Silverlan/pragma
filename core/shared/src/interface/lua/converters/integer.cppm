// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.converters.integer;

export import pragma.lua;

export namespace luabind {
	// uint64_t
	template<>
	struct DLLNETWORK default_converter<uint64_t> : native_converter_base<uint64_t> {
		static int compute_score(lua::State *l, int index) { return Lua::GetType(l, index) == Lua::Type::Number ? 0 : -1; }

		uint64_t to_cpp_deferred(lua::State *l, int index) { return static_cast<uint64_t>(Lua::ToNumber(l, index)); }

		void to_lua_deferred(lua::State *l, uint64_t value) { Lua::PushNumber(l, static_cast<double>(value)); }
	};

	template<>
	struct DLLNETWORK default_converter<uint64_t const> : default_converter<uint64_t> {};

	template<>
	struct DLLNETWORK default_converter<uint64_t const &> : default_converter<uint64_t> {};

	// int64_t
	template<>
	struct DLLNETWORK default_converter<int64_t> : native_converter_base<int64_t> {
		static int compute_score(lua::State *l, int index) { return Lua::GetType(l, index) == Lua::Type::Number ? 0 : -1; }

		int64_t to_cpp_deferred(lua::State *l, int index) { return static_cast<int64_t>(Lua::ToNumber(l, index)); }

		void to_lua_deferred(lua::State *l, int64_t value) { Lua::PushNumber(l, static_cast<double>(value)); }
	};

	template<>
	struct DLLNETWORK default_converter<int64_t const> : default_converter<int64_t> {};

	template<>
	struct DLLNETWORK default_converter<int64_t const &> : default_converter<int64_t> {};
}
