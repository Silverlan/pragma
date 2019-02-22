#ifndef __LSHADERINFO_H__
#define __LSHADERINFO_H__
#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include "shaderinfo.h"
DLLNETWORK void Lua_ShaderInfo_GetName(lua_State *l,ShaderInfo *shader);
#endif