#ifndef __C_LSHADERINFO_H__
#define __C_LSHADERINFO_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
namespace util {class ShaderInfo;};

DLLCLIENT void Lua_ShaderInfo_GetID(lua_State *l,util::ShaderInfo &shader);
#endif