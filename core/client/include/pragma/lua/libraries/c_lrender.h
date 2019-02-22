#ifndef __C_LRENDER_H__
#define __C_LRENDER_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
DLLCLIENT int Lua_render_SetMaterialOverride(lua_State *l);
DLLCLIENT int Lua_render_SetColorScale(lua_State *l);
DLLCLIENT int Lua_render_SetAlphaScale(lua_State *l);
#endif