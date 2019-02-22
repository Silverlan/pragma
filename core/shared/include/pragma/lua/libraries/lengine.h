#ifndef __LENGINE_H__
#define __LENGINE_H__
#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
DLLNETWORK int Lua_engine_CreateLight(lua_State *l);
DLLNETWORK int Lua_engine_RemoveLights(lua_State *l);
DLLNETWORK int Lua_engine_CreateSprite(lua_State *l);
DLLNETWORK int Lua_engine_PrecacheModel_sv(lua_State *l);
DLLNETWORK int Lua_engine_LoadSoundScripts(lua_State *l);
DLLNETWORK int Lua_engine_LoadLibrary(lua_State *l);
DLLNETWORK int Lua_engine_GetTickCount(lua_State *l);
#endif