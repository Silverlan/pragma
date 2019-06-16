#ifndef __C_LMODEL_H__
#define __C_LMODEL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"

class CModel;
namespace Lua
{
	namespace Model
	{
		namespace Client
		{
			DLLCLIENT void AddMaterial(lua_State *l,::Model &mdl,uint32_t textureGroup,const std::string &name);
			DLLCLIENT void SetMaterial(lua_State *l,::Model &mdl,uint32_t texIdx,const std::string &name);
			DLLCLIENT void GetVertexAnimationBuffer(lua_State *l,::Model &mdl);
		};
	};
};

#endif