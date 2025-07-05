// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/lua/libraries/s_lengine.h"
#include "luasystem.h"
#include <pragma/serverstate/serverstate.h>

extern ServerState *server;

Material *Lua::engine::server::LoadMaterial(const std::string &mat, bool reload) { return ::server->LoadMaterial(mat, reload); }
Material *Lua::engine::server::LoadMaterial(const std::string &mat) { return LoadMaterial(mat, false); }
