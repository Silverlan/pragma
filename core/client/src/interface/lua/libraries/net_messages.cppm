// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.net_messages;

export import pragma.shared;

export {
	namespace Lua::net::client {
		DLLCLIENT void send(nwm::Protocol protocol, const std::string &identifier, ::NetPacket &packet);
		DLLCLIENT void receive(lua::State *l, const std::string &name, const func<void> &function);
	};
	DLLCLIENT void NET_cl_luanet(NetPacket &packet);
	DLLCLIENT void NET_cl_luanet_reg(NetPacket &packet);
	DLLCLIENT void NET_cl_register_net_event(NetPacket &packet);
};
