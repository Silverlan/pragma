// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LCLNETMESSAGES_H__
#define __LCLNETMESSAGES_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace nwm {
	enum class Protocol : uint32_t;
};
namespace Lua::net::client {
	DLLCLIENT void send(nwm::Protocol protocol, const std::string &identifier, ::NetPacket &packet);
	DLLCLIENT void receive(lua_State *l, const std::string &name, const Lua::func<void> &function);
};
DLLCLIENT void NET_cl_luanet(NetPacket &packet);
DLLCLIENT void NET_cl_luanet_reg(NetPacket &packet);
DLLCLIENT void NET_cl_register_net_event(NetPacket &packet);

#endif
