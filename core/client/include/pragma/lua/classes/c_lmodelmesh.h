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
			DLLCLIENT void CreateBox(lua_State *l,const Vector3 &min,const Vector3 &max);
			DLLCLIENT void CreateSphere(lua_State *l,const Vector3 &origin,float radius,uint32_t recursionLevel);
			DLLCLIENT void CreateSphere(lua_State *l,const Vector3 &origin,float radius);
			DLLCLIENT void GetVkMesh(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh);
			DLLCLIENT void GetTangents(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh);
			DLLCLIENT void GetBiTangents(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh);
			DLLCLIENT void GetVertexBuffer(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh);
			DLLCLIENT void GetVertexWeightBuffer(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh);
			DLLCLIENT void GetAlphaBuffer(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh);
			DLLCLIENT void GetIndexBuffer(lua_State *l,std::shared_ptr<::ModelSubMesh> &mesh);
		};
	};
};

#endif