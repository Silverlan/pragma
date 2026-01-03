// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.shared;

import :scripting.lua.classes.collision_mesh;

#ifdef __clang__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::physics, CollisionMesh);
#endif

void Lua::CollisionMesh::register_class(luabind::class_<pragma::physics::CollisionMesh> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.scope[luabind::def("Create", static_cast<std::shared_ptr<pragma::physics::CollisionMesh> (*)(lua::State *)>(&Create))];
	classDef.scope[luabind::def("CreateBox", static_cast<std::shared_ptr<pragma::physics::CollisionMesh> (*)(lua::State *, const Vector3 &, const Vector3 &)>(&CreateBox))];
	classDef.def("GetVertices", &GetVertices);
	classDef.def("GetAABB", &GetAABB);
	classDef.def("GetMass", &pragma::physics::CollisionMesh::GetMass);
	classDef.def("SetMass", &pragma::physics::CollisionMesh::SetMass);
	classDef.def("GetBoneParentId", &pragma::physics::CollisionMesh::GetBoneParent);
	classDef.def("GetOrigin", static_cast<const Vector3 &(pragma::physics::CollisionMesh::*)() const>(&pragma::physics::CollisionMesh::GetOrigin), luabind::copy_policy<0> {});
	classDef.def("GetShape", &GetShape);
	classDef.def("GetSurfaceMaterialId", &pragma::physics::CollisionMesh::GetSurfaceMaterial);
	classDef.def("GetSurfaceMaterialIds", &GetSurfaceMaterialIds);
	classDef.def("IntersectAABB", &IntersectAABB);
	classDef.def("IsConvex", &pragma::physics::CollisionMesh::IsConvex);
	classDef.def("SetAABB", &SetAABB);
	classDef.def("SetBoneParentId", &pragma::physics::CollisionMesh::SetBoneParent);
	classDef.def("SetConvex", &pragma::physics::CollisionMesh::SetConvex);
	classDef.def("SetOrigin", &pragma::physics::CollisionMesh::SetOrigin);
	classDef.def("SetSurfaceMaterialId", static_cast<void (pragma::physics::CollisionMesh::*)(int)>(&pragma::physics::CollisionMesh::SetSurfaceMaterial));
	classDef.def("Update", &Update);
	classDef.def("AddVertex", &pragma::physics::CollisionMesh::AddVertex);
	classDef.def("GetVertexCount", &GetVertexCount);
	classDef.def("Rotate", &pragma::physics::CollisionMesh::Rotate);
	classDef.def("Translate", &pragma::physics::CollisionMesh::Translate);
	classDef.def("GetTriangles", &GetTriangles);
	classDef.def("ClipAgainstPlane", static_cast<void (pragma::physics::CollisionMesh::*)(const Vector3 &, double, pragma::physics::CollisionMesh &, pragma::physics::CollisionMesh &)>(&pragma::physics::CollisionMesh::ClipAgainstPlane));
	classDef.def("Centralize", &pragma::physics::CollisionMesh::Centralize);
	classDef.def("GetVolume", &pragma::physics::CollisionMesh::GetVolume);
	classDef.def("SetVolume", &pragma::physics::CollisionMesh::SetVolume);
	classDef.def("ClearVertices", static_cast<void (*)(lua::State *, pragma::physics::CollisionMesh &)>([](lua::State *l, pragma::physics::CollisionMesh &mesh) { mesh.GetVertices().clear(); }));
	classDef.def("ClearTriangles", static_cast<void (*)(lua::State *, pragma::physics::CollisionMesh &)>([](lua::State *l, pragma::physics::CollisionMesh &mesh) { mesh.GetTriangles().clear(); }));
	classDef.def("AddTriangle", static_cast<void (*)(lua::State *, pragma::physics::CollisionMesh &, uint32_t, uint32_t, uint32_t)>([](lua::State *l, pragma::physics::CollisionMesh &mesh, uint32_t idx0, uint32_t idx1, uint32_t idx2) {
		auto &triangles = mesh.GetTriangles();
		triangles.reserve(triangles.size() + 3);
		triangles.push_back(idx0);
		triangles.push_back(idx1);
		triangles.push_back(idx2);
	}));
	classDef.def("SetVertices", static_cast<void (*)(lua::State *, pragma::physics::CollisionMesh &, const luabind::tableT<Vector3> &)>([](lua::State *l, pragma::physics::CollisionMesh &mesh, const luabind::tableT<Vector3> &tVertices) {
		auto &verts = mesh.GetVertices();
		verts.clear();
		table_to_vector(l, tVertices, 2, verts);
	}));
	classDef.def("SetTriangles", static_cast<void (*)(lua::State *, pragma::physics::CollisionMesh &, const luabind::tableT<uint16_t> &)>([](lua::State *l, pragma::physics::CollisionMesh &mesh, const luabind::tableT<uint16_t> &tTriangles) {
		auto &tris = mesh.GetTriangles();
		tris.clear();
		table_to_vector(l, tTriangles, 2, tris);
	}));
	classDef.def("Copy", static_cast<std::shared_ptr<pragma::physics::CollisionMesh> (*)(lua::State *, pragma::physics::CollisionMesh &)>([](lua::State *l, pragma::physics::CollisionMesh &mesh) { return pragma::util::make_shared<pragma::physics::CollisionMesh>(mesh); }));

	classDef.def("SetSoftBody", &pragma::physics::CollisionMesh::SetSoftBody);
	classDef.def("IsSoftBody", &pragma::physics::CollisionMesh::IsSoftBody);
	classDef.def("GetSoftBodyMesh", &GetSoftBodyMesh);
	classDef.def("SetSoftBodyMesh", &SetSoftBodyMesh);
	classDef.def("GetSoftBodyInfo", &GetSoftBodyInfo);
	classDef.def("GetSoftBodyTriangles", &GetSoftBodyTriangles);
	classDef.def("SetSoftBodyTriangles", &SetSoftBodyTriangles);

	classDef.def("AddSoftBodyAnchor", static_cast<luabind::optional<uint32_t> (*)(lua::State *, pragma::physics::CollisionMesh &, uint32_t, uint32_t, pragma::physics::CollisionMesh::SoftBodyAnchor::Flags, float)>(&AddSoftBodyAnchor));
	classDef.def("AddSoftBodyAnchor", static_cast<luabind::optional<uint32_t> (*)(lua::State *, pragma::physics::CollisionMesh &, uint32_t, uint32_t, pragma::physics::CollisionMesh::SoftBodyAnchor::Flags)>(&AddSoftBodyAnchor));
	classDef.def("AddSoftBodyAnchor", static_cast<luabind::optional<uint32_t> (*)(lua::State *, pragma::physics::CollisionMesh &, uint32_t, uint32_t)>(&AddSoftBodyAnchor));
	classDef.def("RemoveSoftBodyAnchor", &pragma::physics::CollisionMesh::RemoveSoftBodyAnchor);
	classDef.def("ClearSoftBodyAnchors", &pragma::physics::CollisionMesh::ClearSoftBodyAnchors);
	classDef.def("GetSoftBodyAnchors", &GetSoftBodyAnchors);

	classDef.add_static_constant("FSOFTBODY_ANCHOR_NONE", pragma::math::to_integral(pragma::physics::CollisionMesh::SoftBodyAnchor::Flags::None));
	classDef.add_static_constant("FSOFTBODY_ANCHOR_RIGID", pragma::math::to_integral(pragma::physics::CollisionMesh::SoftBodyAnchor::Flags::Rigid));
	classDef.add_static_constant("FSOFTBODY_ANCHOR_DISABLE_COLLISIONS", pragma::math::to_integral(pragma::physics::CollisionMesh::SoftBodyAnchor::Flags::DisableCollisions));
}
std::shared_ptr<pragma::physics::CollisionMesh> Lua::CollisionMesh::Create(lua::State *l) { return pragma::physics::CollisionMesh::Create(pragma::Engine::Get()->GetNetworkState(l)->GetGameState()); }
std::shared_ptr<pragma::physics::CollisionMesh> Lua::CollisionMesh::CreateBox(lua::State *l, const Vector3 &cmin, const Vector3 &cmax)
{
	auto min = cmin;
	auto max = cmax;
	uvec::to_min_max(min, max);
	auto mesh = pragma::physics::CollisionMesh::Create(pragma::Engine::Get()->GetNetworkState(l)->GetGameState());
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
luabind::tableT<Vector3> Lua::CollisionMesh::GetVertices(lua::State *l, pragma::physics::CollisionMesh &mesh) { return vector_to_table(l, mesh.GetVertices()); }
uint32_t Lua::CollisionMesh::GetVertexCount(lua::State *l, pragma::physics::CollisionMesh &mesh) { return mesh.GetVertices().size(); }
luabind::mult<Vector3, Vector3> Lua::CollisionMesh::GetAABB(lua::State *l, pragma::physics::CollisionMesh &mesh)
{
	Vector3 min, max;
	mesh.GetAABB(&min, &max);
	return {l, min, max};
}
luabind::optional<pragma::physics::IShape> Lua::CollisionMesh::GetShape(lua::State *l, pragma::physics::CollisionMesh &mesh)
{
	auto shape = mesh.GetShape();
	if(shape == nullptr)
		return nil;
	return shape->GetLuaObject(l);
}
luabind::tableT<int32_t> Lua::CollisionMesh::GetSurfaceMaterialIds(lua::State *l, pragma::physics::CollisionMesh &mesh) { return vector_to_table(l, mesh.GetSurfaceMaterials()); }
bool Lua::CollisionMesh::IntersectAABB(lua::State *l, pragma::physics::CollisionMesh &mesh, const Vector3 &min, const Vector3 &max) { return mesh.IntersectAABB(const_cast<Vector3 *>(&min), const_cast<Vector3 *>(&max)); }
void Lua::CollisionMesh::SetAABB(lua::State *, pragma::physics::CollisionMesh &mesh, const Vector3 &min, const Vector3 &max) { mesh.SetAABB(const_cast<Vector3 &>(min), const_cast<Vector3 &>(max)); }
void Lua::CollisionMesh::Update(lua::State *, pragma::physics::CollisionMesh &mesh) { mesh.Update(); }
void Lua::CollisionMesh::GetTriangles(lua::State *l, pragma::physics::CollisionMesh &mesh)
{
	auto &triangles = mesh.GetTriangles();
	auto t = CreateTable(l);
	int32_t triangleIdx = 1;
	for(auto idx : triangles) {
		PushInt(l, triangleIdx++);
		PushInt(l, idx);
		SetTableValue(l, t);
	}
}
void Lua::CollisionMesh::GetSoftBodyMesh(lua::State *l, pragma::physics::CollisionMesh &mesh)
{
	auto *subMesh = mesh.GetSoftBodyMesh();
	if(subMesh == nullptr)
		return;
	Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, subMesh->shared_from_this());
}
void Lua::CollisionMesh::SetSoftBodyMesh(lua::State *l, pragma::physics::CollisionMesh &mesh, const std::shared_ptr<pragma::geometry::ModelSubMesh> &subMesh) { mesh.SetSoftBodyMesh(*subMesh); }
void Lua::CollisionMesh::GetSoftBodyInfo(lua::State *l, pragma::physics::CollisionMesh &mesh)
{
	auto *sbInfo = mesh.GetSoftBodyInfo();
	if(sbInfo == nullptr)
		return;
	Lua::Push<pragma::physics::PhysSoftBodyInfo *>(l, sbInfo);
}
luabind::optional<luabind::tableT<uint32_t>> Lua::CollisionMesh::GetSoftBodyTriangles(lua::State *l, pragma::physics::CollisionMesh &mesh)
{
	auto *sbTriangles = mesh.GetSoftBodyTriangles();
	if(sbTriangles == nullptr)
		return nil;
	return vector_to_table(l, *sbTriangles);
}
void Lua::CollisionMesh::SetSoftBodyTriangles(lua::State *l, pragma::physics::CollisionMesh &mesh, const luabind::tableT<uint32_t> &o)
{
	auto *sbTriangles = mesh.GetSoftBodyTriangles();
	if(sbTriangles == nullptr)
		return;
	auto numIndices = GetObjectLength(l, o);
	sbTriangles->clear();
	sbTriangles->resize(numIndices);
	table_to_vector(l, o, 2, *sbTriangles);
}
luabind::optional<uint32_t> Lua::CollisionMesh::AddSoftBodyAnchor(lua::State *l, pragma::physics::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx, pragma::physics::CollisionMesh::SoftBodyAnchor::Flags flags, float influence)
{
	auto idx = 0u;
	auto b = mesh.AddSoftBodyAnchor(vertIdx, boneIdx, static_cast<pragma::physics::CollisionMesh::SoftBodyAnchor::Flags>(flags), influence, &idx);
	if(b == false)
		return nil;
	return {l, idx};
}
luabind::optional<uint32_t> Lua::CollisionMesh::AddSoftBodyAnchor(lua::State *l, pragma::physics::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx, pragma::physics::CollisionMesh::SoftBodyAnchor::Flags flags) { return AddSoftBodyAnchor(l, mesh, vertIdx, boneIdx, flags, 1.f); }
luabind::optional<uint32_t> Lua::CollisionMesh::AddSoftBodyAnchor(lua::State *l, pragma::physics::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx) { return AddSoftBodyAnchor(l, mesh, vertIdx, boneIdx, pragma::physics::CollisionMesh::SoftBodyAnchor::Flags::None); }
static luabind::object push_soft_body_anchor(lua::State *l, const pragma::physics::CollisionMesh::SoftBodyAnchor &anchor)
{
	auto t = luabind::newtable(l);
	t["vertexIndex"] = anchor.vertexIndex;
	t["boneId"] = anchor.boneId;
	t["influence"] = anchor.influence;
	t["flags"] = pragma::math::to_integral(anchor.flags);
	return t;
}
luabind::tableT<void> Lua::CollisionMesh::GetSoftBodyAnchors(lua::State *l, pragma::physics::CollisionMesh &mesh)
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
