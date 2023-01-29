/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LCOLLISIONMESH_H__
#define __LCOLLISIONMESH_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/model/model.h"
#include "pragma/physics/collisionmesh.h"

namespace Lua {
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

#endif
