// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __GUI_ELEMENT_POLICY_HPP__
#define __GUI_ELEMENT_POLICY_HPP__

#if 0

namespace luabind {
	namespace detail {

		struct gui_element_converter
		{
			template <class T>
			void to_lua(lua_State* L, T const& x)
			{
				if constexpr(std::is_same_v<T,WIHandle>)
				{
					if(x.expired())
						lua_pushnil(L);
					else
					{
						auto &el = *x;
						auto o = WGUILuaInterface::GetLuaObject(L,const_cast<std::remove_cvref_t<decltype(el)>&>(el));
						o.push(L);
					}
				}
				else
				{
					auto o = WGUILuaInterface::GetLuaObject(L,const_cast<T&>(x));
					o.push(L);
				}
			}

			template <class T>
			void to_lua(lua_State* L, T* x)
			{
				if(!x)
					lua_pushnil(L);
				else
					to_lua(L,*x);
			}
		};

		struct gui_element_policy
		{
			template <class T, class Direction>
			struct specialize
			{
				static_assert(std::is_same<Direction, cpp_to_lua>::value, "Game object policy only supports cpp -> lua");
				using type = gui_element_converter;
			};
		};

	} // namespace detail

	template< unsigned int N >
	using gui_element_policy = meta::type_list< converter_policy_injector< N, detail::gui_element_policy > >;
} // namespace luabind
#endif

#endif
