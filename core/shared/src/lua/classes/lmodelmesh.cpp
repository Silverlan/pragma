/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lmodelmesh.h"
#include "luasystem.h"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/policies/core_policies.hpp"
#include "pragma/lua/libraries/lutil.hpp"
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>

extern DLLNETWORK Engine *engine;

void Lua::ModelMesh::register_class(luabind::class_<::ModelMesh> &classDef)
{
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("GetVertexCount", &Lua::ModelMesh::GetVertexCount);
	classDef.def("GetIndexCount", &Lua::ModelMesh::GetTriangleVertexCount);
	classDef.def("GetTriangleCount", &Lua::ModelMesh::GetTriangleCount);
	classDef.def("GetSubMeshes", &Lua::ModelMesh::GetSubMeshes);
	classDef.def("AddSubMesh", &Lua::ModelMesh::AddSubMesh);
	classDef.def(
	  "AddSubMesh", +[](lua_State *l, ::ModelMesh &mesh, ::ModelSubMesh &subMesh, uint32_t pos) {
		  auto &subMeshes = mesh.GetSubMeshes();
		  subMeshes.insert(subMeshes.begin() + pos, subMesh.shared_from_this());
	  });
	classDef.def("Update", static_cast<void (*)(lua_State *, ::ModelMesh &)>(&Lua::ModelMesh::Update));
	classDef.def("Update", static_cast<void (*)(lua_State *, ::ModelMesh &, uint32_t)>(&Lua::ModelMesh::Update));
	classDef.def("GetBounds", &Lua::ModelMesh::GetBounds);
	classDef.def("SetCenter", &Lua::ModelMesh::SetCenter);
	classDef.def("GetCenter", &Lua::ModelMesh::GetCenter);
	classDef.def("Centralize", &Lua::ModelMesh::Centralize);
	classDef.def("Scale", &Lua::ModelMesh::Scale);
	classDef.def("GetReferenceId", static_cast<void (*)(lua_State *, ::ModelMesh &)>([](lua_State *l, ::ModelMesh &mesh) { Lua::PushInt(l, mesh.GetReferenceId()); }));
	classDef.def("Translate", static_cast<void (*)(lua_State *, ::ModelMesh &, const Vector3 &)>([](lua_State *l, ::ModelMesh &mesh, const Vector3 &translation) { mesh.Translate(translation); }));
	classDef.def("Rotate", static_cast<void (*)(lua_State *, ::ModelMesh &, const Quat &)>([](lua_State *l, ::ModelMesh &mesh, const Quat &rotation) { mesh.Rotate(rotation); }));
	classDef.def("ClearSubMeshes", static_cast<void (*)(lua_State *, ::ModelMesh &)>([](lua_State *l, ::ModelMesh &mesh) { mesh.GetSubMeshes().clear(); }));
	classDef.def("RemoveSubMesh", static_cast<void (*)(lua_State *, ::ModelMesh &, uint32_t)>([](lua_State *l, ::ModelMesh &mesh, uint32_t i) {
		auto &subMeshes = mesh.GetSubMeshes();
		if(i >= subMeshes.size())
			return;
		subMeshes.erase(subMeshes.begin() + i);
	}));
	classDef.def(
	  "RemoveSubMesh", +[](lua_State *l, ::ModelMesh &mesh, const std::string &uuid) {
		  auto &subMeshes = mesh.GetSubMeshes();
		  auto uuidValue = ::util::uuid_string_to_bytes(uuid);
		  auto it = std::find_if(subMeshes.begin(), subMeshes.end(), [&uuidValue](const std::shared_ptr<::ModelSubMesh> &subMesh) { return subMesh->GetUuid() == uuidValue; });
		  if(it == subMeshes.end())
			  return;
		  subMeshes.erase(it);
	  });
	classDef.def(
	  "FindSubMesh", +[](lua_State *l, ::ModelMesh &mesh, const std::string &uuid) -> std::shared_ptr<::ModelSubMesh> {
		  auto &subMeshes = mesh.GetSubMeshes();
		  auto uuidValue = ::util::uuid_string_to_bytes(uuid);
		  auto it = std::find_if(subMeshes.begin(), subMeshes.end(), [&uuidValue](const std::shared_ptr<::ModelSubMesh> &subMesh) { return subMesh->GetUuid() == uuidValue; });
		  if(it == subMeshes.end())
			  return nullptr;
		  return *it;
	  });
	classDef.def("SetSubMeshes", static_cast<void (*)(lua_State *, ::ModelMesh &, luabind::object)>([](lua_State *l, ::ModelMesh &mesh, luabind::object tSubMeshes) {
		auto idxSubMeshes = 2;
		Lua::CheckTable(l, idxSubMeshes);
		auto &subMeshes = mesh.GetSubMeshes();
		subMeshes = {};
		auto numSubMeshes = Lua::GetObjectLength(l, idxSubMeshes);
		subMeshes.reserve(numSubMeshes);
		for(auto i = decltype(numSubMeshes) {0u}; i < numSubMeshes; ++i) {
			Lua::PushInt(l, i + 1);
			Lua::GetTableValue(l, idxSubMeshes);
			auto &subMesh = Lua::Check<std::shared_ptr<::ModelSubMesh>>(l, -1);
			subMeshes.push_back(subMesh);
			Lua::Pop(l, 1);
		}
	}));
	classDef.def("GetSubMeshCount", static_cast<void (*)(lua_State *, ::ModelMesh &)>([](lua_State *l, ::ModelMesh &mesh) { Lua::PushInt(l, mesh.GetSubMeshes().size()); }));
	classDef.def("GetSubMesh", static_cast<void (*)(lua_State *, ::ModelMesh &, uint32_t)>([](lua_State *l, ::ModelMesh &mesh, uint32_t index) {
		auto &subMeshes = mesh.GetSubMeshes();
		if(index >= subMeshes.size())
			return;
		Lua::Push(l, subMeshes.at(index));
	}));
}
void Lua::ModelMesh::GetVertexCount(lua_State *l, ::ModelMesh &mesh) { Lua::PushInt(l, mesh.GetVertexCount()); }
void Lua::ModelMesh::GetTriangleVertexCount(lua_State *l, ::ModelMesh &mesh) { Lua::PushInt(l, mesh.GetIndexCount()); }
void Lua::ModelMesh::GetTriangleCount(lua_State *l, ::ModelMesh &mdl) { Lua::PushInt(l, mdl.GetTriangleCount()); }
void Lua::ModelMesh::GetSubMeshes(lua_State *l, ::ModelMesh &mdl)
{
	auto &subMeshes = mdl.GetSubMeshes();
	Lua::CreateTable(l);
	auto top = Lua::GetStackTop(l);
	UInt i = 0;
	for(auto it = subMeshes.begin(); it != subMeshes.end(); ++it) {
		auto &subMesh = *it;
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, subMesh);
		Lua::SetTableValue(l, top, CInt32(i + 1));
		i++;
	}
}
void Lua::ModelMesh::AddSubMesh(lua_State *, ::ModelMesh &mdl, ::ModelSubMesh &mesh) { mdl.AddSubMesh(mesh.shared_from_this()); }
void Lua::ModelMesh::Update(lua_State *, ::ModelMesh &mdl) { mdl.Update(); }
void Lua::ModelMesh::Update(lua_State *, ::ModelMesh &mdl, uint32_t flags) { mdl.Update(static_cast<ModelUpdateFlags>(flags)); }
void Lua::ModelMesh::GetBounds(lua_State *l, ::ModelMesh &mdl)
{
	Vector3 min, max;
	mdl.GetBounds(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::ModelMesh::SetCenter(lua_State *, ::ModelMesh &mdl, const Vector3 &center) { mdl.SetCenter(center); }
void Lua::ModelMesh::GetCenter(lua_State *l, ::ModelMesh &mdl) { Lua::Push<Vector3>(l, mdl.GetCenter()); }
void Lua::ModelMesh::Centralize(lua_State *, ::ModelMesh &mdl) { mdl.Centralize(); }
void Lua::ModelMesh::Scale(lua_State *l, ::ModelMesh &mdl, const Vector3 &scale) { mdl.Scale(scale); }

////////////////////////////////////////

void Lua::ModelSubMesh::register_class(luabind::class_<::ModelSubMesh> &classDef)
{
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def(luabind::tostring(luabind::self));
	classDef.scope[luabind::def("create_quad", static_cast<std::shared_ptr<::ModelSubMesh> (*)(Game &, const pragma::model::QuadCreateInfo &)>(&pragma::model::create_quad))];
	classDef.scope[luabind::def("create_box", static_cast<std::shared_ptr<::ModelSubMesh> (*)(Game &, const pragma::model::BoxCreateInfo &)>(&pragma::model::create_box))];
	classDef.scope[luabind::def("create_sphere", static_cast<std::shared_ptr<::ModelSubMesh> (*)(Game &, const pragma::model::SphereCreateInfo &)>(&pragma::model::create_sphere))];
	classDef.scope[luabind::def("create_cylinder", static_cast<std::shared_ptr<::ModelSubMesh> (*)(Game &, const pragma::model::CylinderCreateInfo &)>(&pragma::model::create_cylinder))];
	classDef.scope[luabind::def("create_cone", static_cast<std::shared_ptr<::ModelSubMesh> (*)(Game &, const pragma::model::ConeCreateInfo &)>(&pragma::model::create_cone))];
	classDef.scope[luabind::def("create_elliptic_cone", static_cast<std::shared_ptr<::ModelSubMesh> (*)(Game &, const pragma::model::EllipticConeCreateInfo &)>(&pragma::model::create_elliptic_cone))];
	classDef.scope[luabind::def("create_circle", static_cast<std::shared_ptr<::ModelSubMesh> (*)(Game &, const pragma::model::CircleCreateInfo &)>(&pragma::model::create_circle))];
	classDef.scope[luabind::def("create_ring", static_cast<std::shared_ptr<::ModelSubMesh> (*)(Game &, const pragma::model::RingCreateInfo &)>(&pragma::model::create_ring))];
	classDef.def("SetName", &::ModelSubMesh::SetName);
	classDef.def("GetName", &::ModelSubMesh::GetName);
	classDef.def(
	  "GetUuid", +[](::ModelSubMesh &mesh) -> std::string { return ::util::uuid_to_string(mesh.GetUuid()); });
	classDef.def(
	  "SetUuid", +[](::ModelSubMesh &mesh, const std::string &uuid) { mesh.SetUuid(::util::uuid_string_to_bytes(uuid)); });
	classDef.def(
	  "SetUuid", +[](::ModelSubMesh &mesh, const Lua::util::Uuid &uuid) { mesh.SetUuid(uuid.value); }, luabind::const_ref_policy<2> {});
	classDef.def("GetSkinTextureIndex", &Lua::ModelSubMesh::GetSkinTextureIndex);
	classDef.def("FlipTriangleWindingOrder", &Lua::ModelSubMesh::FlipTriangleWindingOrder);
	classDef.def("GetVertexCount", &Lua::ModelSubMesh::GetVertexCount);
	classDef.def("GetAlphaCount", &::ModelSubMesh::GetAlphaCount);
	classDef.def("SetAlphaCount", &::ModelSubMesh::SetAlphaCount);
	classDef.def("SetVertexCount", &Lua::ModelSubMesh::SetVertexCount);
	classDef.def("SetIndexCount", &Lua::ModelSubMesh::SetIndexCount);
	classDef.def("GetIndexCount", &Lua::ModelSubMesh::GetTriangleVertexCount);
	classDef.def("GetTriangleCount", &Lua::ModelSubMesh::GetTriangleCount);
	classDef.def("GetIndexType", &::ModelSubMesh::GetIndexType);
	classDef.def("SetIndexType", &::ModelSubMesh::SetIndexType);
	classDef.def("GetVertices", &Lua::ModelSubMesh::GetVertices);
	classDef.def("GetIndices", &Lua::ModelSubMesh::GetTriangles);
	classDef.def("AddUVSet", &Lua::ModelSubMesh::AddUVSet);
	classDef.def("GetUVs", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>(&Lua::ModelSubMesh::GetUVMapping));
	classDef.def("GetUVs", static_cast<luabind::object (*)(lua_State *, ::ModelSubMesh &, const std::string &)>(&Lua::ModelSubMesh::GetUVMapping));
	classDef.def("GetUVSetNames", &Lua::ModelSubMesh::GetUVSetNames);
	classDef.def(
	  "HasUvSet", +[](lua_State *l, ::ModelSubMesh &mesh, const std::string &name) { return mesh.GetUVSet(name) != nullptr; });
	classDef.def(
	  "HasVertexWeights", +[](lua_State *l, ::ModelSubMesh &mesh) { return !mesh.GetVertexWeights().empty(); });
	classDef.def("GetNormals", &Lua::ModelSubMesh::GetNormalMapping);
	classDef.def("GetVertexWeights", &Lua::ModelSubMesh::GetVertexWeights);
	classDef.def("AddTriangle", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const umath::Vertex &, const umath::Vertex &, const umath::Vertex &)>(&Lua::ModelSubMesh::AddTriangle));
	classDef.def("AddTriangle", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t, uint32_t, uint32_t)>(&Lua::ModelSubMesh::AddTriangle));
	classDef.def("AddIndex", &::ModelSubMesh::AddIndex);
	classDef.def("SetIndex", &::ModelSubMesh::SetIndex);
	classDef.def("GetIndex", &::ModelSubMesh::GetIndex);
	classDef.def("SetSkinTextureIndex", &Lua::ModelSubMesh::SetSkinTextureIndex);
	classDef.def("Update", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t)>(&Lua::ModelSubMesh::Update));
	classDef.def("Update", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>(&Lua::ModelSubMesh::Update));
	classDef.def("AddVertex", &Lua::ModelSubMesh::AddVertex);
	classDef.def("GetBounds", &Lua::ModelSubMesh::GetBounds);
	classDef.def("GetCenter", &Lua::ModelSubMesh::GetCenter);
	classDef.def("SetCenter", &::ModelSubMesh::SetCenter);
	classDef.def("SetVertex", &Lua::ModelSubMesh::SetVertex);
	classDef.def("SetVertexPosition", &Lua::ModelSubMesh::SetVertexPosition);
	classDef.def("SetVertexNormal", &Lua::ModelSubMesh::SetVertexNormal);
	classDef.def("SetVertexUV", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const std::string &, uint32_t, const ::Vector2 &)>(&Lua::ModelSubMesh::SetVertexUV));
	classDef.def("SetVertexUV", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t, const ::Vector2 &)>(&Lua::ModelSubMesh::SetVertexUV));
	classDef.def("SetVertexAlpha", &Lua::ModelSubMesh::SetVertexAlpha);
	classDef.def("SetVertexWeight", &Lua::ModelSubMesh::SetVertexWeight);
	classDef.def("GetVertex", &Lua::ModelSubMesh::GetVertex);
	classDef.def("GetVertexPosition", &Lua::ModelSubMesh::GetVertexPosition);
	classDef.def("GetVertexNormal", &Lua::ModelSubMesh::GetVertexNormal);
	classDef.def("GetVertexUV", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const std::string &, uint32_t)>(&Lua::ModelSubMesh::GetVertexUV));
	classDef.def("GetVertexUV", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t)>(&Lua::ModelSubMesh::GetVertexUV));
	classDef.def("GetVertexAlpha", &Lua::ModelSubMesh::GetVertexAlpha);
	classDef.def("GetVertexWeight", &Lua::ModelSubMesh::GetVertexWeight);
	classDef.def("Optimize", &::ModelSubMesh::Optimize);
