#include "stdafx_server.h"
#include <pragma/serverstate/serverstate.h>
#include <pragma/engine.h>
#include "pragma/entities/player.h"
#include "pragma/networking/iserver.hpp"
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/networking/standard_server.hpp"
#include <pragma/networking/enums.hpp>
#include <wms_shared.h>
#include <pragma/engine_version.h>
#include <pragma/networking/error.hpp>
#include <networkmanager/nwm_error_handle.h>

extern DLLENGINE Engine *engine;
extern DLLSERVER SGame *s_game;
void ServerState::OnMasterServerRegistered(bool b,std::string reason)
{
	if(b == false)
	{
		m_tNextWMSConnect = std::chrono::high_resolution_clock::now();
		Con::cwar<<"WARNING: Unable to connect to master server ("<<reason<<")"<<Con::endl;
	}
}

void ServerState::RegisterServerInfo()
{
	if(m_server == nullptr || s_game == nullptr)
		return;
	if(s_game == nullptr)
		return;
	auto *gameMode = s_game->GetGameMode();
	m_serverData = WMServerData();
	m_serverData.name = GetConVarString("sv_servername");
	m_serverData.map = s_game->GetMapName();
	if(gameMode != nullptr)
		m_serverData.gameMode = gameMode->name;
	auto tcpPort = m_server->GetLocalTCPPort();
	auto udpPort = m_server->GetLocalUDPPort();
	m_serverData.players = s_game->GetPlayerCount();
	m_serverData.tcpPort = tcpPort.has_value() ? *tcpPort : 0;
	m_serverData.udpPort = udpPort.has_value() ? *udpPort : 0;
	m_serverData.engineVersion = get_engine_version();
	//m_serverData.maxPlayers
	//m_serverData.password
	//m_serverData.bots
}

void ServerState::SetServerInterface(std::unique_ptr<pragma::networking::IServer> iserver)
{
	// TODO: Remove this file
	//CloseServer();
	//m_server = std::move(iserver);
}

void ServerState::StartServer()
{
	CloseServer();
	InitializeGameServer();
	if(m_server == nullptr)
		return;

	if(IsGameActive())
		RegisterServerInfo();
}

void ServerState::CloseServer()
{
	if(m_server == nullptr)
		return;
	pragma::networking::Error err;
	if(m_server->Shutdown(err) == true)
		return;
	Con::cerr<<"ERROR: Unable to shut down server: "<<err.GetMessage()<<Con::endl;
}

/////////////////////////////////

DLLSERVER void CMD_startserver(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	if(argv.empty())
	{
		engine->StartServer();
		return;
	}
	engine->StartServer(); // atoi(argv[0].c_str())); // WVTODO
}

DLLSERVER void CMD_closeserver(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&) {engine->CloseServer();}
