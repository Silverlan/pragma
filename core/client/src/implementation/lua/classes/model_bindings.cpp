// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_client.h"
#include <buffers/prosper_buffer.hpp>

module pragma.client;

import :model.model_class;
import :scripting.lua.classes.model;
import :assets;
import :client_state;

std::shared_ptr<Model> Lua::Model::Client::create_generic_model(Game &game, ::ModelSubMesh &subMesh)
{
	auto mesh = game.CreateModelMesh();
	mesh->AddSubMesh(subMesh.shared_from_this());

	auto mat = game.GetNetworkState()->LoadMaterial("white");
	if(!mat)
		return nullptr;
	auto mdl = game.CreateModel();
	if(!mdl)
		return nullptr;
	auto group = mdl->AddMeshGroup("reference");
	mdl->AddMaterial(0, mat);

	mdl->GetMeshGroup("reference")->AddMesh(mesh);
	mdl->Update(ModelUpdateFlags::All);
	return mdl;
}

void Lua::Model::Client::Export(lua_State *l, ::Model &mdl, const pragma::asset::ModelExportInfo &exportInfo)
{
	std::string errMsg;
	std::string outputPath;
	auto result = pragma::asset::export_model(mdl, exportInfo, errMsg, {}, &outputPath);
	Lua::PushBool(l, result);
	if(result == false)
		Lua::PushString(l, errMsg);
	else
		Lua::PushString(l, outputPath);
}
void Lua::Model::Client::ExportAnimation(lua_State *l, ::Model &mdl, const std::string &animName, const pragma::asset::ModelExportInfo &exportInfo)
{
	std::string errMsg;
	auto result = pragma::asset::export_animation(mdl, animName, exportInfo, errMsg);
	Lua::PushBool(l, result);
	if(result == false)
		Lua::PushString(l, errMsg);
}
void Lua::Model::Client::AddMaterial(lua_State *l, ::Model &mdl, uint32_t textureGroup, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = pragma::get_client_state()->LoadMaterial(name);
	Lua::Model::AddMaterial(l, mdl, textureGroup, mat);
}
void Lua::Model::Client::SetMaterial(lua_State *l, ::Model &mdl, uint32_t texIdx, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = pragma::get_client_state()->LoadMaterial(name);
	Lua::Model::SetMaterial(l, mdl, texIdx, mat);
}
void Lua::Model::Client::GetVertexAnimationBuffer(lua_State *l, ::Model &mdl)
{
	auto &buf = static_cast<CModel &>(mdl).GetVertexAnimationBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}