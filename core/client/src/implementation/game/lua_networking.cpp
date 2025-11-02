// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"

#include "pragma/lua/core.hpp"

#include "stdafx_client.h"

module pragma.client;

import :game;

void CGame::HandleLuaNetPacket(NetPacket &packet)
{
	unsigned int ID = packet->Read<unsigned int>();
	std::unordered_map<std::string, int>::iterator j;
	if(ID == 0)
		return;
	std::string *ident = GetNetMessageIdentifier(ID);
	if(ident == nullptr)
		return;
	std::unordered_map<std::string, int>::iterator i = m_luaNetMessages.find(*ident);
	if(i == m_luaNetMessages.end()) {
		Con::cwar << Con::PREFIX_CLIENT << "Unhandled lua net message: " << *ident << Con::endl;
		return;
	}
	ProtectedLuaCall(
	  [this, &i, &packet](lua_State *) {
		  lua_rawgeti(GetLuaState(), LUA_REGISTRYINDEX, i->second);

		  luabind::object(GetLuaState(), packet).push(GetLuaState());
		  return Lua::StatusCode::Ok;
	  },
	  0);
}

////////////////////////////

void Lua::net::client::send(nwm::Protocol protocol, const std::string &identifier, ::NetPacket &packet)
{
	::NetPacket packetNew;
	auto *client = pragma::get_client_state();
	if(!NetIncludePacketID(client, identifier, packet, packetNew)) {
		Con::cwar << Con::PREFIX_CLIENT << "Attempted to send unindexed lua net message: " << identifier << Con::endl;
		return;
	}
	switch(protocol) {
	case nwm::Protocol::TCP:
		client->SendPacket("luanet", packetNew, pragma::networking::Protocol::SlowReliable);
		break;
	case nwm::Protocol::UDP:
		client->SendPacket("luanet", packetNew, pragma::networking::Protocol::FastUnreliable);
		break;
	}
}

void Lua::net::client::receive(lua_State *l, const std::string &name, const Lua::func<void> &function)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	pragma::Game *game = client->GetGameState();
	function.push(l);
	int fc = lua_createreference(l, 2);
	Lua::Pop(l, 1);
	game->RegisterLuaNetMessage(name, fc);
}

