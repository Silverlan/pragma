// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :model.model_class;
import :scripting.lua.classes.model;
import :assets;
import :client_state;

std::shared_ptr<pragma::asset::Model> Lua::Model::Client::create_generic_model(pragma::Game &game, pragma::geometry::ModelSubMesh &subMesh)
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
	mdl->Update(pragma::asset::ModelUpdateFlags::All);
	return mdl;
}

void Lua::Model::Client::Export(lua::State *l, pragma::asset::Model &mdl, const pragma::asset::ModelExportInfo &exportInfo)
{
	std::string errMsg;
	std::string outputPath;
	auto result = pragma::asset::export_model(mdl, exportInfo, errMsg, {}, &outputPath);
	PushBool(l, result);
	if(result == false)
		PushString(l, errMsg);
	else
		PushString(l, outputPath);
}
void Lua::Model::Client::ExportAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &animName, const pragma::asset::ModelExportInfo &exportInfo)
{
	std::string errMsg;
	auto result = pragma::asset::export_animation(mdl, animName, exportInfo, errMsg);
	PushBool(l, result);
	if(result == false)
		PushString(l, errMsg);
}
void Lua::Model::Client::AddMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t textureGroup, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = pragma::get_client_state()->LoadMaterial(name);
	Model::AddMaterial(l, mdl, textureGroup, mat);
}
void Lua::Model::Client::SetMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t texIdx, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = pragma::get_client_state()->LoadMaterial(name);
	Model::SetMaterial(l, mdl, texIdx, mat);
}
void Lua::Model::Client::GetVertexAnimationBuffer(lua::State *l, pragma::asset::Model &mdl)
{
	auto &buf = static_cast<pragma::asset::CModel &>(mdl).GetVertexAnimationBuffer();
	if(buf == nullptr)
		return;
	Push(l, buf);
}