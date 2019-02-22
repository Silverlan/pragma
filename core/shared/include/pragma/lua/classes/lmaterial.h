#ifndef __LMATERIAL_H__
#define __LMATERIAL_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"

class Material;
namespace Lua
{
	namespace Material
	{
		DLLNETWORK void register_class(luabind::class_<::Material> &classDef);
		DLLNETWORK void IsValid(lua_State *l,::Material &mat);
		DLLNETWORK void GetShaderName(lua_State *l,::Material &mat);
		DLLNETWORK void GetName(lua_State *l,::Material &mat);
		DLLNETWORK void IsTranslucent(lua_State *l,::Material &mat);
		DLLNETWORK void GetDataBlock(lua_State *l,::Material &mat);
	};
};

#endif
