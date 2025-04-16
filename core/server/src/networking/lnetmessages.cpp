/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/lua/lnetmessages.h"
#include "pragma/game/s_game.h"
#include <servermanager/sv_nwm_recipientfilter.h>
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/lua/classes/lrecipientfilter.h"
#include "pragma/entities/player.h"
#include "pragma/lua/classes/ldef_recipientfilter.h"
#include "pragma/lua/classes/ldef_netpacket.h"
#include "pragma/lua/classes/ldef_recipientfilter.h"
#include "pragma/lua/libraries/s_lnetmessages.h"
#include "pragma/entities/components/s_player_component.hpp"
#include <pragma/networking/enums.hpp>
#include <pragma/lua/lua_handle.hpp>
#include <pragma/lua/luaapi.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>

void SGame::HandleLuaNetPacket(pragma::networking::IServerClient &session, NetPacket &packet)
{
	unsigned int ID = packet->Read<unsigned int>();
	if(ID == 0)
		return;
	auto *pl = GetPlayer(session);
	if(pl == nullptr)
		return;
	std::string *ident = GetNetMessageIdentifier(ID);
	if(ident == nullptr)
		return;
	std::unordered_map<std::string, int>::iterator i = m_luaNetMessages.find(*ident);
	if(i == m_luaNetMessages.end()) {
		Con::cwar << Con::PREFIX_SERVER << "Unhandled lua net message: " << *ident << Con::endl;
		return;
	}
	ProtectedLuaCall(
	  [&i, &pl, &packet](lua_State *l) {
		  lua_rawgeti(l, LUA_REGISTRYINDEX, i->second);
		  luabind::object(l, packet).push(l);
		  pl->PushLuaObject(l);
		  return Lua::StatusCode::Ok;
	  },
	  0);
}

////////////////////////////

extern ServerState *server;
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
	if(!::server->IsGameActive())
		return false;
	Game *game = ::server->GetGameState();
	return game->RegisterNetMessage(identifier);
}

void Lua::net::server::broadcast(pragma::networking::Protocol protocol, const std::string &identifier, NetPacket &packet)
{
	NetPacket packetNew;
	if(!NetIncludePacketID(::server, identifier, packet, packetNew)) {
		Con::cwar << Con::PREFIX_SERVER << "Attempted to send unindexed lua net message: " << identifier << Con::endl;
		return;
	}
	::server->SendPacket("luanet", packetNew, protocol);
}

static void send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, NetPacket &packet, const pragma::networking::TargetRecipientFilter &rp)
{
	NetPacket packetNew;
	if(!NetIncludePacketID(::server, identifier, packet, packetNew)) {
		Con::cwar << Con::PREFIX_SERVER << "Attempted to send unindexed lua net message: " << identifier << Con::endl;
		return;
	}
	::server->SendPacket("luanet", packetNew, protocol, rp);
}

void Lua::net::server::send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, NetPacket &packet, const luabind::tableT<pragma::SPlayerComponent> &recipients)
{
	pragma::networking::TargetRecipientFilter rp {};
	GetRecipients(recipients, rp);
	::send(l, protocol, identifier, packet, rp);
}
void Lua::net::server::send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, NetPacket &packet, pragma::networking::TargetRecipientFilter &recipients) { ::send(l, protocol, identifier, packet, recipients); }
void Lua::net::server::send(lua_State *l, pragma::networking::Protocol protocol, const std::string &identifier, NetPacket &packet, pragma::SPlayerComponent &recipient)
{
	pragma::networking::TargetRecipientFilter rp {};
	GetRecipients(recipient, rp);
	::send(l, protocol, identifier, packet, rp);
}

void Lua::net::server::receive(lua_State *l, const std::string &name, const Lua::func<void> &function)
{
	if(!::server->IsGameActive())
		return;
	Game *game = ::server->GetGameState();
	function.push(l);
	int fc = lua_createreference(l, -1);
	Lua::Pop(l, 1);
	game->RegisterLuaNetMessage(name, fc);
}

DLLSERVER void NET_sv_luanet(pragma::networking::IServerClient &session, NetPacket packet) { server->HandleLuaNetPacket(session, packet); }
