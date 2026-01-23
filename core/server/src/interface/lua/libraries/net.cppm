// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.libraries.net;

import :entities.components;
export import luabind;

export namespace Lua::net {
	namespace server {
		DLLSERVER void broadcast(pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet);
		DLLSERVER bool register_net_message(const std::string &identifier);
		DLLSERVER void send(lua::State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, const luabind::tableT<pragma::SPlayerComponent> &recipients);
		DLLSERVER void send(lua::State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, pragma::networking::TargetRecipientFilter &recipients);
		DLLSERVER void send(lua::State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, pragma::SPlayerComponent &recipient);
		DLLSERVER void receive(lua::State *l, const std::string &name, const func<void> &function);
		DLLSERVER int register_event(lua::State *l);
	};
};
