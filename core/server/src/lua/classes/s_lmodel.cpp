/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "luasystem.h"
#include "pragma/lua/classes/s_lmodel.h"
#include <pragma/model/model.h>
#include "pragma/lua/classes/ldef_model.h"
#include <pragma/lua/classes/lmodel.h>

extern DLLSERVER ServerState *server;

void Lua::Model::Server::AddMaterial(lua_State *l,::Model &mdl,uint32_t textureGroup,const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = server->LoadMaterial(name);
	Lua::Model::AddMaterial(l,mdl,textureGroup,mat);
}
void Lua::Model::Server::SetMaterial(lua_State *l,::Model &mdl,uint32_t texIdx,const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = server->LoadMaterial(name);
	Lua::Model::SetMaterial(l,mdl,texIdx,mat);
}
