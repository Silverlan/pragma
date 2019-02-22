#ifndef __S_LMODELMESH_H__
#define __S_LMODELMESH_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>

class ModelMesh;

namespace Lua
{
	namespace ModelMesh
	{
		namespace Server
		{
			DLLSERVER void Create(lua_State *l);
		};
	};
	namespace ModelSubMesh
	{
		namespace Server
		{
			DLLSERVER void Create(lua_State *l);
			DLLSERVER void CreateBox(lua_State *l,const Vector3 &min,const Vector3 &max);
			DLLSERVER void CreateSphere(lua_State *l,const Vector3 &origin,float radius,uint32_t recursionLevel);
			DLLSERVER void CreateSphere(lua_State *l,const Vector3 &origin,float radius);
		};
	};
};

#endif