/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/lua/libraries/s_lengine.h"
#include "luasystem.h"
#include <pragma/serverstate/serverstate.h>

extern ServerState *server;

Material *Lua::engine::server::LoadMaterial(const std::string &mat,bool reload) {return ::server->LoadMaterial(mat,reload);}
Material *Lua::engine::server::LoadMaterial(const std::string &mat) {return LoadMaterial(mat,false);}
