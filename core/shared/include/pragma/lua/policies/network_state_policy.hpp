/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __NETWORK_STATE_POLICY_HPP__
#define __NETWORK_STATE_POLICY_HPP__

// TODO: Remove this file
#if 0
#include <luabind/config.hpp>
#include <luabind/detail/policy.hpp>
#include "pragma/engine.h"
#include "pragma/networkstate/networkstate.h"

namespace luabind {
	namespace detail {

		struct network_state_converter
		{
			enum { consumed_args = 0 };

			NetworkState* to_cpp(lua_State* L, by_pointer<NetworkState>, int)
			{
				return pragma::get_engine()->GetNetworkState(L);
			}

			static int match(...)
			{
				return 0;
			}

			void converter_postcall(lua_State*, by_pointer<NetworkState>, int) {}
		};

		struct network_state_policy
		{
			template<class T, class Direction>
			struct specialize
			{
				using type = network_state_converter;
			};
		};

	}

	template<unsigned int N>
	using network_state_policy = meta::type_list< converter_policy_injector< N, detail::network_state_policy > >;

} // namespace luabind
#endif

#endif
