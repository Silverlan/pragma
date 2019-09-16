#ifndef __LMODELMESH_H__
#define __LMODELMESH_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/math/icosphere.h"

class ModelMesh;
class ModelSubMesh;
struct VertexWeight;
struct Vertex;

namespace Lua
{
	namespace ModelMesh
	{
		DLLNETWORK void register_class(luabind::class_<::ModelMesh> &classDef);
		DLLNETWORK void GetVertexCount(lua_State *l,::ModelMesh &mdl);
		DLLNETWORK void GetTriangleVertexCount(lua_State *l,::ModelMesh &mdl);
		DLLNETWORK void GetTriangleCount(lua_State *l,::ModelMesh &mdl);
		DLLNETWORK void GetSubMeshes(lua_State *l,::ModelMesh &mdl);
		DLLNETWORK void AddSubMesh(lua_State *l,::ModelMesh &mdl,::ModelSubMesh &mesh);
		DLLNETWORK void Update(lua_State *l,::ModelMesh &mdl);
		DLLNETWORK void Update(lua_State *l,::ModelMesh &mdl,uint32_t flags);
		DLLNETWORK void GetBounds(lua_State *l,::ModelMesh &mdl);
		DLLNETWORK void GetCenter(lua_State *l,::ModelMesh &mdl);
		DLLNETWORK void SetCenter(lua_State *l,::ModelMesh &mdl,const Vector3 &center);
		DLLNETWORK void Centralize(lua_State *l,::ModelMesh &mdl);
		DLLNETWORK void Scale(lua_State *l,::ModelMesh &mdl,const Vector3 &scale);
	};
	namespace ModelSubMesh
	{
		DLLNETWORK void InitializeQuad(lua_State *l,::ModelSubMesh &mesh,float size);
		DLLNETWORK void InitializeBox(lua_State *l,::ModelSubMesh &mesh,const Vector3 &min,const Vector3 &max);
		DLLNETWORK void InitializeSphere(lua_State *l,::ModelSubMesh &mesh,const Vector3 &origin,float radius,uint32_t recursionLevel=1);
		DLLNETWORK void InitializeCylinder(lua_State *l,::ModelSubMesh &mesh,float startRadius,float length,uint32_t segmentCount=12);
		DLLNETWORK void InitializeCone(lua_State *l,::ModelSubMesh &mesh,float startRadius,float length,float endRadius,uint32_t segmentCount=12);
		DLLNETWORK void InitializeCircle(lua_State *l,::ModelSubMesh &mesh,float radius,bool doubleSided=true,uint32_t segmentCount=36);
		DLLNETWORK void InitializeRing(lua_State *l,::ModelSubMesh &mesh,std::optional<float> innerRadius,float outerRadius,bool doubleSided=true,uint32_t segmentCount=36);

		DLLNETWORK void register_class(luabind::class_<::ModelSubMesh> &classDef);
		DLLNETWORK void GetVertexCount(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void GetTriangleVertexCount(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void GetTriangleCount(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void GetTexture(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void GetVertices(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void GetTriangles(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void GetUVMapping(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void GetNormalMapping(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void GetVertexWeights(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void AddTriangle(lua_State *l,::ModelSubMesh &mdl,const ::Vertex &v1,const ::Vertex &v2,const ::Vertex &v3);
		DLLNETWORK void AddTriangle(lua_State *l,::ModelSubMesh &mdl,uint32_t a,uint32_t b,uint32_t c);
		DLLNETWORK void SetTexture(lua_State *l,::ModelSubMesh &mdl,uint32_t texture);
		DLLNETWORK void Update(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void Update(lua_State *l,::ModelSubMesh &mdl,uint32_t flags);
		DLLNETWORK void AddVertex(lua_State *l,::ModelSubMesh &mdl,const ::Vertex &v);
		DLLNETWORK void GetBounds(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void SetVertex(lua_State *l,::ModelSubMesh &mdl,uint32_t idx,const ::Vertex &v);
		DLLNETWORK void SetVertexPosition(lua_State *l,::ModelSubMesh &mdl,uint32_t idx,const Vector3 &pos);
		DLLNETWORK void SetVertexNormal(lua_State *l,::ModelSubMesh &mdl,uint32_t idx,const Vector3 &normal);
		DLLNETWORK void SetVertexUV(lua_State *l,::ModelSubMesh &mdl,uint32_t idx,const ::Vector2 &uv);
		DLLNETWORK void SetVertexAlpha(lua_State *l,::ModelSubMesh &mdl,uint32_t idx,const ::Vector2 &alpha);
		DLLNETWORK void SetVertexWeight(lua_State *l,::ModelSubMesh &mdl,uint32_t idx,const ::VertexWeight &weight);
		DLLNETWORK void GetVertex(lua_State *l,::ModelSubMesh &mdl,uint32_t idx);
		DLLNETWORK void GetVertexPosition(lua_State *l,::ModelSubMesh &mdl,uint32_t idx);
		DLLNETWORK void GetVertexNormal(lua_State *l,::ModelSubMesh &mdl,uint32_t idx);
		DLLNETWORK void GetVertexUV(lua_State *l,::ModelSubMesh &mdl,uint32_t idx);
		DLLNETWORK void GetVertexAlpha(lua_State *l,::ModelSubMesh &mdl,uint32_t idx);
		DLLNETWORK void GetVertexWeight(lua_State *l,::ModelSubMesh &mdl,uint32_t idx);
		DLLNETWORK void GetCenter(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void Optimize(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void GenerateNormals(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void NormalizeUVCoordinates(lua_State *l,::ModelSubMesh &mdl);
		DLLNETWORK void ClipAgainstPlane(lua_State *l,::ModelSubMesh &mdl,const Vector3 &n,double d);
		DLLNETWORK void ClipAgainstPlane(lua_State *l,::ModelSubMesh &mdl,const Vector3 &n,double d,bool bSplitCoverMeshes);
		DLLNETWORK void ClipAgainstPlane(lua_State *l,::ModelSubMesh &mdl,const Vector3 &n,double d,bool bSplitCoverMeshes,luabind::object boneMatrices);
		DLLNETWORK void ApplyUVMapping(lua_State *l,::ModelSubMesh &mdl,const Vector3 &nu,const Vector3 &nv,uint32_t w,uint32_t h,float ou,float ov,float su,float sv);
		DLLNETWORK void ApplyUVMapping(lua_State *l,::ModelSubMesh &mesh,::Model &mdl,const Vector3 &nu,const Vector3 &nv,float ou,float ov,float su,float sv);
		DLLNETWORK void Scale(lua_State *l,::ModelSubMesh &mesh,const Vector3 &scale);
	};
};

#endif
