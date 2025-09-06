// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/enums.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <sharedutils/netpacket.hpp>
#include <pragma/lua/ldefinitions.h>

export module pragma.server.scripting.lua.libraries.net;

import pragma.server.entities.components;

export namespace Lua::net {
	namespace server {
		DLLSERVER void broadcast(pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet);
		DLLSERVER bool register_net_message(const std::string &identifier);
		DLLSERVER void send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, const luabind::tableT<pragma::SPlayerComponent> &recipients);
		DLLSERVER void send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, pragma::networking::TargetRecipientFilter &recipients);
		DLLSERVER void send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, pragma::SPlayerComponent &recipient);
		DLLSERVER void receive(lua_State *l, const std::string &name, const Lua::func<void> &function);
		DLLSERVER int register_event(lua_State *l);
	};
};

export DLLSERVER void NET_sv_luanet(pragma::networking::IServerClient &session, NetPacket packet);
