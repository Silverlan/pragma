// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.libraries.engine;

import :server_state;

pragma::material::Material *Lua::engine::server::LoadMaterial(const std::string &mat, bool reload) { return pragma::ServerState::Get()->LoadMaterial(mat, reload); }
pragma::material::Material *Lua::engine::server::LoadMaterial(const std::string &mat) { return LoadMaterial(mat, false); }
