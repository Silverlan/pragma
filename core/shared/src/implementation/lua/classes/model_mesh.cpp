// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.shared;

import :scripting.lua.classes.model_mesh;

void Lua::ModelMesh::register_class(luabind::class_<pragma::geometry::ModelMesh> &classDef)
{
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("GetVertexCount", &GetVertexCount);
	classDef.def("GetIndexCount", &GetTriangleVertexCount);
	classDef.def("GetTriangleCount", &GetTriangleCount);
	classDef.def("GetSubMeshes", &GetSubMeshes);
	classDef.def("AddSubMesh", &AddSubMesh);
	classDef.def(
	  "AddSubMesh", +[](lua::State *l, pragma::geometry::ModelMesh &mesh, pragma::geometry::ModelSubMesh &subMesh, uint32_t pos) {
		  auto &subMeshes = mesh.GetSubMeshes();
		  subMeshes.insert(subMeshes.begin() + pos, subMesh.shared_from_this());
	  });
	classDef.def("Update", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &)>(&Update));
	classDef.def("Update", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &, uint32_t)>(&Update));
	classDef.def("GetBounds", &GetBounds);
	classDef.def("SetCenter", &SetCenter);
	classDef.def("GetCenter", &GetCenter);
	classDef.def("Centralize", &Centralize);
	classDef.def("Scale", &Scale);
	classDef.def("GetReferenceId", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &)>([](lua::State *l, pragma::geometry::ModelMesh &mesh) { PushInt(l, mesh.GetReferenceId()); }));
	classDef.def("Translate", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &, const Vector3 &)>([](lua::State *l, pragma::geometry::ModelMesh &mesh, const Vector3 &translation) { mesh.Translate(translation); }));
	classDef.def("Rotate", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &, const Quat &)>([](lua::State *l, pragma::geometry::ModelMesh &mesh, const Quat &rotation) { mesh.Rotate(rotation); }));
	classDef.def("ClearSubMeshes", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &)>([](lua::State *l, pragma::geometry::ModelMesh &mesh) { mesh.GetSubMeshes().clear(); }));
	classDef.def("RemoveSubMesh", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &, uint32_t)>([](lua::State *l, pragma::geometry::ModelMesh &mesh, uint32_t i) {
		auto &subMeshes = mesh.GetSubMeshes();
		if(i >= subMeshes.size())
			return;
		subMeshes.erase(subMeshes.begin() + i);
	}));
	classDef.def(
	  "RemoveSubMesh", +[](lua::State *l, pragma::geometry::ModelMesh &mesh, const std::string &uuid) {
		  auto &subMeshes = mesh.GetSubMeshes();
		  auto uuidValue = pragma::util::uuid_string_to_bytes(uuid);
		  auto it = std::find_if(subMeshes.begin(), subMeshes.end(), [&uuidValue](const std::shared_ptr<pragma::geometry::ModelSubMesh> &subMesh) { return subMesh->GetUuid() == uuidValue; });
		  if(it == subMeshes.end())
			  return;
		  subMeshes.erase(it);
	  });
	classDef.def(
	  "FindSubMesh", +[](lua::State *l, pragma::geometry::ModelMesh &mesh, const std::string &uuid) -> std::shared_ptr<pragma::geometry::ModelSubMesh> {
		  auto &subMeshes = mesh.GetSubMeshes();
		  auto uuidValue = pragma::util::uuid_string_to_bytes(uuid);
		  auto it = std::find_if(subMeshes.begin(), subMeshes.end(), [&uuidValue](const std::shared_ptr<pragma::geometry::ModelSubMesh> &subMesh) { return subMesh->GetUuid() == uuidValue; });
		  if(it == subMeshes.end())
			  return nullptr;
		  return *it;
	  });
	classDef.def("SetSubMeshes", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &, luabind::object)>([](lua::State *l, pragma::geometry::ModelMesh &mesh, luabind::object tSubMeshes) {
		auto idxSubMeshes = 2;
		CheckTable(l, idxSubMeshes);
		auto &subMeshes = mesh.GetSubMeshes();
		subMeshes = {};
		auto numSubMeshes = GetObjectLength(l, idxSubMeshes);
		subMeshes.reserve(numSubMeshes);
		for(auto i = decltype(numSubMeshes) {0u}; i < numSubMeshes; ++i) {
			PushInt(l, i + 1);
			GetTableValue(l, idxSubMeshes);
			auto &subMesh = Lua::Check<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, -1);
			subMeshes.push_back(subMesh);
			Pop(l, 1);
		}
	}));
	classDef.def("GetSubMeshCount", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &)>([](lua::State *l, pragma::geometry::ModelMesh &mesh) { PushInt(l, mesh.GetSubMeshes().size()); }));
	classDef.def("GetSubMesh", static_cast<void (*)(lua::State *, pragma::geometry::ModelMesh &, uint32_t)>([](lua::State *l, pragma::geometry::ModelMesh &mesh, uint32_t index) {
		auto &subMeshes = mesh.GetSubMeshes();
		if(index >= subMeshes.size())
			return;
		Push(l, subMeshes.at(index));
	}));
}
void Lua::ModelMesh::GetVertexCount(lua::State *l, pragma::geometry::ModelMesh &mesh) { PushInt(l, mesh.GetVertexCount()); }
void Lua::ModelMesh::GetTriangleVertexCount(lua::State *l, pragma::geometry::ModelMesh &mesh) { PushInt(l, mesh.GetIndexCount()); }
void Lua::ModelMesh::GetTriangleCount(lua::State *l, pragma::geometry::ModelMesh &mdl) { PushInt(l, mdl.GetTriangleCount()); }
void Lua::ModelMesh::GetSubMeshes(lua::State *l, pragma::geometry::ModelMesh &mdl)
{
	auto &subMeshes = mdl.GetSubMeshes();
	CreateTable(l);
	auto top = GetStackTop(l);
	UInt i = 0;
	for(auto it = subMeshes.begin(); it != subMeshes.end(); ++it) {
		auto &subMesh = *it;
		Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, subMesh);
		SetTableValue(l, top, CInt32(i + 1));
		i++;
	}
}
void Lua::ModelMesh::AddSubMesh(lua::State *, pragma::geometry::ModelMesh &mdl, pragma::geometry::ModelSubMesh &mesh) { mdl.AddSubMesh(mesh.shared_from_this()); }
void Lua::ModelMesh::Update(lua::State *, pragma::geometry::ModelMesh &mdl) { mdl.Update(); }
void Lua::ModelMesh::Update(lua::State *, pragma::geometry::ModelMesh &mdl, uint32_t flags) { mdl.Update(static_cast<pragma::asset::ModelUpdateFlags>(flags)); }
void Lua::ModelMesh::GetBounds(lua::State *l, pragma::geometry::ModelMesh &mdl)
{
	Vector3 min, max;
	mdl.GetBounds(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::ModelMesh::SetCenter(lua::State *, pragma::geometry::ModelMesh &mdl, const Vector3 &center) { mdl.SetCenter(center); }
void Lua::ModelMesh::GetCenter(lua::State *l, pragma::geometry::ModelMesh &mdl) { Lua::Push<Vector3>(l, mdl.GetCenter()); }
void Lua::ModelMesh::Centralize(lua::State *, pragma::geometry::ModelMesh &mdl) { mdl.Centralize(); }
void Lua::ModelMesh::Scale(lua::State *l, pragma::geometry::ModelMesh &mdl, const Vector3 &scale) { mdl.Scale(scale); }

////////////////////////////////////////

void Lua::ModelSubMesh::register_class(luabind::class_<pragma::geometry::ModelSubMesh> &classDef)
{
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def(luabind::tostring(luabind::self));
	classDef.scope[luabind::def("create_quad", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(pragma::Game &, const pragma::geometry::QuadCreateInfo &)>(&pragma::geometry::create_quad))];
	classDef.scope[luabind::def("create_box", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(pragma::Game &, const pragma::geometry::BoxCreateInfo &)>(&pragma::geometry::create_box))];
	classDef.scope[luabind::def("create_sphere", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(pragma::Game &, const pragma::geometry::SphereCreateInfo &)>(&pragma::geometry::create_sphere))];
	classDef.scope[luabind::def("create_cylinder", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(pragma::Game &, const pragma::geometry::CylinderCreateInfo &)>(&pragma::geometry::create_cylinder))];
	classDef.scope[luabind::def("create_cone", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(pragma::Game &, const pragma::geometry::ConeCreateInfo &)>(&pragma::geometry::create_cone))];
	classDef.scope[luabind::def("create_elliptic_cone", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(pragma::Game &, const pragma::geometry::EllipticConeCreateInfo &)>(&pragma::geometry::create_elliptic_cone))];
	classDef.scope[luabind::def("create_circle", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(pragma::Game &, const pragma::geometry::CircleCreateInfo &)>(&pragma::geometry::create_circle))];
	classDef.scope[luabind::def("create_ring", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(pragma::Game &, const pragma::geometry::RingCreateInfo &)>(&pragma::geometry::create_ring))];
	classDef.def("SetName", &pragma::geometry::ModelSubMesh::SetName);
	classDef.def("GetName", &pragma::geometry::ModelSubMesh::GetName);
	classDef.def("GetUuid", +[](pragma::geometry::ModelSubMesh &mesh) -> std::string { return pragma::util::uuid_to_string(mesh.GetUuid()); });
	classDef.def("SetUuid", +[](pragma::geometry::ModelSubMesh &mesh, const std::string &uuid) { mesh.SetUuid(pragma::util::uuid_string_to_bytes(uuid)); });
	classDef.def("SetUuid", +[](pragma::geometry::ModelSubMesh &mesh, const util::Uuid &uuid) { mesh.SetUuid(uuid.value); }, luabind::const_ref_policy<2> {});
	classDef.def("GetSkinTextureIndex", &GetSkinTextureIndex);
	classDef.def("FlipTriangleWindingOrder", &FlipTriangleWindingOrder);
	classDef.def("GetVertexCount", &GetVertexCount);
	classDef.def("GetAlphaCount", &pragma::geometry::ModelSubMesh::GetAlphaCount);
	classDef.def("SetAlphaCount", &pragma::geometry::ModelSubMesh::SetAlphaCount);
	classDef.def("SetVertexCount", &SetVertexCount);
	classDef.def("SetIndexCount", &SetIndexCount);
	classDef.def("GetIndexCount", &GetTriangleVertexCount);
	classDef.def("GetTriangleCount", &GetTriangleCount);
	classDef.def("GetIndexType", &pragma::geometry::ModelSubMesh::GetIndexType);
	classDef.def("SetIndexType", &pragma::geometry::ModelSubMesh::SetIndexType);
	classDef.def("GetVertices", &GetVertices);
	classDef.def("GetIndices", &GetTriangles);
	classDef.def("AddUVSet", &AddUVSet);
	classDef.def("GetUVs", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>(&GetUVMapping));
	classDef.def("GetUVs", static_cast<luabind::object (*)(lua::State *, pragma::geometry::ModelSubMesh &, const std::string &)>(&GetUVMapping));
	classDef.def("GetUVSetNames", &GetUVSetNames);
	classDef.def("HasUvSet", +[](lua::State *l, pragma::geometry::ModelSubMesh &mesh, const std::string &name) { return mesh.GetUVSet(name) != nullptr; });
	classDef.def("HasVertexWeights", +[](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { return !mesh.GetVertexWeights().empty(); });
	classDef.def("GetNormals", &GetNormalMapping);
	classDef.def("GetVertexWeights", &GetVertexWeights);
	classDef.def("AddTriangle", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const pragma::math::Vertex &, const pragma::math::Vertex &, const pragma::math::Vertex &)>(&AddTriangle));
	classDef.def("AddTriangle", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t, uint32_t, uint32_t)>(&AddTriangle));
	classDef.def("AddIndex", &pragma::geometry::ModelSubMesh::AddIndex);
	classDef.def("SetIndex", &pragma::geometry::ModelSubMesh::SetIndex);
	classDef.def("GetIndex", &pragma::geometry::ModelSubMesh::GetIndex);
	classDef.def("SetSkinTextureIndex", &SetSkinTextureIndex);
	classDef.def("Update", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t)>(&Update));
	classDef.def("Update", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>(&Update));
	classDef.def("AddVertex", &AddVertex);
	classDef.def("GetBounds", &GetBounds);
	classDef.def("GetCenter", &GetCenter);
	classDef.def("SetCenter", &pragma::geometry::ModelSubMesh::SetCenter);
	classDef.def("SetVertex", &SetVertex);
	classDef.def("SetVertexPosition", &SetVertexPosition);
	classDef.def("SetVertexNormal", &SetVertexNormal);
	classDef.def("SetVertexUV", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const std::string &, uint32_t, const ::Vector2 &)>(&SetVertexUV));
	classDef.def("SetVertexUV", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t, const ::Vector2 &)>(&SetVertexUV));
	classDef.def("SetVertexAlpha", &SetVertexAlpha);
	classDef.def("SetVertexWeight", &SetVertexWeight);
	classDef.def("GetVertex", &GetVertex);
	classDef.def("GetVertexPosition", &GetVertexPosition);
	classDef.def("GetVertexNormal", &GetVertexNormal);
	classDef.def("GetVertexUV", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const std::string &, uint32_t)>(&GetVertexUV));
	classDef.def("GetVertexUV", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t)>(&GetVertexUV));
	classDef.def("GetVertexAlpha", &GetVertexAlpha);
	classDef.def("GetVertexWeight", &GetVertexWeight);
	classDef.def("Optimize", &pragma::geometry::ModelSubMesh::Optimize);
