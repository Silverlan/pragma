// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_LUA_TYPES_NIL_TYPE_HPP__
#define __PRAGMA_LUA_TYPES_NIL_TYPE_HPP__


#include <luasystem.h>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/pseudo_traits.hpp>
#include <luabind/make_function_signature.hpp>

namespace luabind {
	// Additional types for overload resolution
	namespace adl {
		template<typename T>
		struct nil_type : object {
			nil_type(from_stack const &stack_reference) : object(stack_reference) {}
			nil_type(const object &o) : object(o) {}
			nil_type(lua_State *l, const T &t) : object(l, t) {}
			using value_type = T;
		};
	} // namespace adl

	namespace detail {
		template<typename T>
		struct pseudo_traits<adl::nil_type<T>> {
			enum { is_variadic = false };
			using value_type = T;
		};
	};

	using adl::nil_type;

	template<typename T>
	struct lua_proxy_traits<adl::nil_type<T>> : lua_proxy_traits<object> {
		static bool check(lua_State *L, int idx) { return Lua::IsNil(L,idx); }
	};
};

namespace Lua {
	using nil_type = luabind::nil_type<luabind::object>;
};

#endif
