// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.libraries.net;

import :entities;
import :entities.components;
import :game;
import :server_state;

////////////////////////////

static bool GetRecipients(const luabind::tableT<pragma::SPlayerComponent> &t, pragma::networking::TargetRecipientFilter &rp)
{
	for(luabind::iterator it {t}, end; it != end; ++it) {
		luabind::object o = *it;
		rp.AddRecipient(*luabind::object_cast<pragma::SPlayerComponent *>(o)->GetClientSession());
	}
	return true;
}
static bool GetRecipients(pragma::SPlayerComponent &pl, pragma::networking::TargetRecipientFilter &rp)
{
	rp.AddRecipient(*pl.GetClientSession());
	return true;
}

bool Lua::net::server::register_net_message(const std::string &identifier)
{
	if(!pragma::ServerState::Get()->IsGameActive())
		return false;
	pragma::Game *game = pragma::ServerState::Get()->GetGameState();
	return game->RegisterNetMessage(identifier);
}

void Lua::net::server::broadcast(pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet)
{
	::NetPacket packetNew;
	if(!NetIncludePacketID(pragma::ServerState::Get(), identifier, packet, packetNew)) {
		Con::CWAR << Con::PREFIX_SERVER << "Attempted to send unindexed lua net message: " << identifier << Con::endl;
		return;
	}
	pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::LUANET, packetNew, protocol);
}

static void send(lua::State *l, pragma::networking::Protocol protocol, const std::string &identifier, NetPacket &packet, const pragma::networking::TargetRecipientFilter &rp)
{
	NetPacket packetNew;
	if(!NetIncludePacketID(pragma::ServerState::Get(), identifier, packet, packetNew)) {
		Con::CWAR << Con::PREFIX_SERVER << "Attempted to send unindexed lua net message: " << identifier << Con::endl;
		return;
	}
	pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::LUANET, packetNew, protocol, rp);
}

void Lua::net::server::send(lua::State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, const luabind::tableT<pragma::SPlayerComponent> &recipients)
{
	pragma::networking::TargetRecipientFilter rp {};
	GetRecipients(recipients, rp);
	::send(l, protocol, identifier, packet, rp);
}
void Lua::net::server::send(lua::State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, pragma::networking::TargetRecipientFilter &recipients) { ::send(l, protocol, identifier, packet, recipients); }
void Lua::net::server::send(lua::State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, pragma::SPlayerComponent &recipient)
{
	pragma::networking::TargetRecipientFilter rp {};
	GetRecipients(recipient, rp);
	::send(l, protocol, identifier, packet, rp);
}

void Lua::net::server::receive(lua::State *l, const std::string &name, const func<void> &function)
{
	if(!pragma::ServerState::Get()->IsGameActive())
		return;
	pragma::Game *game = pragma::ServerState::Get()->GetGameState();
	function.push(l);
	int fc = create_reference(l, -1);
	Pop(l, 1);
	game->RegisterLuaNetMessage(name, fc);
}
