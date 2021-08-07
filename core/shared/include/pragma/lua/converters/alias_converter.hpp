/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_ALIAS_CONVERTER_HPP__
#define __LUA_ALIAS_CONVERTER_HPP__

#include "pragma/networkdefinitions.h"
#include <luabind/object.hpp>

namespace luabind
{
	template <typename T>
	using base_type = typename std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

	template<typename T>
		concept is_const_reference = std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>;

	template<typename T>
		using type_converter = std::conditional_t<is_const_reference<T>,luabind::detail::const_ref_converter,luabind::detail::value_converter>;

	template <class TBase,class ...T>
	struct alias_converter
	{
		using TConverter = type_converter<TBase>;
		using UNDERLYING_TYPE = base_type<TBase>;

		using is_native = std::false_type;
		enum { consumed_args = 1 };

		template<size_t I = 0, typename... Tp>
			bool match_any(lua_State *L,int index);

		template <class U>
		TBase to_cpp(lua_State* L, U u, int index);
		void to_lua(lua_State* L, TBase x);

		template <class U>
		int match(lua_State* L, U u, int index);

		template <class U>
		void converter_postcall(lua_State*, U u, int) {}
	private:
		TConverter m_converter;
		std::unique_ptr<UNDERLYING_TYPE> m_tmp = nullptr;
	};

	namespace detail
	{
		template<typename T0,typename T1>
			struct AliasTypeConverter
		{
			static void convert(const T1 &srcValue,T0 &outValue) {outValue = T1{srcValue};}
		};
	};

	template<typename TTest,typename T>
		concept is_valid_alias_candidate = std::is_same_v<T,TTest> || std::is_same_v<TTest,std::add_lvalue_reference_t<std::add_const_t<T>>>;

	template<typename T,typename ...Ts>
		concept is_one_of_alias_candidates = std::disjunction_v<std::is_same<T,Ts>...> || std::disjunction_v<std::is_same<T,std::add_lvalue_reference_t<std::add_const_t<Ts>>>...>; // Is value or const reference?

	template<typename T,typename U>
	struct copy_qualifiers
	{
	private:
		using R = std::remove_reference_t<T>;
		using U1 = std::conditional_t<std::is_const<R>::value, std::add_const_t<U>, U>;
		using U2 = std::conditional_t<std::is_volatile<R>::value, std::add_volatile_t<U1>, U1>;
		using U3 = std::conditional_t<std::is_lvalue_reference<T>::value, std::add_lvalue_reference_t<U2>, U2>;
		using U4 = std::conditional_t<std::is_rvalue_reference<T>::value, std::add_rvalue_reference_t<U3>, U3>;
		using U5 = std::conditional_t<std::is_pointer<T>::value, std::add_pointer_t<U4>, U4>;
	public:
		using type = U5;
	};

	template<typename T,typename U>
		using copy_qualifiers_t = typename copy_qualifiers<T,U>::type;
};

#endif
