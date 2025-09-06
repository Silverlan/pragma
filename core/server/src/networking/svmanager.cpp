// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include <pragma/engine.h>
#include "pragma/networking/iserver.hpp"
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/networking/standard_server.hpp"
#include <pragma/networking/game_server_data.hpp>
#include <pragma/networking/enums.hpp>
#include <wms_shared.h>
#include <pragma/engine_version.h>
#include <pragma/networking/error.hpp>
#include <networkmanager/nwm_error_handle.h>
#include <pragma/logging.hpp>

import pragma.server.entities;
import pragma.server.game;
import pragma.server.networking;
import pragma.server.server_state;

extern DLLNETWORK Engine *engine;
extern SGame *s_game;
void ServerState::OnMasterServerRegistered(bool b, std::string reason)
{
	if(b == false) {
		m_tNextWMSConnect = std::chrono::steady_clock::now();
		Con::cwar << "Unable to connect to master server (" << reason << ")" << Con::endl;
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
	auto port = m_server->GetHostPort();
	auto password = GetConVarString("password");
	m_serverData.players = s_game->GetPlayerCount();
	m_serverData.tcpPort = port.has_value() ? *port : 0;
	m_serverData.udpPort = port.has_value() ? *port : 0;
	m_serverData.engineVersion = get_engine_version();
	m_serverData.maxPlayers = GetConVarInt("sv_maxplayers");
	m_serverData.password = password.empty() == false;
	//m_serverData.bots

	if(m_serverReg) {
		auto &serverInfo = m_serverReg->GetServerInfo();
		serverInfo.name = m_serverData.name;
		serverInfo.mapName = m_serverData.map;
		serverInfo.gameMode = m_serverData.gameMode;
		serverInfo.maxPlayers = m_serverData.maxPlayers;
		serverInfo.passwordProtected = m_serverData.password;
	}
}

void ServerState::SetServerInterface(std::unique_ptr<pragma::networking::IServer> iserver)
{
	// TODO: Remove this file
	//CloseServer();
	//m_server = std::move(iserver);
}

void ServerState::StartServer(bool singlePlayer)
{
	CloseServer();
	InitializeGameServer(singlePlayer);
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
	spdlog::error("Unable to shut down server: ", err.GetMessage());
}

/////////////////////////////////

DLLSERVER void CMD_startserver(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv) { engine->StartServer(false); }
DLLSERVER void CMD_closeserver(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { engine->CloseServer(); }
