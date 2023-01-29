/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __DEFAULT_PARAMETER_POLICY_HPP__
#define __DEFAULT_PARAMETER_POLICY_HPP__

#include <luabind/config.hpp>
#include <luabind/detail/policy.hpp>

namespace luabind {
	namespace detail {

		template<auto DEF = luabind::nil>
		struct default_parameter {
			enum { consumed_args = 0 };

			using TYPE = decltype(DEF);
			template<typename U>
			TYPE to_cpp(lua_State *L, U, int)
			{
				return DEF;
			}

			static int match(...) { return 0; }

			template<class U>
			void converter_postcall(lua_State *, U u, int)
			{
			}
		};

		template<auto DEF = luabind::nil>
		struct default_parameter_policy {
			template<class T, class Direction>
			struct specialize {
				using type = default_parameter<DEF>;
			};
		};

	}

	template<unsigned int N, auto DEF = luabind::nil>
	using default_parameter_policy = meta::type_list<converter_policy_injector<N, detail::default_parameter_policy<DEF>>>;

} // namespace luabind

#endif
