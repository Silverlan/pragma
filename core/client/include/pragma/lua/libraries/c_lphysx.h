#ifndef __C_LPHYSX_H__
#define __C_LPHYSX_H__

#ifdef PHYS_ENGINE_PHYSX
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
DLLCLIENT int Lua_physx_SetVisualizationParameter(lua_State *l);
DLLCLIENT int Lua_physx_SetVisualizationEnabled(lua_State *l);
#endif

#endif