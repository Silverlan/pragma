#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/lnetmessages.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/classes/ldef_netpacket.h"
#include <networkmanager/interface/nwm_manager.hpp>

void CGame::HandleLuaNetPacket(NetPacket &packet)
{
	unsigned int ID = packet->Read<unsigned int>();
	std::unordered_map<std::string,int>::iterator j;
	if(ID == 0)
		return;
	std::string *ident = GetNetMessageIdentifier(ID);
	if(ident == NULL)
		return;
	std::unordered_map<std::string,int>::iterator i = m_luaNetMessages.find(*ident);
	if(i == m_luaNetMessages.end())
	{
		Con::ccl<<"WARNING: Unhandled lua net message: "<<*ident<<Con::endl;
		return;
	}
	ProtectedLuaCall([this,&i,&packet](lua_State*) {
		lua_rawgeti(GetLuaState(),LUA_REGISTRYINDEX,i->second);

		luabind::object(GetLuaState(),packet).push(GetLuaState());
		return Lua::StatusCode::Ok;
	},0);
}

////////////////////////////

extern ClientState *client;
int Lua_cl_net_Send(lua_State *l)
{
	auto protocol = static_cast<nwm::Protocol>(Lua::CheckInt(l,1));
	std::string identifier = luaL_checkstring(l,2);
	NetPacket *p = _lua_NetPacket_check(l,3);
	NetPacket packetNew;
	if(!NetIncludePacketID(client,identifier,*p,packetNew))
	{
		Con::ccl<<"WARNING: Attempted to send unindexed lua net message: "<<identifier<<Con::endl;
		return 0;
	}
	switch(protocol)
	{
		case nwm::Protocol::TCP:
			client->SendPacketTCP("luanet",packetNew);
			break;
		case nwm::Protocol::UDP:
			client->SendPacketUDP("luanet",packetNew);
			break;
	}
	return 0;
}

int Lua_cl_net_Receive(lua_State *l)
{
	if(!client->IsGameActive())
		return 0;
	Game *game = client->GetGameState();
	std::string name = luaL_checkstring(l,1);
	luaL_checkfunction(l,2);
	int fc = lua_createreference(l,2);
	game->RegisterLuaNetMessage(name,fc);
	return 0;
}

DLLCLIENT void NET_cl_luanet(NetPacket packet) {client->HandleLuaNetPacket(packet);}

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
	static_cast<CGame*>(client->GetGameState())->OnReceivedRegisterNetEvent(packet);
}
