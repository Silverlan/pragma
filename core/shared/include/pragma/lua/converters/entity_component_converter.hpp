/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_ENTITY_COMPONENT_CONVERTER_HPP__
#define __LUA_ENTITY_COMPONENT_CONVERTER_HPP__

// TODO: Remove this file

#include "pragma/lua/converters/game_type_converters.hpp"

namespace luabind {
	/*template<typename T> requires(is_type_or_derived<base_type<T>,pragma::BaseEntityComponent>)
	struct default_converter<T>
		: type_converter<T>
	{
		using TEntity = copy_qualifiers_t<T,BaseEntity>;
		using is_native = std::false_type;

		template <class U>
		int match(lua_State* L, U u, int index)
		{
			if constexpr(is_decorated_type_target<U,BaseEntity>)
			{
				auto res = type_converter<T>::match(L,luabind::decorate_type_t<TEntity>{},index);
				if(res == no_match)
					return no_match;
				TEntity c = type_converter<T>::to_cpp(L,luabind::decorate_type_t<TEntity>{},index);
				if constexpr(std::is_pointer_v<TEntity>)
				{
					if(!c)
						return res;
					m_handle = std::make_unique<pragma::ComponentHandle<base_type<T>>>(c->GetComponent<base_type<T>>());
				}
				else
					m_handle = std::make_unique<pragma::ComponentHandle<base_type<T>>>(c.GetComponent<base_type<T>>());
				return m_handle->valid() ? 0 : no_match;
			}
			else
				return type_converter<T>::match(L,u,index);
		}

		template <class U>
		T to_cpp(lua_State* L, U u, int index)
		{
			if constexpr(is_decorated_type_target<U,BaseEntity>)
			{
				if constexpr(std::is_pointer_v<T>)
					return m_handle && m_handle->valid() ? m_handle->get() : nullptr;
				else
				{
					assert(m_handle && m_handle->valid());
					return **m_handle;
				}
			}
			return type_converter<T>::to_cpp(L,u,index);
		}

		void to_lua(lua_State* L, T const& p)
		{
			if constexpr(std::is_pointer_v<T>)
			{
				if(!p)
					lua_pushnil(L);
				else
					const_cast<T>(p)->GetLuaObject().push(L);
			}
			else
				const_cast<T>(p).GetLuaObject().push(L);
		}

		template <class U>
		void converter_postcall(lua_State*, U const&, int) {}
	private:
		std::unique_ptr<util::TWeakSharedHandle<base_type<T>>> m_handle = nullptr;
	};*/
};

#endif
