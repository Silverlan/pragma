// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"
#include "stdafx_server.h"

module pragma.server.scripting.lua.libraries.net;

import pragma.server.entities;
import pragma.server.entities.components;
import pragma.server.game;
import pragma.server.server_state;

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
	if(!::ServerState::Get()->IsGameActive())
		return false;
	pragma::Game *game = ::ServerState::Get()->GetGameState();
	return game->RegisterNetMessage(identifier);
}

void Lua::net::server::broadcast(pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet)
{
	::NetPacket packetNew;
	if(!NetIncludePacketID(::ServerState::Get(), identifier, packet, packetNew)) {
		Con::cwar << Con::PREFIX_SERVER << "Attempted to send unindexed lua net message: " << identifier << Con::endl;
		return;
	}
	::ServerState::Get()->SendPacket("luanet", packetNew, protocol);
}

static void send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, const pragma::networking::TargetRecipientFilter &rp)
{
	::NetPacket packetNew;
	if(!NetIncludePacketID(::ServerState::Get(), identifier, packet, packetNew)) {
		Con::cwar << Con::PREFIX_SERVER << "Attempted to send unindexed lua net message: " << identifier << Con::endl;
		return;
	}
	::ServerState::Get()->SendPacket("luanet", packetNew, protocol, rp);
}

void Lua::net::server::send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, const luabind::tableT<pragma::SPlayerComponent> &recipients)
{
	pragma::networking::TargetRecipientFilter rp {};
	GetRecipients(recipients, rp);
	::send(l, protocol, identifier, packet, rp);
}
void Lua::net::server::send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, pragma::networking::TargetRecipientFilter &recipients) { ::send(l, protocol, identifier, packet, recipients); }
void Lua::net::server::send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, ::NetPacket &packet, pragma::SPlayerComponent &recipient)
{
	pragma::networking::TargetRecipientFilter rp {};
	GetRecipients(recipient, rp);
	::send(l, protocol, identifier, packet, rp);
}

void Lua::net::server::receive(lua_State *l, const std::string &name, const Lua::func<void> &function)
{
	if(!::ServerState::Get()->IsGameActive())
		return;
	pragma::Game *game = ::ServerState::Get()->GetGameState();
	function.push(l);
	int fc = lua_createreference(l, -1);
	Lua::Pop(l, 1);
	game->RegisterLuaNetMessage(name, fc);
}
