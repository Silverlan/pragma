// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "luasystem.h"

module pragma.server.scripting.lua.libraries.engine;

import pragma.server.server_state;

extern ServerState *server;

Material *Lua::engine::server::LoadMaterial(const std::string &mat, bool reload) { return ::server->LoadMaterial(mat, reload); }
Material *Lua::engine::server::LoadMaterial(const std::string &mat) { return LoadMaterial(mat, false); }
