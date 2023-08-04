/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lmodelmesh.h"
#include "luasystem.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/model/vk_mesh.h"
#include <pragma/lua/classes/lmodelmesh.h>
#include <buffers/prosper_buffer.hpp>

void Lua::ModelMesh::Client::Create(lua_State *l) { Lua::Push<std::shared_ptr<::ModelMesh>>(l, std::make_shared<::CModelMesh>()); }
void Lua::ModelSubMesh::Client::Create(lua_State *l) { Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, std::make_shared<::CModelSubMesh>()); }
void Lua::ModelSubMesh::Client::GetVkMesh(lua_State *l, ::ModelSubMesh &mesh) { Lua::Push<std::shared_ptr<::pragma::SceneMesh>>(l, static_cast<CModelSubMesh &>(mesh).GetSceneMesh()); }
void Lua::ModelSubMesh::Client::GetTangents(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();

	auto tTangents = Lua::CreateTable(l);
	for(auto i = decltype(verts.size()) {0}; i < verts.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::Push<Vector3>(l, verts[i].tangent);
		Lua::SetTableValue(l, tTangents);
	}
}
void Lua::ModelSubMesh::Client::GetBiTangents(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();

	auto tBiTangents = Lua::CreateTable(l);
	for(auto i = decltype(verts.size()) {0}; i < verts.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::Push<Vector3>(l, verts[i].GetBiTangent());
		Lua::SetTableValue(l, tBiTangents);
	}
}
void Lua::ModelSubMesh::Client::GetVertexBuffer(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &vkMesh = static_cast<CModelSubMesh &>(mesh).GetSceneMesh();
	auto &buf = vkMesh->GetVertexBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}
void Lua::ModelSubMesh::Client::GetVertexWeightBuffer(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &vkMesh = static_cast<CModelSubMesh &>(mesh).GetSceneMesh();
	auto &buf = vkMesh->GetVertexWeightBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}
void Lua::ModelSubMesh::Client::GetAlphaBuffer(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &vkMesh = static_cast<CModelSubMesh &>(mesh).GetSceneMesh();
	auto &buf = vkMesh->GetAlphaBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}
void Lua::ModelSubMesh::Client::GetIndexBuffer(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &vkMesh = static_cast<CModelSubMesh &>(mesh).GetSceneMesh();
	auto &buf = vkMesh->GetIndexBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}
