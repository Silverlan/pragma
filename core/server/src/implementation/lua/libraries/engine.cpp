// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_server.h"

module pragma.server.scripting.lua.libraries.engine;

import pragma.server.server_state;

msys::Material *Lua::engine::server::LoadMaterial(const std::string &mat, bool reload) { return ServerState::Get()->LoadMaterial(mat, reload); }
msys::Material *Lua::engine::server::LoadMaterial(const std::string &mat) { return LoadMaterial(mat, false); }
