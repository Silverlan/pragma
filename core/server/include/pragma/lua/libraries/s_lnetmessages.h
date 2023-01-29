/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LSVNETMESSAGES_H__
#define __LSVNETMESSAGES_H__

#include "pragma/serverdefinitions.h"
#include <sharedutils/netpacket.hpp>
#include <pragma/lua/ldefinitions.h>

namespace pragma {
	namespace networking {
		class IServerClient;
		class TargetRecipientFilter;
	};
};
namespace Lua::net {
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

DLLSERVER void NET_sv_luanet(pragma::networking::IServerClient &session, NetPacket packet);

#endif
