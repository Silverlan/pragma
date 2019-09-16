#ifndef __C_LMODELMESH_H__
#define __C_LMODELMESH_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"

class ModelMesh;
class ModelSubMesh;

namespace Lua
{
	namespace ModelMesh
	{
		namespace Client
		{
			DLLCLIENT void Create(lua_State *l);
		};
	};
	namespace ModelSubMesh
	{
		namespace Client
		{
			DLLCLIENT void Create(lua_State *l);
			DLLCLIENT void CreateQuad(lua_State *l,float size);
			DLLCLIENT void CreateBox(lua_State *l,const Vector3 &min,const Vector3 &max);
			DLLCLIENT void CreateSphere(lua_State *l,const Vector3 &origin,float radius,uint32_t recursionLevel);
			DLLCLIENT void CreateSphere(lua_State *l,const Vector3 &origin,float radius);
			DLLCLIENT void CreateCylinder(lua_State *l,float startRadius,float length,uint32_t segmentCount);
			DLLCLIENT void CreateCone(lua_State *l,float startRadius,float length,float endRadius,uint32_t segmentCount);
			DLLCLIENT void CreateCircle(lua_State *l,float radius,bool doubleSided,uint32_t segmentCount);
			DLLCLIENT void CreateRing(lua_State *l,float innerRadius,float outerRadius,bool doubleSided,uint32_t segmentCount);
			DLLCLIENT void GetVkMesh(lua_State *l,::ModelSubMesh &mesh);
			DLLCLIENT void GetTangents(lua_State *l,::ModelSubMesh &mesh);
			DLLCLIENT void GetBiTangents(lua_State *l,::ModelSubMesh &mesh);
			DLLCLIENT void GetVertexBuffer(lua_State *l,::ModelSubMesh &mesh);
			DLLCLIENT void GetVertexWeightBuffer(lua_State *l,::ModelSubMesh &mesh);
			DLLCLIENT void GetAlphaBuffer(lua_State *l,::ModelSubMesh &mesh);
			DLLCLIENT void GetIndexBuffer(lua_State *l,::ModelSubMesh &mesh);
		};
	};
};

#endif