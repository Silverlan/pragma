/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LCLNETMESSAGES_H__
#define __LCLNETMESSAGES_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace nwm {enum class Protocol : uint32_t;};
namespace Lua::net::client
{
	DLLCLIENT void send(nwm::Protocol protocol,const std::string &identifier,::NetPacket &packet);
	DLLCLIENT void receive(lua_State *l,const std::string &name,const Lua::func<void> &function);
};
DLLCLIENT void NET_cl_luanet(NetPacket &packet);
DLLCLIENT void NET_cl_luanet_reg(NetPacket &packet);
DLLCLIENT void NET_cl_register_net_event(NetPacket &packet);

#endif
