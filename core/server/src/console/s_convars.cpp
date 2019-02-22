#include "stdafx_server.h"
#include <pragma/serverstate/serverstate.h>
/*
void Engine::OnClientConVarChanged(Player *pl,std::string cvar,std::string value)
{
	if(cvar == "playername")
	{
		pl->m_playerName = value;
		NetPacket p;
		p.WriteEntity(pl);
		p.WriteString(value);
		SendTCPMessageToClients("player_changedname",&p);
	}
}
*/ // WEAVETODO