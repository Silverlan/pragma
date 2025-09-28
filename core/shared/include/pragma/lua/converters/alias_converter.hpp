// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_ALIAS_CONVERTER_HPP__
#define __LUA_ALIAS_CONVERTER_HPP__

#include <luabind/object.hpp>
#include "pragma/lua/core.hpp"

namespace luabind {
	template<typename T>
	using base_type = typename std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

	template<typename T>
	concept is_const_reference = std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>;

	template<class TBase, class... T>
	struct alias_converter {
		using TConverter = type_converter<TBase>;
		using UNDERLYING_TYPE = base_type<TBase>;

		using is_native = std::false_type;
		enum { consumed_args = 1 };

		template<size_t I = 0, typename... Tp>
		bool match_any(lua_State *L, int index);

		template<class U>
		TBase to_cpp(lua_State *L, U u, int index);
		void to_lua(lua_State *L, TBase x);

		template<class U>
		int match(lua_State *L, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
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

#endif
