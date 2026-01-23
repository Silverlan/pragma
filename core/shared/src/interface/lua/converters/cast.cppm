// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.converters.cast;

export import pragma.lua;

export namespace luabind {
	template<typename TCpp, typename TLua>
	struct cast_converter : detail::default_converter_generator<TLua> {
		enum { consumed_args = 1 };

		template<class U>
		TCpp to_cpp(lua::State *L, U, int index);

		template<class U>
		int match(lua::State *, U, int index);

		template<class U>
		void converter_postcall(lua::State *, U, int)
		{
		}

		void to_lua(lua::State *L, TCpp v);
	};
};

export namespace luabind {
	template<typename TCpp, typename TLua>
	template<class U>
	TCpp cast_converter<TCpp, TLua>::to_cpp(lua::State *L, U u, int index)
	{
		return static_cast<TCpp>(detail::default_converter_generator<TLua>::to_cpp(L, u, index));
	}

	template<typename TCpp, typename TLua>
	template<class U>
	int cast_converter<TCpp, TLua>::match(lua::State *l, U u, int index)
	{
		return detail::default_converter_generator<TLua>::match(l, luabind::decorate_type_t<TLua> {}, index);
	}

	template<typename TCpp, typename TLua>
	void cast_converter<TCpp, TLua>::to_lua(lua::State *L, TCpp v)
	{
		detail::default_converter_generator<TLua>::to_lua(L, static_cast<TLua>(v));
	}
}
