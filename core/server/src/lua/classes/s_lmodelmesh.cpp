// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "luasystem.h"
#include "pragma/lua/classes/s_lmodelmesh.h"
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/lua/classes/lmodelmesh.h>

void Lua::ModelMesh::Server::Create(lua_State *l) { Lua::Push<std::shared_ptr<::ModelMesh>>(l, std::make_shared<::ModelMesh>()); }

void Lua::ModelSubMesh::Server::Create(lua_State *l) { Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, std::make_shared<::ModelSubMesh>()); }
