// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.converters.alias;

export import :scripting.lua.types.base_types;
export import pragma.lua;

export namespace luabind {
	template<typename T>
	concept is_const_reference = std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>;

	template<class TBase, class... T>
	struct alias_converter {
		using TConverter = type_converter<TBase>;
		using UNDERLYING_TYPE = base_type<TBase>;

		using is_native = std::false_type;
		enum { consumed_args = 1 };

		template<size_t I = 0, typename... Tp>
		bool match_any(lua::State *L, int index);

		template<class U>
		TBase to_cpp(lua::State *L, U u, int index);
		void to_lua(lua::State *L, TBase x);

		template<class U>
		int match(lua::State *L, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}
	  private:
		TConverter m_converter;
		std::unique_ptr<UNDERLYING_TYPE> m_tmp = nullptr;
	};

	namespace detail {
		template<typename T0, typename T1>
		struct AliasTypeConverter {
			static void convert(const T1 &srcValue, T0 &outValue) { outValue = T1 {srcValue}; }
		};
	};

	template<typename TTest, typename T>
	concept is_valid_alias_candidate = std::is_same_v<T, TTest> || std::is_same_v<TTest, std::add_lvalue_reference_t<std::add_const_t<T>>>;

	template<typename T, typename... Ts>
	concept is_one_of_alias_candidates = std::disjunction_v<std::is_same<T, Ts>...> || std::disjunction_v<std::is_same<T, std::add_lvalue_reference_t<std::add_const_t<Ts>>>...>; // Is value or const reference?
};

export namespace luabind {
	template<class TBase, class... T>
	template<size_t I, typename... Tp>
	bool alias_converter<TBase, T...>::match_any(lua::State *L, int index)
	{
		using T2 = typename std::tuple_element<I, std::tuple<Tp...>>::type;
		if constexpr(!std::is_same_v<T2, TBase>) // Base type has already been covered by 'match'
		{
			if constexpr(is_native_type<T2>) {
				default_converter<T2> converter;
				if(converter.match(L, decorate_type_t<T2>(), index) != no_match) {
					m_tmp = std::make_unique<UNDERLYING_TYPE>();
					detail::AliasTypeConverter<UNDERLYING_TYPE, T2>::convert(converter.to_cpp(L, decorate_type_t<T2>(), index), *m_tmp);
					return true;
				}
			}
			else {
				if(m_converter.match(L, decorate_type_t<copy_qualifiers_t<TBase, T2>>(), index) != no_match) {
					m_tmp = std::make_unique<UNDERLYING_TYPE>();
					detail::AliasTypeConverter<UNDERLYING_TYPE, T2>::convert(m_converter.to_cpp(L, decorate_type_t<copy_qualifiers_t<TBase, T2>>(), index), *m_tmp);
					return true;
				}
			}
		}
		if constexpr(I + 1 != sizeof...(Tp))
			return match_any<I + 1, Tp...>(L, index);
		return false;
	}

	template<class TBase, class... T>
	template<class U>
	int alias_converter<TBase, T...>::match(lua::State *L, U u, int index)
	{
		auto res = m_converter.match(L, decorate_type_t<TBase>(), index);
		if(res != no_match)
			return 0;
		auto hasMatch = match_any<0, T...>(L, index);
		return hasMatch ? 1 : no_match;
	}

	template<class TBase, class... T>
	template<class U>
	TBase alias_converter<TBase, T...>::to_cpp(lua::State *L, U u, int index)
	{
		if(m_tmp)
			return *m_tmp;
		return m_converter.to_cpp(L, decorate_type_t<TBase>(), index);
	}

	template<class TBase, class... T>
	void alias_converter<TBase, T...>::to_lua(lua::State *L, TBase x)
	{
		m_converter.to_lua(L, x);
	}
}
