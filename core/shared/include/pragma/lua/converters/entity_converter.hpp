// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_ENTITY_CONVERTER_HPP__
#define __LUA_ENTITY_CONVERTER_HPP__

#include "pragma/lua/converters/game_type_converters.hpp"

namespace luabind {
	template<typename T>
	    requires(is_type_or_derived<base_type<T>, BaseEntity>)
	struct default_converter<T> : type_converter<T> {
		using TComponent = copy_qualifiers_t<T, pragma::BaseEntityComponent>;
		using is_native = std::false_type;

		template<class U>
		int match(lua_State *L, U u, int index)
		{
			auto res = type_converter<T>::match(L, u, index);
			if(res != no_match) {
				m_isComponent = false;
				return res;
			}
			m_isComponent = true;
			return type_converter<T>::match(L, luabind::decorate_type_t<TComponent> {}, index);
		}

		template<class U>
		T to_cpp(lua_State *L, U u, int index)
		{
			if(m_isComponent) {
				TComponent c = type_converter<T>::to_cpp(L, luabind::decorate_type_t<TComponent> {}, index);
				if constexpr(std::is_pointer_v<TComponent>) {
					if(!c)
						return nullptr;
					return static_cast<T>(&c->GetEntity());
				}
				else
					return static_cast<T>(c.GetEntity());
			}
			else
				return type_converter<T>::to_cpp(L, u, index);
		}

		void to_lua(lua_State *L, T const &p)
		{
			if constexpr(std::is_pointer_v<T>) {
				if(!p)
					lua_pushnil(L);
				else
					const_cast<T>(p)->GetLuaObject().push(L);
			}
			else
				const_cast<T>(p).GetLuaObject().push(L);
		}

		template<class U>
		void converter_postcall(lua_State *, U const &, int)
		{
		}
	  private:
		bool m_isComponent = false;
	};
};

#endif
