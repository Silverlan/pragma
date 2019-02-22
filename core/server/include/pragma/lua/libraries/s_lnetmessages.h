#ifndef __LSVNETMESSAGES_H__
#define __LSVNETMESSAGES_H__
#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
DLLSERVER int Lua_sv_net_Broadcast(lua_State *l);
DLLSERVER int Lua_sv_net_Send(lua_State *l);
DLLSERVER int Lua_sv_net_RegisterMessage(lua_State *l);
DLLSERVER int Lua_sv_net_Receive(lua_State *l);
DLLSERVER int Lua_sv_net_Register(lua_State *l);
DLLSERVER void NET_sv_luanet(WVServerClient *session,NetPacket packet);
#endif