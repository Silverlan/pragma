// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"
#include "mathutil/uvec.h"

export module pragma.shared:scripting.lua.classes.collision_mesh;

export import :physics.collision_mesh;

export namespace Lua {
	namespace CollisionMesh {
		DLLNETWORK void register_class(luabind::class_<::CollisionMesh> &classDef);
		DLLNETWORK std::shared_ptr<::CollisionMesh> Create(lua_State *l);
		DLLNETWORK std::shared_ptr<::CollisionMesh> CreateBox(lua_State *l, const Vector3 &min, const Vector3 &max);
		DLLNETWORK luabind::tableT<Vector3> GetVertices(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK luabind::mult<Vector3, Vector3> GetAABB(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK luabind::optional<pragma::physics::IShape> GetShape(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK luabind::tableT<int32_t> GetSurfaceMaterialIds(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK bool IntersectAABB(lua_State *l, ::CollisionMesh &mesh, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void SetAABB(lua_State *l, ::CollisionMesh &mesh, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void SetConvex(lua_State *l, ::CollisionMesh &mesh, bool bConvex);
		DLLNETWORK void SetOrigin(lua_State *l, ::CollisionMesh &mesh, const Vector3 &origin);
		DLLNETWORK void SetSurfaceMaterialId(lua_State *l, ::CollisionMesh &mesh, int32_t surfaceMaterialId);
		DLLNETWORK void Update(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK uint32_t GetVertexCount(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK void GetTriangles(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK void ClipAgainstPlane(lua_State *l, ::CollisionMesh &mdl, const Vector3 &n, double d, ::CollisionMesh &clippedMeshA, ::CollisionMesh &clippedMeshB);
		DLLNETWORK void Centralize(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK void GetVolume(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK void SetVolume(lua_State *l, ::CollisionMesh &mesh, float volume);

		DLLNETWORK void SetSoftBody(lua_State *l, ::CollisionMesh &mesh, bool bSoftBody);
		DLLNETWORK void IsSoftBody(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK void GetSoftBodyMesh(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK void SetSoftBodyMesh(lua_State *l, ::CollisionMesh &mesh, const std::shared_ptr<::ModelSubMesh> &subMesh);
		DLLNETWORK void GetSoftBodyInfo(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK luabind::optional<luabind::tableT<uint32_t>> GetSoftBodyTriangles(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK void SetSoftBodyTriangles(lua_State *l, ::CollisionMesh &mesh, const luabind::tableT<uint32_t> &o);

		DLLNETWORK luabind::optional<uint32_t> AddSoftBodyAnchor(lua_State *l, ::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx, ::CollisionMesh::SoftBodyAnchor::Flags flags, float influence);
		DLLNETWORK luabind::optional<uint32_t> AddSoftBodyAnchor(lua_State *l, ::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx, ::CollisionMesh::SoftBodyAnchor::Flags flags);
		DLLNETWORK luabind::optional<uint32_t> AddSoftBodyAnchor(lua_State *l, ::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx);
		DLLNETWORK void RemoveSoftBodyAnchor(lua_State *l, ::CollisionMesh &mesh, uint32_t anchorIdx);
		DLLNETWORK void ClearSoftBodyAnchors(lua_State *l, ::CollisionMesh &mesh);
		DLLNETWORK luabind::tableT<void> GetSoftBodyAnchors(lua_State *l, ::CollisionMesh &mesh);
	};
};
