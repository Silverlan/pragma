// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.model;
import :server_state;

void Lua::Model::Server::AddMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t textureGroup, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = pragma::ServerState::Get()->LoadMaterial(name);
	Model::AddMaterial(l, mdl, textureGroup, mat);
}
void Lua::Model::Server::SetMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t texIdx, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = pragma::ServerState::Get()->LoadMaterial(name);
	Model::SetMaterial(l, mdl, texIdx, mat);
}
