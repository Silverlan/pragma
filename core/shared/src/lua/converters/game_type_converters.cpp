/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/converters/game_type_converters.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"

Game *luabind::detail::get_game(lua_State *l) { return pragma::get_engine()->GetNetworkState(l)->GetGameState(); }
NetworkState *luabind::detail::get_network_state(lua_State *l) { return pragma::get_engine()->GetNetworkState(l); }
Engine *luabind::detail::get_engine(lua_State *l) { return pragma::get_engine(); }
pragma::physics::IEnvironment *luabind::detail::get_physics_environment(lua_State *l) { return pragma::get_engine()->GetNetworkState(l)->GetGameState()->GetPhysicsEnvironment(); }
