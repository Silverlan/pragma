/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_SHADER_CONVERTERS_HPP__
#define __LUA_SHADER_CONVERTERS_HPP__

#include "pragma/networkdefinitions.h"
#include <luabind/detail/conversion_policies/native_converter.hpp>

namespace prosper {
	class Shader;
};
namespace luabind {
	template<typename T>
	using base_type = typename std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

	template<typename T, typename TConverter>
	struct shader_converter {
		enum { consumed_args = 1 };

		template<class U>
		T to_cpp(lua_State *L, U u, int index);
		void to_lua(lua_State *L, T x);

		template<class U>
		int match(lua_State *L, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}
	  private:
		TConverter m_converter;
	};

	template<typename T>
	concept is_shader = std::derived_from<base_type<T>, ::prosper::Shader>;

	template<typename T>
	    requires(is_shader<T> && std::is_pointer_v<T> && !std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : shader_converter<T, luabind::detail::pointer_converter> {};
	template<typename T>
	    requires(is_shader<T> && std::is_pointer_v<T> && std::is_const_v<std::remove_pointer_t<T>>)
	struct default_converter<T> : shader_converter<T, luabind::detail::const_pointer_converter> {};

	template<typename T>
	    requires(is_shader<T> && std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : shader_converter<T, luabind::detail::ref_converter> {};
	template<typename T>
	    requires(is_shader<T> && std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>)
	struct default_converter<T> : shader_converter<T, luabind::detail::const_ref_converter> {};
};

#endif
