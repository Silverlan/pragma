// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.model_mesh;

export import :model.model_mesh;
export import pragma.lua;

export namespace pragma::asset {
	class Model;
}
export namespace Lua {
	namespace ModelMesh {
		DLLNETWORK void register_class(luabind::class_<pragma::geometry::ModelMesh> &classDef);
		DLLNETWORK void GetVertexCount(lua::State *l, pragma::geometry::ModelMesh &mdl);
		DLLNETWORK void GetTriangleVertexCount(lua::State *l, pragma::geometry::ModelMesh &mdl);
		DLLNETWORK void GetTriangleCount(lua::State *l, pragma::geometry::ModelMesh &mdl);
		DLLNETWORK void GetSubMeshes(lua::State *l, pragma::geometry::ModelMesh &mdl);
		DLLNETWORK void AddSubMesh(lua::State *l, pragma::geometry::ModelMesh &mdl, pragma::geometry::ModelSubMesh &mesh);
		DLLNETWORK void Update(lua::State *l, pragma::geometry::ModelMesh &mdl);
		DLLNETWORK void Update(lua::State *l, pragma::geometry::ModelMesh &mdl, uint32_t flags);
		DLLNETWORK void GetBounds(lua::State *l, pragma::geometry::ModelMesh &mdl);
		DLLNETWORK void GetCenter(lua::State *l, pragma::geometry::ModelMesh &mdl);
		DLLNETWORK void SetCenter(lua::State *l, pragma::geometry::ModelMesh &mdl, const Vector3 &center);
		DLLNETWORK void Centralize(lua::State *l, pragma::geometry::ModelMesh &mdl);
		DLLNETWORK void Scale(lua::State *l, pragma::geometry::ModelMesh &mdl, const Vector3 &scale);
	};
	namespace ModelSubMesh {
		DLLNETWORK void FlipTriangleWindingOrder(lua::State *l, pragma::geometry::ModelSubMesh &mesh);

		DLLNETWORK void register_class(luabind::class_<pragma::geometry::ModelSubMesh> &classDef);
		DLLNETWORK void GetVertexCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void SetVertexCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t n);
		DLLNETWORK void SetIndexCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t n);
		DLLNETWORK void GetTriangleVertexCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void GetTriangleCount(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void GetSkinTextureIndex(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void GetVertices(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void GetTriangles(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void AddUVSet(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const std::string &uvSetName);
		DLLNETWORK void GetUVMapping(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK luabind::object GetUVMapping(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const std::string &uvSet);
		DLLNETWORK luabind::object GetUVSetNames(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void GetNormalMapping(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void GetVertexWeights(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void AddTriangle(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const pragma::math::Vertex &v1, const pragma::math::Vertex &v2, const pragma::math::Vertex &v3);
		DLLNETWORK void AddTriangle(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t a, uint32_t b, uint32_t c);
		DLLNETWORK void SetSkinTextureIndex(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t texture);
		DLLNETWORK void Update(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void Update(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t flags);
		DLLNETWORK void AddVertex(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const pragma::math::Vertex &v);
		DLLNETWORK void GetBounds(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void SetVertex(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const pragma::math::Vertex &v);
		DLLNETWORK void SetVertexPosition(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const Vector3 &pos);
		DLLNETWORK void SetVertexNormal(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const Vector3 &normal);
		DLLNETWORK void SetVertexUV(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const Vector2 &uv);
		DLLNETWORK void SetVertexUV(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const std::string &uvSetName, uint32_t idx, const Vector2 &uv);
		DLLNETWORK void SetVertexAlpha(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const Vector2 &alpha);
		DLLNETWORK void SetVertexWeight(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx, const pragma::math::VertexWeight &weight);
		DLLNETWORK void GetVertex(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexPosition(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexNormal(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexUV(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexUV(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const std::string &uvSetName, uint32_t idx);
		DLLNETWORK void GetVertexAlpha(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexWeight(lua::State *l, pragma::geometry::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetCenter(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void GenerateNormals(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void NormalizeUVCoordinates(lua::State *l, pragma::geometry::ModelSubMesh &mdl);
		DLLNETWORK void ClipAgainstPlane(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const Vector3 &n, double d);
		DLLNETWORK void ClipAgainstPlane(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const Vector3 &n, double d, bool bSplitCoverMeshes);
		DLLNETWORK void ClipAgainstPlane(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const Vector3 &n, double d, bool bSplitCoverMeshes, luabind::object boneMatrices);
		DLLNETWORK void ApplyUVMapping(lua::State *l, pragma::geometry::ModelSubMesh &mdl, const Vector3 &nu, const Vector3 &nv, uint32_t w, uint32_t h, float ou, float ov, float su, float sv);
		DLLNETWORK void ApplyUVMapping(lua::State *l, pragma::geometry::ModelSubMesh &mesh, pragma::asset::Model &mdl, const Vector3 &nu, const Vector3 &nv, float ou, float ov, float su, float sv);
		DLLNETWORK void Scale(lua::State *l, pragma::geometry::ModelSubMesh &mesh, const Vector3 &scale);
	};
};
