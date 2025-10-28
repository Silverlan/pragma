// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_server.h"

module pragma.server.scripting.lua.classes.model;
import pragma.server.server_state;

void Lua::Model::Server::AddMaterial(lua_State *l, pragma::Model &mdl, uint32_t textureGroup, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = ServerState::Get()->LoadMaterial(name);
	Lua::Model::AddMaterial(l, mdl, textureGroup, mat);
}
void Lua::Model::Server::SetMaterial(lua_State *l, pragma::Model &mdl, uint32_t texIdx, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = ServerState::Get()->LoadMaterial(name);
	Lua::Model::SetMaterial(l, mdl, texIdx, mat);
}