#ifdef _WIN32
	classDef.def("Optimize", &::ModelSubMesh::Optimize, luabind::default_parameter_policy<2, double {umath::VERTEX_EPSILON}> {});
#else
	classDef.def(
	  "Optimize", +[](::ModelSubMesh &mesh) { return mesh.Optimize(); });
#endif
	classDef.def("GenerateNormals", &Lua::ModelSubMesh::GenerateNormals);
	classDef.def("NormalizeUVCoordinates", &Lua::ModelSubMesh::NormalizeUVCoordinates);
	classDef.def("ClipAgainstPlane", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const Vector3 &, double, bool, luabind::object)>(&Lua::ModelSubMesh::ClipAgainstPlane));
	classDef.def("ClipAgainstPlane", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const Vector3 &, double, bool)>(&Lua::ModelSubMesh::ClipAgainstPlane));
	classDef.def("ClipAgainstPlane", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const Vector3 &, double)>(&Lua::ModelSubMesh::ClipAgainstPlane));
	classDef.def("ApplyUVMapping", static_cast<void (*)(lua_State *, ::ModelSubMesh &, ::Model &, const Vector3 &, const Vector3 &, float, float, float, float)>(&Lua::ModelSubMesh::ApplyUVMapping));
	classDef.def("ApplyUVMapping", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const Vector3 &, const Vector3 &, uint32_t, uint32_t, float, float, float, float)>(&Lua::ModelSubMesh::ApplyUVMapping));
	classDef.def("Scale", &Lua::ModelSubMesh::Scale);
	classDef.def(
	  "GetTriangle", +[](lua_State *l, ::ModelSubMesh &mesh, uint32_t idx) -> std::optional<std::tuple<uint16_t, uint16_t, uint16_t>> {
		  idx *= 3;
		  std::optional<std::tuple<uint16_t, uint16_t, uint16_t>> tri {};
		  mesh.VisitIndices([idx, &tri](auto *indexData, uint32_t numIndices) {
			  if(idx + 3 > numIndices)
				  return;
			  tri = std::tuple<uint16_t, uint16_t, uint16_t> {indexData[idx], indexData[idx + 1], indexData[idx + 2]};
		  });
		  return tri;
	  });
	classDef.def("Copy", static_cast<std::shared_ptr<::ModelSubMesh> (*)(lua_State *, ::ModelSubMesh &, bool)>([](lua_State *l, ::ModelSubMesh &mesh, bool fullCopy) -> std::shared_ptr<::ModelSubMesh> { return mesh.Copy(fullCopy); }));
	classDef.def("Copy", static_cast<std::shared_ptr<::ModelSubMesh> (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) -> std::shared_ptr<::ModelSubMesh> { return mesh.Copy(); }));
	classDef.def("SetVertexTangent", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t, const Vector4 &)>([](lua_State *l, ::ModelSubMesh &mesh, uint32_t idx, const Vector4 &t) {
		if(idx >= mesh.GetVertexCount())
			return;
		auto &verts = mesh.GetVertices();
		verts.at(idx).tangent = t;
	}));
	classDef.def("Translate", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const Vector3 &)>([](lua_State *l, ::ModelSubMesh &mesh, const Vector3 &translation) { mesh.Translate(translation); }));
	classDef.def("Rotate", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const Quat &)>([](lua_State *l, ::ModelSubMesh &mesh, const Quat &rotation) { mesh.Rotate(rotation); }));
	classDef.def(
	  "MakeVerticesUnique", +[](lua_State *l, ::ModelSubMesh &mesh) {
		  auto &verts = mesh.GetVertices();
		  auto &vws = mesh.GetVertexWeights();
		  std::vector<umath::Vertex> newVerts;
		  std::vector<umath::VertexWeight> newVertWeights;
		  mesh.VisitIndices([&newVerts, &verts, &newVertWeights, &vws](auto *indexData, uint32_t numIndices) {
			  newVerts.resize(numIndices);
			  for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i)
				  newVerts[i] = verts[indexData[i]];

			  if(vws.empty())
				  return;
			  newVertWeights.resize(numIndices);
			  for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i)
				  newVertWeights[i] = vws[indexData[i]];
		  });
		  verts = std::move(newVerts);
		  vws = std::move(newVertWeights);
	  });
	classDef.def(
	  "SetVertices", +[](lua_State *l, ::ModelSubMesh &mesh, std::vector<umath::Vertex> verts) { mesh.GetVertices() = std::move(verts); });
	classDef.def(
	  "SetIndices", +[](lua_State *l, ::ModelSubMesh &mesh, const std::vector<uint32_t> &indices) {
		  mesh.SetIndices(indices);
		  uint32_t highestIndex = 0;
		  for(auto idx : indices)
			  highestIndex = umath::max(highestIndex, idx);
		  if(highestIndex <= std::numeric_limits<uint16_t>::max())
			  mesh.SetIndexType(pragma::model::IndexType::UInt16);
	  });
	classDef.def("GetReferenceId", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) { Lua::PushInt(l, mesh.GetReferenceId()); }));
	classDef.def("GetGeometryType", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) { Lua::PushInt(l, umath::to_integral(mesh.GetGeometryType())); }));
	classDef.def("SetGeometryType", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t)>([](lua_State *l, ::ModelSubMesh &mesh, uint32_t geometryType) { mesh.SetGeometryType(static_cast<::ModelSubMesh::GeometryType>(geometryType)); }));
	classDef.def("AddLine", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t, uint32_t)>([](lua_State *l, ::ModelSubMesh &mesh, uint32_t idx0, uint32_t idx1) { mesh.AddLine(idx0, idx1); }));
	classDef.def("AddPoint", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t)>([](lua_State *l, ::ModelSubMesh &mesh, uint32_t idx) { mesh.AddPoint(idx); }));
	classDef.def("GetPose", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) { Lua::Push<umath::ScaledTransform>(l, mesh.GetPose()); }));
	classDef.def("SetPose", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const umath::ScaledTransform &)>([](lua_State *l, ::ModelSubMesh &mesh, const umath::ScaledTransform &pose) { mesh.SetPose(pose); }));
	classDef.def("Transform", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const umath::ScaledTransform &)>([](lua_State *l, ::ModelSubMesh &mesh, const umath::ScaledTransform &pose) { mesh.Transform(pose); }));
	classDef.def("ClearVertices", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) { mesh.GetVertices().clear(); }));
	classDef.def("ClearIndices", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) { mesh.GetIndexData().clear(); }));
	classDef.def("ClearAlphas", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) { mesh.GetAlphas().clear(); }));
	classDef.def("ClearUVSets", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) { mesh.GetUVSets().clear(); }));
	classDef.def("ClearVertexWeights", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) { mesh.GetVertexWeights().clear(); }));
	classDef.def("ClearExtendedVertexWeights", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) { mesh.GetExtendedVertexWeights().clear(); }));
	classDef.def("ClearVertexData", static_cast<void (*)(lua_State *, ::ModelSubMesh &)>([](lua_State *l, ::ModelSubMesh &mesh) {
		mesh.GetIndexData().clear();
		mesh.GetVertices().clear();
		mesh.GetAlphas().clear();
		mesh.GetUVSets().clear();
		mesh.GetVertexWeights().clear();
		mesh.GetExtendedVertexWeights().clear();
	}));
	classDef.def("HasUVSet", static_cast<void (*)(lua_State *, ::ModelSubMesh &, const std::string &)>([](lua_State *l, ::ModelSubMesh &mesh, const std::string &uvSetName) {
		auto *uvSet = mesh.GetUVSet(uvSetName);
		Lua::PushBool(l, uvSet ? true : false);
	}));
	classDef.def("ReserveIndices", &::ModelSubMesh::ReserveIndices);
	classDef.def("ReserveVertices", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t)>([](lua_State *l, ::ModelSubMesh &mesh, uint32_t numVerts) { mesh.GetVertices().reserve(numVerts); }));
	classDef.def("ReserveTriangles", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t)>([](lua_State *l, ::ModelSubMesh &mesh, uint32_t numTris) { mesh.ReserveIndices(numTris * 3); }));
	classDef.def("ReserveVertexWeights", static_cast<void (*)(lua_State *, ::ModelSubMesh &, uint32_t)>([](lua_State *l, ::ModelSubMesh &mesh, uint32_t numVerts) { mesh.GetVertexWeights().reserve(numVerts); }));
	classDef.def(
	  "Simplify", +[](::ModelSubMesh &subMesh, uint32_t targetVertexCount, double aggressiveness) -> std::tuple<std::shared_ptr<::ModelSubMesh>, std::vector<uint64_t>> {
		  std::vector<uint64_t> vertexMapping;
		  auto simplifiedMesh = subMesh.Simplify(targetVertexCount, aggressiveness, &vertexMapping);
		  return std::tuple<std::shared_ptr<::ModelSubMesh>, std::vector<uint64_t>> {simplifiedMesh, std::move(vertexMapping)};
	  });
	classDef.def(
	  "Simplify", +[](::ModelSubMesh &subMesh, uint32_t targetVertexCount) -> std::tuple<std::shared_ptr<::ModelSubMesh>, std::vector<uint64_t>> {
		  std::vector<uint64_t> vertexMapping;
		  auto simplifiedMesh = subMesh.Simplify(targetVertexCount, 5.0, &vertexMapping);
		  return std::tuple<std::shared_ptr<::ModelSubMesh>, std::vector<uint64_t>> {simplifiedMesh, std::move(vertexMapping)};
	  });
	classDef.def(
	  "Save", +[](lua_State *l, ::ModelSubMesh &mesh, udm::AssetData &assetData) {
		  auto *nw = engine->GetNetworkState(l);
		  auto *game = nw ? nw->GetGameState() : nullptr;
		  if(game == nullptr)
			  return;
		  std::string err;
		  auto result = mesh.Save(assetData, err);
		  if(result == false)
			  Lua::PushString(l, err);
		  else
			  Lua::PushBool(l, result);
	  });
	classDef.scope[luabind::def(
	  "Load", +[](lua_State *l, ::Game &game, udm::AssetData &assetData) -> Lua::var<::ModelSubMesh, std::pair<bool, std::string>> {
		  std::string err;
		  auto mesh = ::ModelSubMesh::Load(game, assetData, err);
		  if(mesh)
			  return luabind::object {l, mesh};
		  return luabind::object {l, std::pair<bool, std::string> {false, err}};
	  })];
	classDef.add_static_constant("GEOMETRY_TYPE_TRIANGLES", umath::to_integral(::ModelSubMesh::GeometryType::Triangles));
	classDef.add_static_constant("GEOMETRY_TYPE_LINES", umath::to_integral(::ModelSubMesh::GeometryType::Lines));
	classDef.add_static_constant("GEOMETRY_TYPE_POINTS", umath::to_integral(::ModelSubMesh::GeometryType::Points));

	classDef.add_static_constant("INDEX_TYPE_UINT16", umath::to_integral(pragma::model::IndexType::UInt16));
	classDef.add_static_constant("INDEX_TYPE_UINT32", umath::to_integral(pragma::model::IndexType::UInt32));

	classDef.add_static_constant("MAX_INDEX16", ::ModelSubMesh::MAX_INDEX16);
	classDef.add_static_constant("MAX_INDEX32", ::ModelSubMesh::MAX_INDEX32);
}
void Lua::ModelSubMesh::GetSkinTextureIndex(lua_State *l, ::ModelSubMesh &mesh) { Lua::PushInt(l, mesh.GetSkinTextureIndex()); }
void Lua::ModelSubMesh::GetVertexCount(lua_State *l, ::ModelSubMesh &mdl) { Lua::PushInt(l, mdl.GetVertexCount()); }
void Lua::ModelSubMesh::SetVertexCount(lua_State *l, ::ModelSubMesh &mdl, uint32_t n)
{
	mdl.GetVertices().resize(n);
	for(auto &pair : mdl.GetUVSets())
		pair.second.resize(n);
}
void Lua::ModelSubMesh::SetIndexCount(lua_State *l, ::ModelSubMesh &mdl, uint32_t n) { mdl.SetIndexCount(n); }
void Lua::ModelSubMesh::GetTriangleVertexCount(lua_State *l, ::ModelSubMesh &mdl) { Lua::PushInt(l, mdl.GetIndexCount()); }
void Lua::ModelSubMesh::GetTriangleCount(lua_State *l, ::ModelSubMesh &mdl) { Lua::PushInt(l, mdl.GetTriangleCount()); }
void Lua::ModelSubMesh::AddUVSet(lua_State *l, ::ModelSubMesh &mdl, const std::string &uvSetName)
{
	auto &uvSet = mdl.AddUVSet(uvSetName);
	uvSet.resize(mdl.GetVertexCount());
}
void Lua::ModelSubMesh::GetVertices(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i = 0; i < verts.size(); i++) {
		Lua::Push<Vector3>(l, verts[i].position);
		lua_rawseti(l, top, i + 1);
	}
}
void Lua::ModelSubMesh::GetTriangles(lua_State *l, ::ModelSubMesh &mesh)
{
	lua_newtable(l);
	mesh.VisitIndices([l](auto *indexData, uint32_t numIndices) {
		int top = lua_gettop(l);
		for(int i = 0; i < numIndices; i++) {
			Lua::PushInt(l, indexData[i]);
			lua_rawseti(l, top, i + 1);
		}
	});
}
void Lua::ModelSubMesh::GetUVMapping(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i = 0; i < verts.size(); i++) {
		Lua::Push<Vector2>(l, verts[i].uv);
		lua_rawseti(l, top, i + 1);
	}
}
luabind::object Lua::ModelSubMesh::GetUVMapping(lua_State *l, ::ModelSubMesh &mesh, const std::string &uvSetName)
{
	auto *uvSet = mesh.GetUVSet(uvSetName);
	if(uvSet == nullptr)
		return {};
	auto t = luabind::newtable(l);
	for(auto i = decltype(uvSet->size()) {0u}; i < uvSet->size(); ++i)
		t[i + 1] = uvSet->at(i);
	return t;
}
luabind::object Lua::ModelSubMesh::GetUVSetNames(lua_State *l, ::ModelSubMesh &mesh)
{
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto &pair : mesh.GetUVSets())
		t[idx++] = pair.first;
	return t;
}
void Lua::ModelSubMesh::GetNormalMapping(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i = 0; i < verts.size(); i++) {
		Lua::Push<Vector3>(l, verts[i].normal);
		lua_rawseti(l, top, i + 1);
	}
}
void Lua::ModelSubMesh::GetVertexWeights(lua_State *l, ::ModelSubMesh &mesh)
{
	auto &vertWeights = mesh.GetVertexWeights();
	Lua::vector_to_table(l, vertWeights).push(l);
}
void Lua::ModelSubMesh::GetCenter(lua_State *l, ::ModelSubMesh &mdl) { Lua::Push<Vector3>(l, mdl.GetCenter()); }
void Lua::ModelSubMesh::AddTriangle(lua_State *, ::ModelSubMesh &mdl, const umath::Vertex &v1, const umath::Vertex &v2, const umath::Vertex &v3) { mdl.AddTriangle(v1, v2, v3); }
void Lua::ModelSubMesh::AddTriangle(lua_State *, ::ModelSubMesh &mdl, uint32_t a, uint32_t b, uint32_t c) { mdl.AddTriangle(a, b, c); }
void Lua::ModelSubMesh::SetSkinTextureIndex(lua_State *, ::ModelSubMesh &mdl, uint32_t texture) { mdl.SetSkinTextureIndex(texture); }
void Lua::ModelSubMesh::Update(lua_State *, ::ModelSubMesh &mdl) { mdl.Update(); }
void Lua::ModelSubMesh::Update(lua_State *, ::ModelSubMesh &mdl, uint32_t flags) { mdl.Update(static_cast<ModelUpdateFlags>(flags)); }
void Lua::ModelSubMesh::AddVertex(lua_State *l, ::ModelSubMesh &mdl, const umath::Vertex &v)
{
	auto idx = mdl.AddVertex(v);
	Lua::PushInt(l, idx);
}
void Lua::ModelSubMesh::GetBounds(lua_State *l, ::ModelSubMesh &mdl)
{
	Vector3 min, max;
	mdl.GetBounds(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}

void Lua::ModelSubMesh::SetVertex(lua_State *, ::ModelSubMesh &mdl, uint32_t idx, const umath::Vertex &v) { mdl.SetVertex(idx, v); }
void Lua::ModelSubMesh::SetVertexPosition(lua_State *, ::ModelSubMesh &mdl, uint32_t idx, const Vector3 &pos) { mdl.SetVertexPosition(idx, pos); }
void Lua::ModelSubMesh::SetVertexNormal(lua_State *, ::ModelSubMesh &mdl, uint32_t idx, const Vector3 &normal) { mdl.SetVertexNormal(idx, normal); }
void Lua::ModelSubMesh::SetVertexUV(lua_State *l, ::ModelSubMesh &mdl, const std::string &uvSetName, uint32_t idx, const ::Vector2 &uv)
{
	auto *uvSet = mdl.GetUVSet(uvSetName);
	if(uvSet == nullptr || idx >= uvSet->size())
		return;
	uvSet->at(idx) = uv;
}
void Lua::ModelSubMesh::SetVertexUV(lua_State *, ::ModelSubMesh &mdl, uint32_t idx, const Vector2 &uv) { mdl.SetVertexUV(idx, uv); }
void Lua::ModelSubMesh::SetVertexAlpha(lua_State *, ::ModelSubMesh &mdl, uint32_t idx, const Vector2 &alpha) { mdl.SetVertexAlpha(idx, alpha); }
void Lua::ModelSubMesh::SetVertexWeight(lua_State *, ::ModelSubMesh &mdl, uint32_t idx, const umath::VertexWeight &weight) { mdl.SetVertexWeight(idx, weight); }
void Lua::ModelSubMesh::GetVertex(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<umath::Vertex>(l, mdl.GetVertex(idx));
}
void Lua::ModelSubMesh::GetVertexPosition(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<Vector3>(l, mdl.GetVertexPosition(idx));
}
void Lua::ModelSubMesh::GetVertexNormal(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<Vector3>(l, mdl.GetVertexNormal(idx));
}
void Lua::ModelSubMesh::GetVertexUV(lua_State *l, ::ModelSubMesh &mdl, const std::string &uvSetName, uint32_t idx)
{
	auto *uvSet = mdl.GetUVSet(uvSetName);
	if(uvSet == nullptr || idx >= uvSet->size())
		return;
	Lua::Push<Vector2>(l, uvSet->at(idx));
}
void Lua::ModelSubMesh::GetVertexUV(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<Vector2>(l, mdl.GetVertexUV(idx));
}
void Lua::ModelSubMesh::GetVertexAlpha(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx)
{
	auto &alphas = mdl.GetAlphas();
	if(idx >= alphas.size())
		return;
	Lua::Push<Vector2>(l, alphas[idx]);
}
void Lua::ModelSubMesh::GetVertexWeight(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexWeights().size())
		return;
	Lua::Push<umath::VertexWeight>(l, mdl.GetVertexWeight(idx));
}
void Lua::ModelSubMesh::GenerateNormals(lua_State *, ::ModelSubMesh &mdl) { mdl.GenerateNormals(); }
void Lua::ModelSubMesh::NormalizeUVCoordinates(lua_State *l, ::ModelSubMesh &mdl) { mdl.NormalizeUVCoordinates(); }
void Lua::ModelSubMesh::ClipAgainstPlane(lua_State *l, ::ModelSubMesh &mdl, const Vector3 &n, double d) { ClipAgainstPlane(l, mdl, n, d, false); }
void Lua::ModelSubMesh::ClipAgainstPlane(lua_State *l, ::ModelSubMesh &mdl, const Vector3 &n, double d, bool bSplitCoverMeshes)
{
	auto clippedMeshA = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	auto clippedMeshB = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	std::shared_ptr<::ModelSubMesh> clippedCoverA = nullptr;
	std::shared_ptr<::ModelSubMesh> clippedCoverB = nullptr;
	if(bSplitCoverMeshes) {
		clippedCoverA = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
		clippedCoverB = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	}
	mdl.ClipAgainstPlane(n, d, *clippedMeshA, *clippedMeshB, nullptr, clippedCoverA.get(), clippedCoverB.get());
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, clippedMeshA);
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, clippedMeshB);
	if(bSplitCoverMeshes) {
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, clippedCoverA);
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, clippedCoverB);
	}
}
void Lua::ModelSubMesh::ClipAgainstPlane(lua_State *l, ::ModelSubMesh &mdl, const Vector3 &n, double d, bool bSplitCoverMeshes, luabind::object tBoneMatrices)
{
	const auto tMatrices = 5;
	Lua::CheckTable(l, tMatrices);
	std::vector<Mat4> boneMatrices {};
	auto numMatrices = Lua::GetObjectLength(l, tMatrices);
	for(auto i = decltype(numMatrices) {0u}; i < numMatrices; ++i) {
		Lua::PushInt(l, i + 1);
		Lua::GetTableValue(l, tMatrices);
		auto &m = Lua::Check<Mat4>(l, -1);
		boneMatrices.push_back(m);
		Lua::Pop(l, 1);
	}
	auto clippedMeshA = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	auto clippedMeshB = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	std::shared_ptr<::ModelSubMesh> clippedCoverA = nullptr;
	std::shared_ptr<::ModelSubMesh> clippedCoverB = nullptr;
	if(bSplitCoverMeshes) {
		clippedCoverA = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
		clippedCoverB = engine->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	}
	mdl.ClipAgainstPlane(n, d, *clippedMeshA, *clippedMeshB, &boneMatrices, clippedCoverA.get(), clippedCoverB.get());
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, clippedMeshA);
	Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, clippedMeshB);
	if(bSplitCoverMeshes) {
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, clippedCoverA);
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l, clippedCoverB);
	}
}
void Lua::ModelSubMesh::ApplyUVMapping(lua_State *l, ::ModelSubMesh &mdl, const Vector3 &nu, const Vector3 &nv, uint32_t w, uint32_t h, float ou, float ov, float su, float sv) { mdl.ApplyUVMapping(nu, nv, w, h, ou, ov, su, sv); }
void Lua::ModelSubMesh::ApplyUVMapping(lua_State *l, ::ModelSubMesh &mesh, ::Model &mdl, const Vector3 &nu, const Vector3 &nv, float ou, float ov, float su, float sv)
{
	auto matId = mdl.GetMaterialIndex(mesh);
	auto *mat = mdl.GetMaterial(matId.has_value() ? *matId : 0);
	auto w = 0u;
	auto h = 0u;
	if(mat != nullptr) {
		auto *texInfo = mat->GetDiffuseMap();
		if(texInfo != nullptr) {
			w = texInfo->width;
			h = texInfo->height;
		}
	}
	ApplyUVMapping(l, mesh, nu, nv, w, h, ou, ov, su, sv);
}
void Lua::ModelSubMesh::Scale(lua_State *l, ::ModelSubMesh &mesh, const Vector3 &scale) { mesh.Scale(scale); }

void Lua::ModelSubMesh::FlipTriangleWindingOrder(lua_State *l, ::ModelSubMesh &mesh)
{
	auto numIndices = mesh.GetIndexCount();
	if((numIndices % 3) != 0)
		return;
	mesh.VisitIndices([](auto *indexData, uint32_t numIndices) {
		for(auto i = decltype(numIndices) {0u}; i < numIndices; i += 3)
			umath::swap(indexData[i], indexData[i + 1]);
	});
}
