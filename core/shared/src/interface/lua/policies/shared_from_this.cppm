// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.policies.shared_from_this;

export import pragma.lua;

export namespace luabind {
	namespace detail {

		struct shared_from_this_converter {
			template<class T>
			void to_lua(lua::State *L, T const &x)
			{
				default_converter<T>().to_lua(L, const_cast<T &>(x).shared_from_this());
			}

			template<class T>
			void to_lua(lua::State *L, T *x)
			{
				if(!x)
					Lua::PushNil(L);
				else
					to_lua(L, *x);
			}
		};

		struct shared_from_this_policy {
			template<class T, class Direction>
			struct specialize {
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "shared_from_this policy only supports cpp -> lua");
				using type = shared_from_this_converter;
			};
		};

	} // namespace detail

	template<unsigned int N>
	using shared_from_this_policy = meta::type_list<converter_policy_injector<N, detail::shared_from_this_policy>>;

} // namespace luabind
