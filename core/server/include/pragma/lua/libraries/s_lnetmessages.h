/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LSVNETMESSAGES_H__
#define __LSVNETMESSAGES_H__
#include "pragma/serverdefinitions.h"
#include <pragma/lua/ldefinitions.h>
namespace pragma {namespace networking {class IServerClient;};};
DLLSERVER int Lua_sv_net_Broadcast(lua_State *l);
DLLSERVER int Lua_sv_net_Send(lua_State *l);
DLLSERVER int Lua_sv_net_RegisterMessage(lua_State *l);
DLLSERVER int Lua_sv_net_Receive(lua_State *l);
DLLSERVER int Lua_sv_net_Register(lua_State *l);
DLLSERVER void NET_sv_luanet(pragma::networking::IServerClient &session,NetPacket packet);
#endif