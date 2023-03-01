/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/lnetmessages.h"
#include "pragma/lua/libraries/c_lnetmessages.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/classes/ldef_netpacket.h"
#include <pragma/networking/enums.hpp>
#include <networkmanager/interface/nwm_manager.hpp>

void CGame::HandleLuaNetPacket(NetPacket &packet)
{
	unsigned int ID = packet->Read<unsigned int>();
	std::unordered_map<std::string, int>::iterator j;
	if(ID == 0)
		return;
	std::string *ident = GetNetMessageIdentifier(ID);
	if(ident == NULL)
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

extern ClientState *client;
void Lua::net::client::send(nwm::Protocol protocol, const std::string &identifier, ::NetPacket &packet)
{
	NetPacket packetNew;
	if(!NetIncludePacketID(::client, identifier, packet, packetNew)) {
		Con::cwar << Con::PREFIX_CLIENT << "Attempted to send unindexed lua net message: " << identifier << Con::endl;
		return;
	}
	switch(protocol) {
	case nwm::Protocol::TCP:
		::client->SendPacket("luanet", packetNew, pragma::networking::Protocol::SlowReliable);
		break;
	case nwm::Protocol::UDP:
		::client->SendPacket("luanet", packetNew, pragma::networking::Protocol::FastUnreliable);
		break;
	}
}

void Lua::net::client::receive(lua_State *l, const std::string &name, const Lua::func<void> &function)
{
	if(!::client->IsGameActive())
		return;
	Game *game = ::client->GetGameState();
	function.push(l);
	int fc = lua_createreference(l, 2);
	Lua::Pop(l, 1);
	game->RegisterLuaNetMessage(name, fc);
}

DLLCLIENT void NET_cl_luanet(NetPacket packet) { client->HandleLuaNetPacket(packet); }

DLLCLIENT void NET_cl_luanet_reg(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	std::string name = packet->ReadString();
	Game *game = client->GetGameState();
	game->RegisterNetMessage(name);
}

DLLCLIENT void NET_cl_register_net_event(NetPacket packet)
{
	if(!client->IsGameActive())
		return;
	static_cast<CGame *>(client->GetGameState())->OnReceivedRegisterNetEvent(packet);
}
