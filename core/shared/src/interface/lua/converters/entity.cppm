// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.converters.entity;

export import :scripting.lua.types.base_types;
export import :types;
export import pragma.lua;

export namespace luabind {
	template<typename T>
	    requires(is_type_or_derived<base_type<T>, pragma::ecs::BaseEntity>)
	struct default_converter<T> : type_converter<T> {
		using TComponent = copy_qualifiers_t<T, pragma::BaseEntityComponent>;
		using is_native = std::false_type;

		template<class U>
		int match(lua::State *L, U u, int index)
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
		T to_cpp(lua::State *L, U u, int index)
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

		void to_lua(lua::State *L, T const &p)
		{
			if constexpr(std::is_pointer_v<T>) {
				if(!p)
					Lua::PushNil(L);
				else
					const_cast<T>(p)->GetLuaObject().push(L);
			}
			else
				const_cast<T>(p).GetLuaObject().push(L);
		}

		template<class U>
		void converter_postcall(lua::State *, U const &, int)
		{
		}
	  private:
		bool m_isComponent = false;
	};
};
