// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"
#include "luasystem.h"

module pragma.server.scripting.lua.libraries.engine;

import pragma.server.server_state;

Material *Lua::engine::server::LoadMaterial(const std::string &mat, bool reload) { return ServerState::Get()->LoadMaterial(mat, reload); }
Material *Lua::engine::server::LoadMaterial(const std::string &mat) { return LoadMaterial(mat, false); }
