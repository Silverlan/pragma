// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/lua/core.hpp"
#include <luabind/detail/policy.hpp>

export module pragma.shared:scripting.lua.policies.generic;

export namespace luabind {
	namespace detail {

		template<typename T, int (*TFuncMatch)(lua_State *, int), T (*TToCpp)(lua_State *, int), uint32_t TNumConsumed = 1>
		struct generic_converter {
			enum { consumed_args = TNumConsumed };

			template<class U>
			T to_cpp(lua_State *L, U u, int index)
			{
				return TToCpp(L, index);
			}

			template<class U>
			static int match(lua_State *l, U, int index)
			{
				return TFuncMatch(l, index);
			}

			template<class U>
			void converter_postcall(lua_State *, U u, int)
			{
			}
		};

		template<typename TType, int (*TFuncMatch)(lua_State *, int), TType (*TToCpp)(lua_State *, int), uint32_t TNumConsumed = 1>
		struct generic_policy {
			template<class T, class Direction>
			struct specialize {
				using type = generic_converter<TType, TFuncMatch, TToCpp, TNumConsumed>;
			};
		};

	} // namespace detail

	template<unsigned int N, typename T, int (*TFuncMatch)(lua_State *, int), T (*TToCpp)(lua_State *, int), uint32_t TNumConsumed = 1>
	using generic_policy = meta::type_list<converter_policy_injector<N, detail::generic_policy<T, TFuncMatch, TToCpp, TNumConsumed>>>;
} // namespace luabind
