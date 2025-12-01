// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.policies.generic;

export import pragma.lua;

export namespace luabind {
	namespace detail {

		template<typename T, int (*TFuncMatch)(lua::State *, int), T (*TToCpp)(lua::State *, int), uint32_t TNumConsumed = 1>
		struct generic_converter {
			enum { consumed_args = TNumConsumed };

			template<class U>
			T to_cpp(lua::State *L, U u, int index)
			{
				return TToCpp(L, index);
			}

			template<class U>
			static int match(lua::State *l, U, int index)
			{
				return TFuncMatch(l, index);
			}

			template<class U>
			void converter_postcall(lua::State *, U u, int)
			{
			}
		};

		template<typename TType, int (*TFuncMatch)(lua::State *, int), TType (*TToCpp)(lua::State *, int), uint32_t TNumConsumed = 1>
		struct generic_policy {
			template<class T, class Direction>
			struct specialize {
				using type = generic_converter<TType, TFuncMatch, TToCpp, TNumConsumed>;
			};
		};

	} // namespace detail

	template<unsigned int N, typename T, int (*TFuncMatch)(lua::State *, int), T (*TToCpp)(lua::State *, int), uint32_t TNumConsumed = 1>
	using generic_policy = meta::type_list<converter_policy_injector<N, detail::generic_policy<T, TFuncMatch, TToCpp, TNumConsumed>>>;
} // namespace luabind
