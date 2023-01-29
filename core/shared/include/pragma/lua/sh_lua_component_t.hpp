/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SH_LUA_COMPONENT_T_HPP__
#define __SH_LUA_COMPONENT_T_HPP__

#include "pragma/lua/lentity_component_lua.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include "pragma/lua/sh_lua_component_wrapper.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"

namespace Lua {
	DLLNETWORK void register_base_entity_component(luabind::module_ &modEnts);
};

#endif
