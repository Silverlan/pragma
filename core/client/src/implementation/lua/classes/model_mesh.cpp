// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :scripting.lua.classes.model_mesh;
import :model;

void Lua::ModelMesh::Client::Create(lua::State *l) { Lua::Push<std::shared_ptr<::ModelMesh>>(l, std::make_shared<::CModelMesh>()); }
void Lua::ModelSubMesh::Client::Create(lua::State *l) { Lua::Push<std::shared_ptr<pragma::ModelSubMesh>>(l, std::make_shared<::CModelSubMesh>()); }
void Lua::ModelSubMesh::Client::GetVkMesh(lua::State *l, pragma::ModelSubMesh &mesh) { Lua::Push<std::shared_ptr<::pragma::SceneMesh>>(l, static_cast<CModelSubMesh &>(mesh).GetSceneMesh()); }
void Lua::ModelSubMesh::Client::GetTangents(lua::State *l, pragma::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();

	auto tTangents = Lua::CreateTable(l);
	for(auto i = decltype(verts.size()) {0}; i < verts.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::Push<Vector3>(l, verts[i].tangent);
		Lua::SetTableValue(l, tTangents);
	}
}
void Lua::ModelSubMesh::Client::GetBiTangents(lua::State *l, pragma::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();

	auto tBiTangents = Lua::CreateTable(l);
	for(auto i = decltype(verts.size()) {0}; i < verts.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::Push<Vector3>(l, verts[i].GetBiTangent());
		Lua::SetTableValue(l, tBiTangents);
	}
}
void Lua::ModelSubMesh::Client::GetVertexBuffer(lua::State *l, pragma::ModelSubMesh &mesh)
{
	auto &vkMesh = static_cast<CModelSubMesh &>(mesh).GetSceneMesh();
	auto &buf = vkMesh->GetVertexBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}
void Lua::ModelSubMesh::Client::GetVertexWeightBuffer(lua::State *l, pragma::ModelSubMesh &mesh)
{
	auto &vkMesh = static_cast<CModelSubMesh &>(mesh).GetSceneMesh();
	auto &buf = vkMesh->GetVertexWeightBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}
void Lua::ModelSubMesh::Client::GetAlphaBuffer(lua::State *l, pragma::ModelSubMesh &mesh)
{
	auto &vkMesh = static_cast<CModelSubMesh &>(mesh).GetSceneMesh();
	auto &buf = vkMesh->GetAlphaBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}
void Lua::ModelSubMesh::Client::GetIndexBuffer(lua::State *l, pragma::ModelSubMesh &mesh)
{
	auto &vkMesh = static_cast<CModelSubMesh &>(mesh).GetSceneMesh();
	auto &buf = vkMesh->GetIndexBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf);
}
