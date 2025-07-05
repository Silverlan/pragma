// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LMODELMESH_H__
#define __LMODELMESH_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/math/icosphere.h"

class ModelMesh;
class ModelSubMesh;
namespace umath {
	struct VertexWeight;
	struct Vertex;
};

namespace Lua {
	namespace ModelMesh {
		DLLNETWORK void register_class(luabind::class_<::ModelMesh> &classDef);
		DLLNETWORK void GetVertexCount(lua_State *l, ::ModelMesh &mdl);
		DLLNETWORK void GetTriangleVertexCount(lua_State *l, ::ModelMesh &mdl);
		DLLNETWORK void GetTriangleCount(lua_State *l, ::ModelMesh &mdl);
		DLLNETWORK void GetSubMeshes(lua_State *l, ::ModelMesh &mdl);
		DLLNETWORK void AddSubMesh(lua_State *l, ::ModelMesh &mdl, ::ModelSubMesh &mesh);
		DLLNETWORK void Update(lua_State *l, ::ModelMesh &mdl);
		DLLNETWORK void Update(lua_State *l, ::ModelMesh &mdl, uint32_t flags);
		DLLNETWORK void GetBounds(lua_State *l, ::ModelMesh &mdl);
		DLLNETWORK void GetCenter(lua_State *l, ::ModelMesh &mdl);
		DLLNETWORK void SetCenter(lua_State *l, ::ModelMesh &mdl, const Vector3 &center);
		DLLNETWORK void Centralize(lua_State *l, ::ModelMesh &mdl);
		DLLNETWORK void Scale(lua_State *l, ::ModelMesh &mdl, const Vector3 &scale);
	};
	namespace ModelSubMesh {
		DLLNETWORK void FlipTriangleWindingOrder(lua_State *l, ::ModelSubMesh &mesh);

		DLLNETWORK void register_class(luabind::class_<::ModelSubMesh> &classDef);
		DLLNETWORK void GetVertexCount(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void SetVertexCount(lua_State *l, ::ModelSubMesh &mdl, uint32_t n);
		DLLNETWORK void SetIndexCount(lua_State *l, ::ModelSubMesh &mdl, uint32_t n);
		DLLNETWORK void GetTriangleVertexCount(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void GetTriangleCount(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void GetSkinTextureIndex(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void GetVertices(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void GetTriangles(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void AddUVSet(lua_State *l, ::ModelSubMesh &mdl, const std::string &uvSetName);
		DLLNETWORK void GetUVMapping(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK luabind::object GetUVMapping(lua_State *l, ::ModelSubMesh &mdl, const std::string &uvSet);
		DLLNETWORK luabind::object GetUVSetNames(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void GetNormalMapping(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void GetVertexWeights(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void AddTriangle(lua_State *l, ::ModelSubMesh &mdl, const umath::Vertex &v1, const umath::Vertex &v2, const umath::Vertex &v3);
		DLLNETWORK void AddTriangle(lua_State *l, ::ModelSubMesh &mdl, uint32_t a, uint32_t b, uint32_t c);
		DLLNETWORK void SetSkinTextureIndex(lua_State *l, ::ModelSubMesh &mdl, uint32_t texture);
		DLLNETWORK void Update(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void Update(lua_State *l, ::ModelSubMesh &mdl, uint32_t flags);
		DLLNETWORK void AddVertex(lua_State *l, ::ModelSubMesh &mdl, const umath::Vertex &v);
		DLLNETWORK void GetBounds(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void SetVertex(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx, const umath::Vertex &v);
		DLLNETWORK void SetVertexPosition(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx, const Vector3 &pos);
		DLLNETWORK void SetVertexNormal(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx, const Vector3 &normal);
		DLLNETWORK void SetVertexUV(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx, const ::Vector2 &uv);
		DLLNETWORK void SetVertexUV(lua_State *l, ::ModelSubMesh &mdl, const std::string &uvSetName, uint32_t idx, const ::Vector2 &uv);
		DLLNETWORK void SetVertexAlpha(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx, const ::Vector2 &alpha);
		DLLNETWORK void SetVertexWeight(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx, const umath::VertexWeight &weight);
		DLLNETWORK void GetVertex(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexPosition(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexNormal(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexUV(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexUV(lua_State *l, ::ModelSubMesh &mdl, const std::string &uvSetName, uint32_t idx);
		DLLNETWORK void GetVertexAlpha(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetVertexWeight(lua_State *l, ::ModelSubMesh &mdl, uint32_t idx);
		DLLNETWORK void GetCenter(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void GenerateNormals(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void NormalizeUVCoordinates(lua_State *l, ::ModelSubMesh &mdl);
		DLLNETWORK void ClipAgainstPlane(lua_State *l, ::ModelSubMesh &mdl, const Vector3 &n, double d);
		DLLNETWORK void ClipAgainstPlane(lua_State *l, ::ModelSubMesh &mdl, const Vector3 &n, double d, bool bSplitCoverMeshes);
		DLLNETWORK void ClipAgainstPlane(lua_State *l, ::ModelSubMesh &mdl, const Vector3 &n, double d, bool bSplitCoverMeshes, luabind::object boneMatrices);
		DLLNETWORK void ApplyUVMapping(lua_State *l, ::ModelSubMesh &mdl, const Vector3 &nu, const Vector3 &nv, uint32_t w, uint32_t h, float ou, float ov, float su, float sv);
		DLLNETWORK void ApplyUVMapping(lua_State *l, ::ModelSubMesh &mesh, ::Model &mdl, const Vector3 &nu, const Vector3 &nv, float ou, float ov, float su, float sv);
		DLLNETWORK void Scale(lua_State *l, ::ModelSubMesh &mesh, const Vector3 &scale);
	};
};

#endif
