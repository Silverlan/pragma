// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :server_state;

import :entities;
import :game;
import :networking;

#undef GetMessage

void pragma::ServerState::OnMasterServerRegistered(bool b, std::string reason)
{
	if(b == false) {
		m_tNextWMSConnect = std::chrono::steady_clock::now();
		Con::CWAR << "Unable to connect to master server (" << reason << ")" << Con::endl;
	}
}

void pragma::ServerState::RegisterServerInfo()
{
	if(m_server == nullptr || SGame::Get() == nullptr)
		return;
	if(SGame::Get() == nullptr)
		return;
	auto *gameMode = SGame::Get()->GetGameMode();
	m_serverData = WMServerData();
	m_serverData.name = GetConVarString("sv_servername");
	m_serverData.map = SGame::Get()->GetMapName();
	if(gameMode != nullptr)
		m_serverData.gameMode = gameMode->name;
	auto port = m_server->GetHostPort();
	auto password = GetConVarString("password");
	m_serverData.players = SGame::Get()->GetPlayerCount();
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

void pragma::ServerState::SetServerInterface(std::unique_ptr<networking::IServer> iserver)
{
	// TODO: Remove this file
	//CloseServer();
	//m_server = std::move(iserver);
}

void pragma::ServerState::StartServer(bool singlePlayer)
{
	CloseServer();
	InitializeGameServer(singlePlayer);
	if(m_server == nullptr)
		return;

	if(IsGameActive())
		RegisterServerInfo();
}

void pragma::ServerState::CloseServer()
{
	if(m_server == nullptr)
		return;
	networking::Error err;
	if(m_server->Shutdown(err) == true)
		return;
	spdlog::error("Unable to shut down server: ", err.GetMessage());
}
