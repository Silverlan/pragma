/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "luasystem.h"
#include "pragma/lua/classes/s_lmodelmesh.h"
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/lua/classes/lmodelmesh.h>

void Lua::ModelMesh::Server::Create(lua_State *l) { Lua::Push<std::shared_ptr<::ModelMesh>>(l, std::make_shared<::ModelMesh>()); }

void Lua::ModelSubMesh::Server::Create(lua_State *l) { Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, std::make_shared<::ModelSubMesh>()); }
