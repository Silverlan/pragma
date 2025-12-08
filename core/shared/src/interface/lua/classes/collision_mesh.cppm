// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.collision_mesh;

export import :physics.collision_mesh;
export import :scripting.lua.api;
export import :types;

export namespace Lua {
	namespace CollisionMesh {
		DLLNETWORK void register_class(luabind::class_<pragma::physics::CollisionMesh> &classDef);
		DLLNETWORK std::shared_ptr<pragma::physics::CollisionMesh> Create(lua::State *l);
		DLLNETWORK std::shared_ptr<pragma::physics::CollisionMesh> CreateBox(lua::State *l, const Vector3 &min, const Vector3 &max);
		DLLNETWORK luabind::tableT<Vector3> GetVertices(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK luabind::mult<Vector3, Vector3> GetAABB(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK luabind::optional<pragma::physics::IShape> GetShape(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK luabind::tableT<int32_t> GetSurfaceMaterialIds(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK bool IntersectAABB(lua::State *l, pragma::physics::CollisionMesh &mesh, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void SetAABB(lua::State *l, pragma::physics::CollisionMesh &mesh, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void SetConvex(lua::State *l, pragma::physics::CollisionMesh &mesh, bool bConvex);
		DLLNETWORK void SetOrigin(lua::State *l, pragma::physics::CollisionMesh &mesh, const Vector3 &origin);
		DLLNETWORK void SetSurfaceMaterialId(lua::State *l, pragma::physics::CollisionMesh &mesh, int32_t surfaceMaterialId);
		DLLNETWORK void Update(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK uint32_t GetVertexCount(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK void GetTriangles(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK void ClipAgainstPlane(lua::State *l, pragma::physics::CollisionMesh &mdl, const Vector3 &n, double d, pragma::physics::CollisionMesh &clippedMeshA, pragma::physics::CollisionMesh &clippedMeshB);
		DLLNETWORK void Centralize(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK void GetVolume(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK void SetVolume(lua::State *l, pragma::physics::CollisionMesh &mesh, float volume);

		DLLNETWORK void SetSoftBody(lua::State *l, pragma::physics::CollisionMesh &mesh, bool bSoftBody);
		DLLNETWORK void IsSoftBody(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK void GetSoftBodyMesh(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK void SetSoftBodyMesh(lua::State *l, pragma::physics::CollisionMesh &mesh, const std::shared_ptr<pragma::geometry::ModelSubMesh> &subMesh);
		DLLNETWORK void GetSoftBodyInfo(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK luabind::optional<luabind::tableT<uint32_t>> GetSoftBodyTriangles(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK void SetSoftBodyTriangles(lua::State *l, pragma::physics::CollisionMesh &mesh, const luabind::tableT<uint32_t> &o);

		DLLNETWORK luabind::optional<uint32_t> AddSoftBodyAnchor(lua::State *l, pragma::physics::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx, pragma::physics::CollisionMesh::SoftBodyAnchor::Flags flags, float influence);
		DLLNETWORK luabind::optional<uint32_t> AddSoftBodyAnchor(lua::State *l, pragma::physics::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx, pragma::physics::CollisionMesh::SoftBodyAnchor::Flags flags);
		DLLNETWORK luabind::optional<uint32_t> AddSoftBodyAnchor(lua::State *l, pragma::physics::CollisionMesh &mesh, uint32_t vertIdx, uint32_t boneIdx);
		DLLNETWORK void RemoveSoftBodyAnchor(lua::State *l, pragma::physics::CollisionMesh &mesh, uint32_t anchorIdx);
		DLLNETWORK void ClearSoftBodyAnchors(lua::State *l, pragma::physics::CollisionMesh &mesh);
		DLLNETWORK luabind::tableT<void> GetSoftBodyAnchors(lua::State *l, pragma::physics::CollisionMesh &mesh);
	};
};
