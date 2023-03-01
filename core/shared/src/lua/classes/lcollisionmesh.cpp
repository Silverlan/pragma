/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lcollisionmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/physics/shape.hpp"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include <pragma/lua/luaapi.h>
#include <luabind/copy_policy.hpp>

extern DLLNETWORK Engine *engine;

void Lua::CollisionMesh::register_class(luabind::class_<::CollisionMesh> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.scope[luabind::def("Create", static_cast<std::shared_ptr<::CollisionMesh> (*)(lua_State *)>(&Create))];
	classDef.scope[luabind::def("CreateBox", static_cast<std::shared_ptr<::CollisionMesh> (*)(lua_State *, const Vector3 &, const Vector3 &)>(&CreateBox))];
	classDef.def("GetVertices", &GetVertices);
	classDef.def("GetAABB", &GetAABB);
	classDef.def("GetMass", &::CollisionMesh::GetMass);
	classDef.def("SetMass", &::CollisionMesh::SetMass);
	classDef.def("GetBoneParentId", &::CollisionMesh::GetBoneParent);
	classDef.def("GetOrigin", static_cast<const Vector3 &(::CollisionMesh::*)() const>(&::CollisionMesh::GetOrigin), luabind::copy_policy<0> {});
	classDef.def("GetShape", &GetShape);
	classDef.def("GetSurfaceMaterialId", &::CollisionMesh::GetSurfaceMaterial);
	classDef.def("GetSurfaceMaterialIds", &GetSurfaceMaterialIds);
	classDef.def("IntersectAABB", &IntersectAABB);
	classDef.def("IsConvex", &::CollisionMesh::IsConvex);
	classDef.def("SetAABB", &SetAABB);
	classDef.def("SetBoneParentId", &::CollisionMesh::SetBoneParent);
	classDef.def("SetConvex", &::CollisionMesh::SetConvex);
	classDef.def("SetOrigin", &::CollisionMesh::SetOrigin);
	classDef.def("SetSurfaceMaterialId", static_cast<void (::CollisionMesh::*)(int)>(&::CollisionMesh::SetSurfaceMaterial));
	classDef.def("Update", &Update);
	classDef.def("AddVertex", &::CollisionMesh::AddVertex);
	classDef.def("GetVertexCount", &GetVertexCount);
	classDef.def("Rotate", &::CollisionMesh::Rotate);
	classDef.def("Translate", &::CollisionMesh::Translate);
	classDef.def("GetTriangles", &GetTriangles);
	classDef.def("ClipAgainstPlane", static_cast<void (::CollisionMesh::*)(const Vector3 &, double, ::CollisionMesh &, ::CollisionMesh &)>(&::CollisionMesh::ClipAgainstPlane));
	classDef.def("Centralize", &::CollisionMesh::Centralize);
	classDef.def("GetVolume", &::CollisionMesh::GetVolume);
	classDef.def("SetVolume", &::CollisionMesh::SetVolume);
	classDef.def("ClearVertices", static_cast<void (*)(lua_State *, ::CollisionMesh &)>([](lua_State *l, ::CollisionMesh &mesh) { mesh.GetVertices().clear(); }));
	classDef.def("ClearTriangles", static_cast<void (*)(lua_State *, ::CollisionMesh &)>([](lua_State *l, ::CollisionMesh &mesh) { mesh.GetTriangles().clear(); }));
	classDef.def("AddTriangle", static_cast<void (*)(lua_State *, ::CollisionMesh &, uint32_t, uint32_t, uint32_t)>([](lua_State *l, ::CollisionMesh &mesh, uint32_t idx0, uint32_t idx1, uint32_t idx2) {
		auto &triangles = mesh.GetTriangles();
		triangles.reserve(triangles.size() + 3);
		triangles.push_back(idx0);
		triangles.push_back(idx1);
		triangles.push_back(idx2);
	}));
	classDef.def("SetVertices", static_cast<void (*)(lua_State *, ::CollisionMesh &, const luabind::tableT<Vector3> &)>([](lua_State *l, ::CollisionMesh &mesh, const luabind::tableT<Vector3> &tVertices) {
		auto &verts = mesh.GetVertices();
		verts.clear();
		Lua::table_to_vector(l, tVertices, 2, verts);
	}));
	classDef.def("SetTriangles", static_cast<void (*)(lua_State *, ::CollisionMesh &, const luabind::tableT<uint16_t> &)>([](lua_State *l, ::CollisionMesh &mesh, const luabind::tableT<uint16_t> &tTriangles) {
		auto &tris = mesh.GetTriangles();
		tris.clear();
		Lua::table_to_vector(l, tTriangles, 2, tris);
	}));
	classDef.def("Copy", static_cast<std::shared_ptr<::CollisionMesh> (*)(lua_State *, ::CollisionMesh &)>([](lua_State *l, ::CollisionMesh &mesh) { return std::make_shared<::CollisionMesh>(mesh); }));

	classDef.def("SetSoftBody", &::CollisionMesh::SetSoftBody);
	classDef.def("IsSoftBody", &::CollisionMesh::IsSoftBody);
	classDef.def("GetSoftBodyMesh", &GetSoftBodyMesh);
	classDef.def("SetSoftBodyMesh", &SetSoftBodyMesh);
	classDef.def("GetSoftBodyInfo", &GetSoftBodyInfo);
	classDef.def("GetSoftBodyTriangles", &GetSoftBodyTriangles);
	classDef.def("SetSoftBodyTriangles", &SetSoftBodyTriangles);

	classDef.def("AddSoftBodyAnchor", static_cast<luabind::optional<uint32_t> (*)(lua_State *, ::CollisionMesh &, uint32_t, uint32_t, ::CollisionMesh::SoftBodyAnchor::Flags, float)>(&AddSoftBodyAnchor));
	classDef.def("AddSoftBodyAnchor", static_cast<luabind::optional<uint32_t> (*)(lua_State *, ::CollisionMesh &, uint32_t, uint32_t, ::CollisionMesh::SoftBodyAnchor::Flags)>(&AddSoftBodyAnchor));
	classDef.def("AddSoftBodyAnchor", static_cast<luabind::optional<uint32_t> (*)(lua_State *, ::CollisionMesh &, uint32_t, uint32_t)>(&AddSoftBodyAnchor));
	classDef.def("RemoveSoftBodyAnchor", &::CollisionMesh::RemoveSoftBodyAnchor);
	classDef.def("ClearSoftBodyAnchors", &::CollisionMesh::ClearSoftBodyAnchors);
	classDef.def("GetSoftBodyAnchors", &GetSoftBodyAnchors);

	classDef.add_static_constant("FSOFTBODY_ANCHOR_NONE", umath::to_integral(::CollisionMesh::SoftBodyAnchor::Flags::None));
	classDef.add_static_constant("FSOFTBODY_ANCHOR_RIGID", umath::to_integral(::CollisionMesh::SoftBodyAnchor::Flags::Rigid));
	classDef.add_static_constant("FSOFTBODY_ANCHOR_DISABLE_COLLISIONS", umath::to_integral(::CollisionMesh::SoftBodyAnchor::Flags::DisableCollisions));
}
std::shared_ptr<::CollisionMesh> Lua::CollisionMesh::Create(lua_State *l) { return ::CollisionMesh::Create(engine->GetNetworkState(l)->GetGameState()); }
std::shared_ptr<::CollisionMesh> Lua::CollisionMesh::CreateBox(lua_State *l, const Vector3 &cmin, const Vector3 &cmax)
{
	auto min = cmin;
	auto max = cmax;
	uvec::to_min_max(min, max);
	auto mesh = ::CollisionMesh::Create(engine->GetNetworkState(l)->GetGameState());
	mesh->AddVertex(min);
	mesh->AddVertex(Vector3(max.x, min.y, min.z));
	mesh->AddVertex(Vector3(max.x, min.y, max.z));
	mesh->AddVertex(Vector3(max.x, max.y, min.z));
	mesh->AddVertex(max);
	mesh->AddVertex(Vector3(min.x, max.y, min.z));
	mesh->AddVertex(Vector3(min.x, min.y, max.z));
	mesh->AddVertex(Vector3(min.x, max.y, max.z));
	mesh->Update();
	return mesh;
}
luabind::tableT<Vector3> Lua::CollisionMesh::GetVertices(lua_State *l, ::CollisionMesh &mesh) { return Lua::vector_to_table(l, mesh.GetVertices()); }
uint32_t Lua::CollisionMesh::GetVertexCount(lua_State *l, ::CollisionMesh &mesh) { return mesh.GetVertices().size(); }
luabind::mult<Vector3, Vector3> Lua::CollisionMesh::GetAABB(lua_State *l, ::CollisionMesh &mesh)
{
	Vector3 min, max;
	mesh.GetAABB(&min, &max);
	return {l, min, max};
}
luabind::optional<pragma::physics::IShape> Lua::CollisionMesh::GetShape(lua_State *l, ::CollisionMesh &mesh)
{
	auto shape = mesh.GetShape();
	if(shape == nullptr)
		return nil;
	return shape->GetLuaObject();
}
luabind::tableT<int32_t> Lua::CollisionMesh::GetSurfaceMaterialIds(lua_State *l, ::CollisionMesh &mesh) { return Lua::vector_to_table(l, mesh.GetSurfaceMaterials()); }
bool Lua::CollisionMesh::IntersectAABB(lua_State *l, ::CollisionMesh &mesh, const Vector3 &min, const Vector3 &max) { return mesh.IntersectAABB(const_cast<Vector3 *>(&min), const_cast<Vector3 *>(&max)); }
void Lua::CollisionMesh::SetAABB(lua_State *, ::CollisionMesh &mesh, const Vector3 &min, const Vector3 &max) { mesh.SetAABB(const_cast<Vector3 &>(min), const_cast<Vector3 &>(max)); }
void Lua::CollisionMesh::Update(lua_State *, ::CollisionMesh &mesh) { mesh.Update(); }
void Lua::CollisionMesh::GetTriangles(lua_State *l, ::CollisionMesh &mesh)
{
	auto &triangles = mesh.GetTriangles();
	auto t = Lua::CreateTable(l);
	int32_t triangleIdx = 1;
	for(auto idx : triangles) {
		Lua::PushInt(l, triangleIdx++);
		Lua::PushInt(l, idx);
		Lua::SetTableValue(l, t);
	}
}
void Lua::CollisionMesh::GetSoftBodyMesh(lua_State *l, ::CollisionMesh &mesh)
{
	auto *subMesh = mesh.GetSoftBodyMesh();
	if(subMesh == nullptr)
		return;
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, subMesh->shared_from_this());
}
void Lua::CollisionMesh::SetSoftBodyMesh(lua_State *l, ::CollisionMesh &mesh, const std::shared_ptr<::ModelSubMesh> &subMesh) { mesh.SetSoftBodyMesh(*subMesh); }
void Lua::CollisionMesh::GetSoftBodyInfo(lua_State *l, ::CollisionMesh &mesh)
{
	auto *sbInfo = mesh.GetSoftBodyInfo();
	if(sbInfo == nullptr)
		return;
	Lua::Push<PhysSoftBodyInfo *>(l, sbInfo);
}
luabind::optional<luabind::tableT<uint32_t>> Lua::CollisionMesh::GetSoftBodyTriangles(lua_State *l, ::CollisionMesh &mesh)
{
	auto *sbTriangles = mesh.GetSoftBodyTriangles();
	if(sbTriangles == nullptr)
		return nil;
	return Lua::vector_to_table(l, *sbTriangles);
}
void Lua::CollisionMesh::SetSoftBodyTriangles(lua_State *l, ::CollisionMesh &mesh, const luabind::tableT<uint32_t> &o)
{
	auto *sbTriangles = mesh.GetSoftBodyTriangles();
	if(sbTriangles == nullptr)
		return;
	auto numIndices = Lua::GetObjectLength(l, o);
	sbTriangles->clear();
	sbTriangles->resize(numIndices);
	Lua::table_to_vector(l, o, 2, *sbTriangles);
}
luabind::optional<uint32_t> Lua::CollisionMesh::AddSoftBodyAnchor(lua_State *l, ::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx, ::CollisionMesh::SoftBodyAnchor::Flags flags, float influence)
{
	auto idx = 0u;
	auto b = mesh.AddSoftBodyAnchor(vertIdx, boneIdx, static_cast<::CollisionMesh::SoftBodyAnchor::Flags>(flags), influence, &idx);
	if(b == false)
		return nil;
	return {l, idx};
}
luabind::optional<uint32_t> Lua::CollisionMesh::AddSoftBodyAnchor(lua_State *l, ::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx, ::CollisionMesh::SoftBodyAnchor::Flags flags) { return AddSoftBodyAnchor(l, mesh, vertIdx, boneIdx, flags, 1.f); }
luabind::optional<uint32_t> Lua::CollisionMesh::AddSoftBodyAnchor(lua_State *l, ::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx) { return AddSoftBodyAnchor(l, mesh, vertIdx, boneIdx, ::CollisionMesh::SoftBodyAnchor::Flags::None); }
static luabind::object push_soft_body_anchor(lua_State *l, const ::CollisionMesh::SoftBodyAnchor &anchor)
{
	auto t = luabind::newtable(l);
	t["vertexIndex"] = anchor.vertexIndex;
	t["boneId"] = anchor.boneId;
	t["influence"] = anchor.influence;
	t["flags"] = umath::to_integral(anchor.flags);
	return t;
}
luabind::tableT<void> Lua::CollisionMesh::GetSoftBodyAnchors(lua_State *l, ::CollisionMesh &mesh)
{
	auto *anchors = mesh.GetSoftBodyAnchors();
	if(anchors == nullptr)
		return nil;
	auto t = luabind::newtable(l);
	auto anchorIdx = 1u;
	for(auto &anchor : *anchors)
		t[anchorIdx++] = push_soft_body_anchor(l, anchor);
	return t;
}
