#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/lua/lnetmessages.h"
#include "pragma/game/s_game.h"
#include <servermanager/sv_nwm_recipientfilter.h>
#include "pragma/networking/wvserverclient.h"
#include "pragma/lua/classes/lrecipientfilter.h"
#include "pragma/entities/player.h"
#include "pragma/lua/classes/ldef_recipientfilter.h"
#include "pragma/lua/classes/ldef_netpacket.h"
#include "pragma/lua/classes/ldef_recipientfilter.h"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/lua/lua_handle.hpp>
#include <luasystem.h>

#pragma optimize("",off)
void SGame::HandleLuaNetPacket(WVServerClient *session,NetPacket &packet)
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
	std::unordered_map<std::string,int>::iterator i = m_luaNetMessages.find(*ident);
	if(i == m_luaNetMessages.end())
	{
		Con::csv<<"WARNING: Unhandled lua net message: "<<*ident<<Con::endl;
		return;
	}
	ProtectedLuaCall([&i,&pl,&packet](lua_State *l) {
		lua_rawgeti(l,LUA_REGISTRYINDEX,i->second);
		luabind::object(l,packet).push(l);
		pl->PushLuaObject(l);
		return Lua::StatusCode::Ok;
	},0);
}

////////////////////////////

extern ServerState *server;
DLLSERVER bool GetRecipients(lua_State *l,int arg,nwm::RecipientFilter *rp)
{
	if(lua_istable(l,arg))
	{
		luaL_checktype(l,arg,LUA_TTABLE);
		lua_pushnil(l);
		while(lua_next(l,-2) != 0)
		{
			if(lua_isnumber(l,-2))
			{
				auto *hnd = Lua::CheckSPlayer(l,-1);
				if(hnd->expired() == false)
				{
					WVServerClient *session = (*hnd)->GetClientSession();
					if(session != NULL)
						rp->Add(session);
				}
			}
			lua_pop(l,1);
		}
	}
	else if(_lua_isRecipientFilter(l,arg))
	{
		auto *rpc = _lua_RecipientFilter_check(l,arg);
		*rp = *rpc;
	}
	else
	{
		auto *hnd = Lua::CheckSPlayer(l,arg);
		if(Lua::CheckComponentHandle(l,*hnd) == false)
			return false;
		WVServerClient *session = (*hnd)->GetClientSession();
		if(session != NULL)
			rp->Add(session);
	}
	return true;
}

DLLSERVER int Lua_sv_net_Register(lua_State *l)
{
	if(!server->IsGameActive())
		return 0;
	Game *game = server->GetGameState();
	std::string identifier = luaL_checkstring(l,1);
	game->RegisterNetMessage(identifier);
	return 0;
}

DLLSERVER int Lua_sv_net_Broadcast(lua_State *l)
{
	auto protocol = static_cast<nwm::Protocol>(Lua::CheckInt(l,1));
	std::string identifier = luaL_checkstring(l,2);
	NetPacket *p = _lua_NetPacket_check(l,3);
	NetPacket packetNew;
	if(!NetIncludePacketID(server,identifier,*p,packetNew))
	{
		Con::csv<<"WARNING: Attempted to send unindexed lua net message: "<<identifier<<Con::endl;
		return 0;
	}
	switch(protocol)
	{
		case nwm::Protocol::TCP:
			server->BroadcastTCP("luanet",packetNew);
			break;
		case nwm::Protocol::UDP:
			server->BroadcastUDP("luanet",packetNew);
			break;
	}
	return 0;
}

DLLSERVER int Lua_sv_net_Send(lua_State *l)
{
	auto protocol = static_cast<nwm::Protocol>(Lua::CheckInt(l,1));
	std::string identifier = luaL_checkstring(l,2);
	NetPacket *p = _lua_NetPacket_check(l,3);
	NetPacket packetNew;
	if(!NetIncludePacketID(server,identifier,*p,packetNew))
	{
		Con::csv<<"WARNING: Attempted to send unindexed lua net message: "<<identifier<<Con::endl;
		return 0;
	}
	nwm::RecipientFilter rp;
	if(!GetRecipients(l,4,&rp))
		return 0;
	switch(protocol)
	{
		case nwm::Protocol::TCP:
			server->SendPacketTCP("luanet",packetNew,rp);
			break;
		case nwm::Protocol::UDP:
			server->SendPacketUDP("luanet",packetNew,rp);
			break;
	}
	return 0;
}

DLLSERVER int Lua_sv_net_Receive(lua_State *l)
{
	if(!server->IsGameActive())
		return 0;
	Game *game = server->GetGameState();
	std::string name = luaL_checkstring(l,1);
	luaL_checkfunction(l,2);
	int fc = lua_createreference(l,2);
	game->RegisterLuaNetMessage(name,fc);
	return 0;
}


DLLSERVER void NET_sv_luanet(WVServerClient *session,NetPacket packet) {server->HandleLuaNetPacket(session,packet);}
#pragma optimize("",on)
