/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lmodel.h"
#include "luasystem.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/asset/c_util_model.hpp"
#include <pragma/lua/classes/lmodel.h>
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <pragma/lua/libraries/lfile.h>
#include <buffers/prosper_buffer.hpp>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static std::shared_ptr<Model> create_generic_model(Game &game)
{
	auto *l = game.GetLuaState();
	auto subMesh = luabind::object_cast<std::shared_ptr<::ModelSubMesh>>(luabind::object {luabind::from_stack(l, -1)});
	Lua::Pop(l, 1);

	auto mesh = game.CreateModelMesh();
	mesh->AddSubMesh(subMesh);

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

#include "pragma/lua/classes/c_lmodelmesh.h"
std::shared_ptr<::Model> Lua::Model::Client::create_quad(lua_State *l, Game &game, float size)
{
	Lua::ModelSubMesh::Client::CreateQuad(l, size);
	return create_generic_model(game);
}
std::shared_ptr<::Model> Lua::Model::Client::create_box(lua_State *l, Game &game, const Vector3 &min, const Vector3 &max)
{
	Lua::ModelSubMesh::Client::CreateBox(l, min, max);
	return create_generic_model(game);
}
std::shared_ptr<::Model> Lua::Model::Client::create_sphere(lua_State *l, Game &game, const Vector3 &origin, float radius, uint32_t recursionLevel)
{
	Lua::ModelSubMesh::Client::CreateSphere(l, origin, radius, recursionLevel);
	return create_generic_model(game);
}
std::shared_ptr<::Model> Lua::Model::Client::create_sphere(lua_State *l, Game &game, const Vector3 &origin, float radius)
{
	Lua::ModelSubMesh::Client::CreateSphere(l, origin, radius);
	return create_generic_model(game);
}
std::shared_ptr<::Model> Lua::Model::Client::create_cylinder(lua_State *l, Game &game, float startRadius, float length, uint32_t segmentCount)
{
	Lua::ModelSubMesh::Client::CreateCylinder(l, startRadius, length, segmentCount);
	return create_generic_model(game);
}
std::shared_ptr<::Model> Lua::Model::Client::create_cone(lua_State *l, Game &game, float startRadius, float length, float endRadius, uint32_t segmentCount)
{
	Lua::ModelSubMesh::Client::CreateCone(l, startRadius, length, endRadius, segmentCount);
	return create_generic_model(game);
}
std::shared_ptr<::Model> Lua::Model::Client::create_circle(lua_State *l, Game &game, float radius, bool doubleSided, uint32_t segmentCount)
{
	Lua::ModelSubMesh::Client::CreateCircle(l, radius, doubleSided, segmentCount);
	return create_generic_model(game);
}
std::shared_ptr<::Model> Lua::Model::Client::create_ring(lua_State *l, Game &game, float innerRadius, float outerRadius, bool doubleSided, uint32_t segmentCount)
{
	Lua::ModelSubMesh::Client::CreateRing(l, innerRadius, outerRadius, doubleSided, segmentCount);
	return create_generic_model(game);
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
	auto *mat = client->LoadMaterial(name);
	Lua::Model::AddMaterial(l, mdl, textureGroup, mat);
}
void Lua::Model::Client::SetMaterial(lua_State *l, ::Model &mdl, uint32_t texIdx, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = client->LoadMaterial(name);
	Lua::Model::SetMaterial(l, mdl, texIdx, mat);
}
void Lua::Model::Client::GetVertexAnimationBuffer(lua_State *l, ::Model &mdl)
{
	auto &buf = static_cast<CModel &>(mdl).GetVertexAnimationBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}
