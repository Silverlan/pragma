// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.converters.pair;

export import :scripting.lua.types.base_types;
export import pragma.lua;

export namespace luabind {
	template<typename T0, typename T1>
	struct default_converter<std::pair<T0, T1>> : native_converter_base<std::pair<T0, T1>> {
		enum { consumed_args = 2 };

		template<typename U>
		std::pair<T0, T1> to_cpp(lua::State *L, U u, int index);

		template<class U>
		int match(lua::State *l, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, std::pair<T0, T1> const &x);
		void to_lua(lua::State *L, std::pair<T0, T1> *x);
	  private:
		default_converter<T0> c0;
		default_converter<T1> c1;
	};

	template<typename T0, typename T1>
	struct default_converter<const std::pair<T0, T1>> : default_converter<std::pair<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<std::pair<T0, T1> const &> : default_converter<std::pair<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<std::pair<T0, T1> &&> : default_converter<std::pair<T0, T1>> {};
}

export namespace luabind {
	template<class... T>
	struct default_converter<std::tuple<T...>> : native_converter_base<std::tuple<T...>> {
		enum { consumed_args = sizeof...(T) };

		template<size_t I = 0, typename... Tp>
		int match_all(lua::State *L, int index, std::tuple<default_converter<Tp>...> &); // tuple parameter is unused but required for overload resolution for some reason

		template<size_t I = 0, typename... Tp>
		void to_lua_all(lua::State *L, const std::tuple<Tp...> &t);

		template<size_t I = 0, typename... Tp>
		void to_cpp_all(lua::State *L, int index, std::tuple<Tp...> &t);

		template<typename U>
		std::tuple<T...> to_cpp(lua::State *L, U u, int index);

		template<class U>
		int match(lua::State *l, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, std::tuple<T...> const &x);
		void to_lua(lua::State *L, std::tuple<T...> *x);
	  private:
		std::tuple<default_converter<T>...> m_converters;
	};

	template<class... T>
	struct default_converter<const std::tuple<T...>> : default_converter<std::tuple<T...>> {};

	template<class... T>
	struct default_converter<std::tuple<T...> const &> : default_converter<std::tuple<T...>> {};

	template<class... T>
	struct default_converter<std::tuple<T...> &&> : default_converter<std::tuple<T...>> {};
}

export namespace luabind {
	template<typename T0, typename T1>
	template<typename U>
	std::pair<T0, T1> default_converter<std::pair<T0, T1>>::to_cpp(lua::State *L, U u, int index)
	{
		std::pair<T0, T1> pair {};
		pair.first = c0.to_cpp(L, decorate_type_t<T0>(), index);
		pair.second = c1.to_cpp(L, decorate_type_t<T1>(), index + 1);
		return pair;
	}

	template<typename T0, typename T1>
	template<class U>
	int default_converter<std::pair<T0, T1>>::match(lua::State *l, U u, int index)
	{
		return (c0.match(l, decorate_type_t<T0>(), index) == 0 && c1.match(l, decorate_type_t<T1>(), index + 1) == 0) ? 0 : no_match;
	}

	template<typename T0, typename T1>
	void default_converter<std::pair<T0, T1>>::to_lua(lua::State *L, std::pair<T0, T1> const &x)
	{
		c0.to_lua(L, x.first);
		c1.to_lua(L, x.second);
	}

	template<typename T0, typename T1>
	void default_converter<std::pair<T0, T1>>::to_lua(lua::State *L, std::pair<T0, T1> *x)
	{
		if(!x)
			Lua::PushNil(L);
		else
			to_lua(L, *x);
	}

	template<class... T>
	template<size_t I, typename... Tp>
	int default_converter<std::tuple<T...>>::match_all(lua::State *L, int index, std::tuple<default_converter<Tp>...> &)
	{                                                                       // tuple parameter is unused but required for overload resolution for some reason
		using T2 = typename std::tuple_element<I, std::tuple<Tp...>>::type; //HEY!
		if(std::get<I>(m_converters).match(L, decorate_type_t<base_type<T2>>(), index) != 0)
			return no_match;
		if constexpr(I + 1 != sizeof...(Tp))
			return match_all<I + 1>(L, index + 1, m_converters);
		return 0;
	}

	template<class... T>
	template<size_t I, typename... Tp>
	void default_converter<std::tuple<T...>>::to_lua_all(lua::State *L, const std::tuple<Tp...> &t)
	{
		std::get<I>(m_converters).to_lua(L, std::get<I>(t));
		if constexpr(I + 1 != sizeof...(Tp))
			to_lua_all<I + 1>(L, t);
	}

	template<class... T>
	template<size_t I, typename... Tp>
	void default_converter<std::tuple<T...>>::to_cpp_all(lua::State *L, int index, std::tuple<Tp...> &t)
	{
		auto &v = std::get<I>(t);
		v = std::get<I>(m_converters).to_cpp(L, decorate_type_t<base_type<decltype(v)>>(), index++);
		if constexpr(I + 1 != sizeof...(Tp))
			to_cpp_all<I + 1>(L, index, t);
	}

	template<class... T>
	template<typename U>
	std::tuple<T...> default_converter<std::tuple<T...>>::to_cpp(lua::State *L, U u, int index)
	{
		std::tuple<T...> tuple {};
		to_cpp_all(L, index, tuple);
		return tuple;
	}

	template<class... T>
	template<class U>
	int default_converter<std::tuple<T...>>::match(lua::State *l, U u, int index)
	{
		return match_all<0, T...>(l, index, m_converters);
	}

	template<class... T>
	void default_converter<std::tuple<T...>>::to_lua(lua::State *L, std::tuple<T...> const &x)
	{
		to_lua_all(L, x);
	}

	template<class... T>
	void default_converter<std::tuple<T...>>::to_lua(lua::State *L, std::tuple<T...> *x)
	{
		if(!x)
			Lua::PushNil(L);
		else
			to_lua(L, *x);
	}
}
