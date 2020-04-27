/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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