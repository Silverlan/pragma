#ifndef __LCOLLISIONMESH_H__
#define __LCOLLISIONMESH_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/model/model.h"

namespace Lua
{
	namespace CollisionMesh
	{
		DLLNETWORK void register_class(luabind::class_<::CollisionMesh> &classDef);
		DLLNETWORK void Create(lua_State *l);
		DLLNETWORK void CreateBox(lua_State *l,const Vector3 &min,const Vector3 &max);
		DLLNETWORK void GetVertices(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetAABB(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetBoneParentId(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetOrigin(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetShape(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetSurfaceMaterialId(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetSurfaceMaterialIds(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void IntersectAABB(lua_State *l,::CollisionMesh &mesh,const Vector3 &min,const Vector3 &max);
		DLLNETWORK void IsConvex(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void SetAABB(lua_State *l,::CollisionMesh &mesh,const Vector3 &min,const Vector3 &max);
		DLLNETWORK void SetBoneParentId(lua_State *l,::CollisionMesh &mesh,int32_t boneId);
		DLLNETWORK void SetConvex(lua_State *l,::CollisionMesh &mesh,bool bConvex);
		DLLNETWORK void SetOrigin(lua_State *l,::CollisionMesh &mesh,const Vector3 &origin);
		DLLNETWORK void SetSurfaceMaterialId(lua_State *l,::CollisionMesh &mesh,int32_t surfaceMaterialId);
		DLLNETWORK void Update(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void AddVertex(lua_State *l,::CollisionMesh &mesh,const Vector3 &v);
		DLLNETWORK void GetVertexCount(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void Rotate(lua_State *l,::CollisionMesh &mesh,const Quat &rot);
		DLLNETWORK void Translate(lua_State *l,::CollisionMesh &mesh,const Vector3 &t);
		DLLNETWORK void GetTriangles(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void ClipAgainstPlane(lua_State *l,::CollisionMesh &mdl,const Vector3 &n,double d,::CollisionMesh &clippedMeshA,::CollisionMesh &clippedMeshB);
		DLLNETWORK void Centralize(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetVolume(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void SetVolume(lua_State *l,::CollisionMesh &mesh,float volume);

		DLLNETWORK void SetSoftBody(lua_State *l,::CollisionMesh &mesh,bool bSoftBody);
		DLLNETWORK void IsSoftBody(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetSoftBodyMesh(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void SetSoftBodyMesh(lua_State *l,::CollisionMesh &mesh,const std::shared_ptr<::ModelSubMesh> &subMesh);
		DLLNETWORK void GetSoftBodyInfo(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetSoftBodyTriangles(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void SetSoftBodyTriangles(lua_State *l,::CollisionMesh &mesh,luabind::object o);

		DLLNETWORK void AddSoftBodyAnchor(lua_State *l,::CollisionMesh &mesh,uint32_t vertIdx,uint32_t boneIdx,uint32_t flags,float influence);
		DLLNETWORK void AddSoftBodyAnchor(lua_State *l,::CollisionMesh &mesh,uint32_t vertIdx,uint32_t boneIdx,uint32_t flags);
		DLLNETWORK void AddSoftBodyAnchor(lua_State *l,::CollisionMesh &mesh,uint32_t vertIdx,uint32_t boneIdx);
		DLLNETWORK void RemoveSoftBodyAnchor(lua_State *l,::CollisionMesh &mesh,uint32_t anchorIdx);
		DLLNETWORK void ClearSoftBodyAnchors(lua_State *l,::CollisionMesh &mesh);
		DLLNETWORK void GetSoftBodyAnchors(lua_State *l,::CollisionMesh &mesh);
	};
};

#endif
