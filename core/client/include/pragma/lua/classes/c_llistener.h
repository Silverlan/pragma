#ifndef __C_LLISTENER_H__
#define __C_LLISTENER_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
class ListenerHandle;
DLLCLIENT void Lua_Listener_GetGain(lua_State *l,ListenerHandle &hEnt);
DLLCLIENT void Lua_Listener_SetGain(lua_State *l,ListenerHandle &hEnt,float gain);
#endif