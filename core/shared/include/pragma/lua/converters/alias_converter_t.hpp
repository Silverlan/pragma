// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_ALIAS_CONVERTER_T_HPP__
#define __LUA_ALIAS_CONVERTER_T_HPP__


template<class TBase, class... T>
template<size_t I, typename... Tp>
bool luabind::alias_converter<TBase, T...>::match_any(lua_State *L, int index)
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
int luabind::alias_converter<TBase, T...>::match(lua_State *L, U u, int index)
{
	auto res = m_converter.match(L, decorate_type_t<TBase>(), index);
	if(res != no_match)
		return 0;
	auto hasMatch = match_any<0, T...>(L, index);
	return hasMatch ? 1 : no_match;
}

template<class TBase, class... T>
template<class U>
TBase luabind::alias_converter<TBase, T...>::to_cpp(lua_State *L, U u, int index)
{
	if(m_tmp)
		return *m_tmp;
	return m_converter.to_cpp(L, decorate_type_t<TBase>(), index);
}

template<class TBase, class... T>
void luabind::alias_converter<TBase, T...>::to_lua(lua_State *L, TBase x)
{
	m_converter.to_lua(L, x);
}

#endif
