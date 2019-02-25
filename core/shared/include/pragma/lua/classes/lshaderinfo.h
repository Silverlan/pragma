#ifndef __LSHADERINFO_H__
#define __LSHADERINFO_H__
#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include <sharedutils/util_shaderinfo.hpp>
DLLNETWORK void Lua_ShaderInfo_GetName(lua_State *l,util::ShaderInfo *shader);
#endif