#ifdef _WIN32
	classDef.def("Optimize", &pragma::geometry::ModelSubMesh::Optimize, luabind::default_parameter_policy<2, double {pragma::math::VERTEX_EPSILON}> {});
#else
	classDef.def("Optimize", +[](pragma::geometry::ModelSubMesh &mesh) { return mesh.Optimize(); });
#endif
	classDef.def("GenerateNormals", &GenerateNormals);
	classDef.def("NormalizeUVCoordinates", &NormalizeUVCoordinates);
	classDef.def("ClipAgainstPlane", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const Vector3 &, double, bool, luabind::object)>(&ClipAgainstPlane));
	classDef.def("ClipAgainstPlane", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const Vector3 &, double, bool)>(&ClipAgainstPlane));
	classDef.def("ClipAgainstPlane", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const Vector3 &, double)>(&ClipAgainstPlane));
	classDef.def("ApplyUVMapping", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, pragma::asset::Model &, const Vector3 &, const Vector3 &, float, float, float, float)>(&ApplyUVMapping));
	classDef.def("ApplyUVMapping", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const Vector3 &, const Vector3 &, uint32_t, uint32_t, float, float, float, float)>(&ApplyUVMapping));
	classDef.def("Scale", &Scale);
	classDef.def(
	  "GetTriangle", +[](lua::State *l, pragma::geometry::ModelSubMesh &mesh, uint32_t idx) -> std::optional<std::tuple<uint16_t, uint16_t, uint16_t>> {
		  idx *= 3;
		  std::optional<std::tuple<uint16_t, uint16_t, uint16_t>> tri {};
		  mesh.VisitIndices([idx, &tri](auto *indexData, uint32_t numIndices) {
			  if(idx + 3 > numIndices)
				  return;
			  tri = std::tuple<uint16_t, uint16_t, uint16_t> {indexData[idx], indexData[idx + 1], indexData[idx + 2]};
		  });
		  return tri;
	  });
	classDef.def("Copy", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(lua::State *, pragma::geometry::ModelSubMesh &, bool)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, bool fullCopy) -> std::shared_ptr<pragma::geometry::ModelSubMesh> { return mesh.Copy(fullCopy); }));
	classDef.def("Copy", static_cast<std::shared_ptr<pragma::geometry::ModelSubMesh> (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) -> std::shared_ptr<pragma::geometry::ModelSubMesh> { return mesh.Copy(); }));
	classDef.def("SetVertexTangent", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t, const ::Vector4 &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, uint32_t idx, const ::Vector4 &t) {
		if(idx >= mesh.GetVertexCount())
			return;
		auto &verts = mesh.GetVertices();
		verts.at(idx).tangent = t;
	}));
	classDef.def("Translate", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const Vector3 &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, const Vector3 &translation) { mesh.Translate(translation); }));
	classDef.def("Rotate", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const Quat &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, const Quat &rotation) { mesh.Rotate(rotation); }));
	classDef.def(
	  "MakeVerticesUnique", +[](lua::State *l, pragma::geometry::ModelSubMesh &mesh) {
		  auto &verts = mesh.GetVertices();
		  auto &vws = mesh.GetVertexWeights();
		  std::vector<pragma::math::Vertex> newVerts;
		  std::vector<pragma::math::VertexWeight> newVertWeights;
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
	classDef.def("SetVertices", +[](lua::State *l, pragma::geometry::ModelSubMesh &mesh, std::vector<pragma::math::Vertex> verts) { mesh.GetVertices() = std::move(verts); });
	classDef.def(
	  "SetIndices", +[](lua::State *l, pragma::geometry::ModelSubMesh &mesh, const std::vector<uint32_t> &indices) {
		  mesh.SetIndices(indices);
		  uint32_t highestIndex = 0;
		  for(auto idx : indices)
			  highestIndex = pragma::math::max(highestIndex, idx);
		  if(highestIndex <= std::numeric_limits<uint16_t>::max())
			  mesh.SetIndexType(pragma::geometry::IndexType::UInt16);
	  });
	classDef.def("GetReferenceId", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { PushInt(l, mesh.GetReferenceId()); }));
	classDef.def("GetGeometryType", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { PushInt(l, pragma::math::to_integral(mesh.GetGeometryType())); }));
	classDef.def("SetGeometryType", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, uint32_t geometryType) { mesh.SetGeometryType(static_cast<pragma::geometry::ModelSubMesh::GeometryType>(geometryType)); }));
	classDef.def("AddLine", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t, uint32_t)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, uint32_t idx0, uint32_t idx1) { mesh.AddLine(idx0, idx1); }));
	classDef.def("AddPoint", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, uint32_t idx) { mesh.AddPoint(idx); }));
	classDef.def("GetPose", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { Lua::Push<pragma::math::ScaledTransform>(l, mesh.GetPose()); }));
	classDef.def("SetPose", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const pragma::math::ScaledTransform &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, const pragma::math::ScaledTransform &pose) { mesh.SetPose(pose); }));
	classDef.def("Transform", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const pragma::math::ScaledTransform &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, const pragma::math::ScaledTransform &pose) { mesh.Transform(pose); }));
	classDef.def("ClearVertices", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { mesh.GetVertices().clear(); }));
	classDef.def("ClearIndices", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { mesh.GetIndexData().clear(); }));
	classDef.def("ClearAlphas", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { mesh.GetAlphas().clear(); }));
	classDef.def("ClearUVSets", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { mesh.GetUVSets().clear(); }));
	classDef.def("ClearVertexWeights", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { mesh.GetVertexWeights().clear(); }));
	classDef.def("ClearExtendedVertexWeights", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) { mesh.GetExtendedVertexWeights().clear(); }));
	classDef.def("ClearVertexData", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh) {
		mesh.GetIndexData().clear();
		mesh.GetVertices().clear();
		mesh.GetAlphas().clear();
		mesh.GetUVSets().clear();
		mesh.GetVertexWeights().clear();
		mesh.GetExtendedVertexWeights().clear();
	}));
	classDef.def("HasUVSet", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, const std::string &)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, const std::string &uvSetName) {
		auto *uvSet = mesh.GetUVSet(uvSetName);
		PushBool(l, uvSet ? true : false);
	}));
	classDef.def("ReserveIndices", &pragma::geometry::ModelSubMesh::ReserveIndices);
	classDef.def("ReserveVertices", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, uint32_t numVerts) { mesh.GetVertices().reserve(numVerts); }));
	classDef.def("ReserveTriangles", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, uint32_t numTris) { mesh.ReserveIndices(numTris * 3); }));
	classDef.def("ReserveVertexWeights", static_cast<void (*)(lua::State *, pragma::geometry::ModelSubMesh &, uint32_t)>([](lua::State *l, pragma::geometry::ModelSubMesh &mesh, uint32_t numVerts) { mesh.GetVertexWeights().reserve(numVerts); }));
	classDef.def(
	  "Simplify", +[](pragma::geometry::ModelSubMesh &subMesh, uint32_t targetVertexCount, double aggressiveness) -> std::tuple<std::shared_ptr<pragma::geometry::ModelSubMesh>, std::vector<uint64_t>> {
		  std::vector<uint64_t> vertexMapping;
		  auto simplifiedMesh = subMesh.Simplify(targetVertexCount, aggressiveness, &vertexMapping);
		  return std::tuple<std::shared_ptr<pragma::geometry::ModelSubMesh>, std::vector<uint64_t>> {simplifiedMesh, std::move(vertexMapping)};
	  });
	classDef.def(
	  "Simplify", +[](pragma::geometry::ModelSubMesh &subMesh, uint32_t targetVertexCount) -> std::tuple<std::shared_ptr<pragma::geometry::ModelSubMesh>, std::vector<uint64_t>> {
		  std::vector<uint64_t> vertexMapping;
		  auto simplifiedMesh = subMesh.Simplify(targetVertexCount, 5.0, &vertexMapping);
		  return std::tuple<std::shared_ptr<pragma::geometry::ModelSubMesh>, std::vector<uint64_t>> {simplifiedMesh, std::move(vertexMapping)};
	  });
	classDef.def(
	  "Save", +[](lua::State *l, pragma::geometry::ModelSubMesh &mesh, ::udm::AssetData &assetData) {
		  auto *nw = pragma::Engine::Get()->GetNetworkState(l);
		  auto *game = nw ? nw->GetGameState() : nullptr;
		  if(game == nullptr)
			  return;
		  std::string err;
		  auto result = mesh.Save(assetData, err);
		  if(result == false)
			  PushString(l, err);
		  else
			  PushBool(l, result);
	  });
	classDef.scope[luabind::def(
	  "Load", +[](lua::State *l, pragma::Game &game, ::udm::AssetData &assetData) -> var<pragma::geometry::ModelSubMesh, std::pair<bool, std::string>> {
		  std::string err;
		  auto mesh = pragma::geometry::ModelSubMesh::Load(game, assetData, err);
		  if(mesh)
			  return luabind::object {l, mesh};
		  return luabind::object {l, std::pair<bool, std::string> {false, err}};
	  })];
	classDef.add_static_constant("GEOMETRY_TYPE_TRIANGLES", pragma::math::to_integral(pragma::geometry::ModelSubMesh::GeometryType::Triangles));
	classDef.add_static_constant("GEOMETRY_TYPE_LINES", pragma::math::to_integral(pragma::geometry::ModelSubMesh::GeometryType::Lines));
	classDef.add_static_constant("GEOMETRY_TYPE_POINTS", pragma::math::to_integral(pragma::geometry::ModelSubMesh::GeometryType::Points));

	classDef.add_static_constant("INDEX_TYPE_UINT16", pragma::math::to_integral(pragma::geometry::IndexType::UInt16));
	classDef.add_static_constant("INDEX_TYPE_UINT32", pragma::math::to_integral(pragma::geometry::IndexType::UInt32));

	classDef.add_static_constant("MAX_INDEX16", pragma::geometry::ModelSubMesh::MAX_INDEX16);
	classDef.add_static_constant("MAX_INDEX32", pragma::geometry::ModelSubMesh::MAX_INDEX32);
}
void Lua::ModelSubMesh::GetSkinTextureIndex(lua::State *l, pragma::geometry::ModelSubMesh &mesh) { PushInt(l, mesh.GetSkinTextureIndex()); }
void Lua::ModelSubMesh::GetVertexCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl) { PushInt(l, mdl.GetVertexCount()); }
void Lua::ModelSubMesh::SetVertexCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t n)
{
	mdl.GetVertices().resize(n);
	for(auto &pair : mdl.GetUVSets())
		pair.second.resize(n);
}
void Lua::ModelSubMesh::SetIndexCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t n) { mdl.SetIndexCount(n); }
void Lua::ModelSubMesh::GetTriangleVertexCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl) { PushInt(l, mdl.GetIndexCount()); }
void Lua::ModelSubMesh::GetTriangleCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl) { PushInt(l, mdl.GetTriangleCount()); }
void Lua::ModelSubMesh::AddUVSet(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const std::string &uvSetName)
{
	auto &uvSet = mdl.AddUVSet(uvSetName);
	uvSet.resize(mdl.GetVertexCount());
}
void Lua::ModelSubMesh::GetVertices(lua::State *l, pragma::geometry::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();
	CreateTable(l);
	int top = GetStackTop(l);
	for(int i = 0; i < verts.size(); i++) {
		Lua::Push<Vector3>(l, verts[i].position);
		SetTableValue(l, top, i + 1);
	}
}
void Lua::ModelSubMesh::GetTriangles(lua::State *l, pragma::geometry::ModelSubMesh &mesh)
{
	CreateTable(l);
	mesh.VisitIndices([l](auto *indexData, uint32_t numIndices) {
		int top = GetStackTop(l);
		for(int i = 0; i < numIndices; i++) {
			Lua::PushInt(l, indexData[i]);
			SetTableValue(l, top, i + 1);
		}
	});
}
void Lua::ModelSubMesh::GetUVMapping(lua::State *l, pragma::geometry::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();
	CreateTable(l);
	int top = GetStackTop(l);
	for(int i = 0; i < verts.size(); i++) {
		Lua::Push<::Vector2>(l, verts[i].uv);
		SetTableValue(l, top, i + 1);
	}
}
luabind::object Lua::ModelSubMesh::GetUVMapping(lua::State *l, pragma::geometry::ModelSubMesh &mesh, const std::string &uvSetName)
{
	auto *uvSet = mesh.GetUVSet(uvSetName);
	if(uvSet == nullptr)
		return {};
	auto t = luabind::newtable(l);
	for(auto i = decltype(uvSet->size()) {0u}; i < uvSet->size(); ++i)
		t[i + 1] = uvSet->at(i);
	return t;
}
luabind::object Lua::ModelSubMesh::GetUVSetNames(lua::State *l, pragma::geometry::ModelSubMesh &mesh)
{
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto &pair : mesh.GetUVSets())
		t[idx++] = pair.first;
	return t;
}
void Lua::ModelSubMesh::GetNormalMapping(lua::State *l, pragma::geometry::ModelSubMesh &mesh)
{
	auto &verts = mesh.GetVertices();
	CreateTable(l);
	int top = GetStackTop(l);
	for(int i = 0; i < verts.size(); i++) {
		Lua::Push<Vector3>(l, verts[i].normal);
		SetTableValue(l, top, i + 1);
	}
}
void Lua::ModelSubMesh::GetVertexWeights(lua::State *l, pragma::geometry::ModelSubMesh &mesh)
{
	auto &vertWeights = mesh.GetVertexWeights();
	vector_to_table(l, vertWeights).push(l);
}
void Lua::ModelSubMesh::GetCenter(lua::State *l, pragma::geometry::ModelSubMesh &mdl) { Lua::Push<Vector3>(l, mdl.GetCenter()); }
void Lua::ModelSubMesh::AddTriangle(lua::State *, pragma::geometry::ModelSubMesh &mdl, const pragma::math::Vertex &v1, const pragma::math::Vertex &v2, const pragma::math::Vertex &v3) { mdl.AddTriangle(v1, v2, v3); }
void Lua::ModelSubMesh::AddTriangle(lua::State *, pragma::geometry::ModelSubMesh &mdl, uint32_t a, uint32_t b, uint32_t c) { mdl.AddTriangle(a, b, c); }
void Lua::ModelSubMesh::SetSkinTextureIndex(lua::State *, pragma::geometry::ModelSubMesh &mdl, uint32_t texture) { mdl.SetSkinTextureIndex(texture); }
void Lua::ModelSubMesh::Update(lua::State *, pragma::geometry::ModelSubMesh &mdl) { mdl.Update(); }
void Lua::ModelSubMesh::Update(lua::State *, pragma::geometry::ModelSubMesh &mdl, uint32_t flags) { mdl.Update(static_cast<pragma::asset::ModelUpdateFlags>(flags)); }
void Lua::ModelSubMesh::AddVertex(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const pragma::math::Vertex &v)
{
	auto idx = mdl.AddVertex(v);
	PushInt(l, idx);
}
void Lua::ModelSubMesh::GetBounds(lua::State *l, pragma::geometry::ModelSubMesh &mdl)
{
	Vector3 min, max;
	mdl.GetBounds(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}

void Lua::ModelSubMesh::SetVertex(lua::State *, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const pragma::math::Vertex &v) { mdl.SetVertex(idx, v); }
void Lua::ModelSubMesh::SetVertexPosition(lua::State *, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const Vector3 &pos) { mdl.SetVertexPosition(idx, pos); }
void Lua::ModelSubMesh::SetVertexNormal(lua::State *, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const Vector3 &normal) { mdl.SetVertexNormal(idx, normal); }
void Lua::ModelSubMesh::SetVertexUV(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const std::string &uvSetName, uint32_t idx, const ::Vector2 &uv)
{
	auto *uvSet = mdl.GetUVSet(uvSetName);
	if(uvSet == nullptr || idx >= uvSet->size())
		return;
	uvSet->at(idx) = uv;
}
void Lua::ModelSubMesh::SetVertexUV(lua::State *, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const ::Vector2 &uv) { mdl.SetVertexUV(idx, uv); }
void Lua::ModelSubMesh::SetVertexAlpha(lua::State *, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const ::Vector2 &alpha) { mdl.SetVertexAlpha(idx, alpha); }
void Lua::ModelSubMesh::SetVertexWeight(lua::State *, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const pragma::math::VertexWeight &weight) { mdl.SetVertexWeight(idx, weight); }
void Lua::ModelSubMesh::GetVertex(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<pragma::math::Vertex>(l, mdl.GetVertex(idx));
}
void Lua::ModelSubMesh::GetVertexPosition(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<Vector3>(l, mdl.GetVertexPosition(idx));
}
void Lua::ModelSubMesh::GetVertexNormal(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<Vector3>(l, mdl.GetVertexNormal(idx));
}
void Lua::ModelSubMesh::GetVertexUV(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const std::string &uvSetName, uint32_t idx)
{
	auto *uvSet = mdl.GetUVSet(uvSetName);
	if(uvSet == nullptr || idx >= uvSet->size())
		return;
	Lua::Push<::Vector2>(l, uvSet->at(idx));
}
void Lua::ModelSubMesh::GetVertexUV(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexCount())
		return;
	Lua::Push<::Vector2>(l, mdl.GetVertexUV(idx));
}
void Lua::ModelSubMesh::GetVertexAlpha(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx)
{
	auto &alphas = mdl.GetAlphas();
	if(idx >= alphas.size())
		return;
	Lua::Push<::Vector2>(l, alphas[idx]);
}
void Lua::ModelSubMesh::GetVertexWeight(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx)
{
	if(idx >= mdl.GetVertexWeights().size())
		return;
	Lua::Push<pragma::math::VertexWeight>(l, mdl.GetVertexWeight(idx));
}
void Lua::ModelSubMesh::GenerateNormals(lua::State *, pragma::geometry::ModelSubMesh &mdl) { mdl.GenerateNormals(); }
void Lua::ModelSubMesh::NormalizeUVCoordinates(lua::State *l, pragma::geometry::ModelSubMesh &mdl) { mdl.NormalizeUVCoordinates(); }
void Lua::ModelSubMesh::ClipAgainstPlane(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const Vector3 &n, double d) { ClipAgainstPlane(l, mdl, n, d, false); }
void Lua::ModelSubMesh::ClipAgainstPlane(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const Vector3 &n, double d, bool bSplitCoverMeshes)
{
	auto clippedMeshA = pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	auto clippedMeshB = pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	std::shared_ptr<pragma::geometry::ModelSubMesh> clippedCoverA = nullptr;
	std::shared_ptr<pragma::geometry::ModelSubMesh> clippedCoverB = nullptr;
	if(bSplitCoverMeshes) {
		clippedCoverA = pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
		clippedCoverB = pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	}
	mdl.ClipAgainstPlane(n, d, *clippedMeshA, *clippedMeshB, nullptr, clippedCoverA.get(), clippedCoverB.get());
	Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, clippedMeshA);
	Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, clippedMeshB);
	if(bSplitCoverMeshes) {
		Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, clippedCoverA);
		Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, clippedCoverB);
	}
}
void Lua::ModelSubMesh::ClipAgainstPlane(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const Vector3 &n, double d, bool bSplitCoverMeshes, luabind::object tBoneMatrices)
{
	const auto tMatrices = 5;
	CheckTable(l, tMatrices);
	std::vector<::Mat4> boneMatrices {};
	auto numMatrices = GetObjectLength(l, tMatrices);
	for(auto i = decltype(numMatrices) {0u}; i < numMatrices; ++i) {
		PushInt(l, i + 1);
		GetTableValue(l, tMatrices);
		auto &m = Lua::Check<::Mat4>(l, -1);
		boneMatrices.push_back(m);
		Pop(l, 1);
	}
	auto clippedMeshA = pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	auto clippedMeshB = pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	std::shared_ptr<pragma::geometry::ModelSubMesh> clippedCoverA = nullptr;
	std::shared_ptr<pragma::geometry::ModelSubMesh> clippedCoverB = nullptr;
	if(bSplitCoverMeshes) {
		clippedCoverA = pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
		clippedCoverB = pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->CreateModelSubMesh();
	}
	mdl.ClipAgainstPlane(n, d, *clippedMeshA, *clippedMeshB, &boneMatrices, clippedCoverA.get(), clippedCoverB.get());
	Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, clippedMeshA);
	Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, clippedMeshB);
	if(bSplitCoverMeshes) {
		Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, clippedCoverA);
		Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, clippedCoverB);
	}
}
void Lua::ModelSubMesh::ApplyUVMapping(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const Vector3 &nu, const Vector3 &nv, uint32_t w, uint32_t h, float ou, float ov, float su, float sv) { mdl.ApplyUVMapping(nu, nv, w, h, ou, ov, su, sv); }
void Lua::ModelSubMesh::ApplyUVMapping(lua::State *l, pragma::geometry::ModelSubMesh &mesh, pragma::asset::Model &mdl, const Vector3 &nu, const Vector3 &nv, float ou, float ov, float su, float sv)
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
void Lua::ModelSubMesh::Scale(lua::State *l, pragma::geometry::ModelSubMesh &mesh, const Vector3 &scale) { mesh.Scale(scale); }

void Lua::ModelSubMesh::FlipTriangleWindingOrder(lua::State *l, pragma::geometry::ModelSubMesh &mesh)
{
	auto numIndices = mesh.GetIndexCount();
	if((numIndices % 3) != 0)
		return;
	mesh.VisitIndices([](auto *indexData, uint32_t numIndices) {
		for(auto i = decltype(numIndices) {0u}; i < numIndices; i += 3)
			pragma::math::swap(indexData[i], indexData[i + 1]);
	});
}
