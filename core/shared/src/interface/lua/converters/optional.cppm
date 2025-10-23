// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/core.hpp"
#include <optional>

export module pragma.shared:scripting.lua.converters.optional;

export import luabind;

export namespace luabind {
	template<typename T>
	struct default_converter<std::optional<T>> : native_converter_base<std::optional<T>> {
		enum { consumed_args = 1 };

		template<typename U>
		std::optional<T> to_cpp(lua_State *L, U u, int index);

		template<class U>
		int match(lua_State *l, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}

		void to_lua(lua_State *L, std::optional<T> const &x);
		void to_lua(lua_State *L, std::optional<T> *x);
	  private:
		default_converter<T> m_converter;
	};

	template<typename T>
	struct default_converter<const std::optional<T>> : default_converter<std::optional<T>> {};

	template<typename T>
	struct default_converter<std::optional<T> const &> : default_converter<std::optional<T>> {};

	template<typename T>
	struct default_converter<std::optional<T> &&> : default_converter<std::optional<T>> {};
}

export {
	template<typename T>
	template<typename U>
	std::optional<T> luabind::default_converter<std::optional<T>>::to_cpp(lua_State *L, U u, int index)
	{
		if(lua_isnil(L, index))
			return {};
		return m_converter.to_cpp(L, decorate_type_t<T>(), index);
	}

	template<typename T>
	template<class U>
	int luabind::default_converter<std::optional<T>>::match(lua_State *l, U u, int index)
	{
		if(lua_isnil(l, index))
			return 1;
		return m_converter.match(l, decorate_type_t<T>(), index);
	}

	template<typename T>
	void luabind::default_converter<std::optional<T>>::to_lua(lua_State *L, std::optional<T> const &x)
	{
		if(!x.has_value())
			lua_pushnil(L);
		else
			m_converter.to_lua(L, *x);
	}

	template<typename T>
	void luabind::default_converter<std::optional<T>>::to_lua(lua_State *L, std::optional<T> *x)
	{
		if(!x || !x->has_value())
			lua_pushnil(L);
		else
			to_lua(L, *x);
	}
}
