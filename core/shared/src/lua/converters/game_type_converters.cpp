

// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/lua/converters/game_type_converters.hpp"

import pragma.shared;

pragma::Game *luabind::detail::get_game(lua_State *l) { return pragma::get_engine()->GetNetworkState(l)->GetGameState(); }
NetworkState *luabind::detail::get_network_state(lua_State *l) { return pragma::get_engine()->GetNetworkState(l); }
pragma::Engine *luabind::detail::get_engine(lua_State *l) { return pragma::get_engine(); }
pragma::physics::IEnvironment *luabind::detail::get_physics_environment(lua_State *l) { return pragma::get_engine()->GetNetworkState(l)->GetGameState()->GetPhysicsEnvironment(); }
