/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GAME_POLICY_HPP__
#define __GAME_POLICY_HPP__

#include <luabind/config.hpp>
#include <luabind/detail/policy.hpp>
#include "pragma/engine.h"
#include "pragma/networkstate/networkstate.h"

namespace luabind {
	namespace detail {

		struct game_converter
		{
			enum { consumed_args = 0 };

			Game* to_cpp(lua_State* L, by_pointer<Game>, int)
			{
				return pragma::get_engine()->GetNetworkState(L)->GetGameState();
			}

			static int match(...)
			{
				return 0;
			}

			void converter_postcall(lua_State*, by_pointer<Game>, int) {}
		};

		struct game_policy
		{
			template<class T, class Direction>
			struct specialize
			{
				using type = game_converter;
			};
		};

	}

	template<unsigned int N>
	using game_policy = meta::type_list< converter_policy_injector< N, detail::game_policy > >();

} // namespace luabind

#endif

