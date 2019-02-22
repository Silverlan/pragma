#ifndef __LCLNETMESSAGES_H__
#define __LCLNETMESSAGES_H__
#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
DLLCLIENT int Lua_cl_net_Send(lua_State *l);
DLLCLIENT int Lua_cl_net_Receive(lua_State *l);
DLLCLIENT void NET_cl_luanet(NetPacket &packet);
DLLCLIENT void NET_cl_luanet_reg(NetPacket &packet);
DLLCLIENT void NET_cl_register_net_event(NetPacket &packet);
#endif