// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.converters.game_types;

pragma::Game *luabind::detail::get_game(lua::State *l) { return pragma::get_engine()->GetNetworkState(l)->GetGameState(); }
pragma::NetworkState *luabind::detail::get_network_state(lua::State *l) { return pragma::get_engine()->GetNetworkState(l); }
pragma::Engine *luabind::detail::get_engine(lua::State *l) { return pragma::get_engine(); }
pragma::physics::IEnvironment *luabind::detail::get_physics_environment(lua::State *l) { return pragma::get_engine()->GetNetworkState(l)->GetGameState()->GetPhysicsEnvironment(); }
