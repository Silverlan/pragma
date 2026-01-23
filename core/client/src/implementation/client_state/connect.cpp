// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :client_state;

#undef GetMessage

void pragma::ClientState::Connect(std::string ip, std::string sport)
{
	EndGame();
	// "localhost" is ALWAYS single-player!
	auto localGame = (ip == "localhost");
	if(localGame)
		ip = "127.0.0.1";
	auto &lastConnection = GetLastConnectionInfo();
	lastConnection = {};
	auto port = static_cast<uint16_t>(util::to_int(sport));
	lastConnection.address = {ip, port};
#ifdef DEBUG_SOCKET
	Con::CCL << "Connecting to " << ip << ":" << port << "..." << Con::endl;
#endif
	Disconnect();
	InitializeGameClient(localGame);
	if(m_client == nullptr)
		return;
	networking::Error err;
	if(m_client->Connect(ip, port, err) == false)
		Con::CWAR << "Unable to connect to '" << ip << ":" << port << "': " << err.GetMessage() << "!" << Con::endl;
}

void pragma::ClientState::Connect(uint64_t steamId)
{
	EndGame();
	auto &lastConnection = GetLastConnectionInfo();
	lastConnection = {};
	lastConnection.steamId = steamId;
#ifdef DEBUG_SOCKET
	Con::CCL << "Connecting to host with Steam ID " << steamId << "..." << Con::endl;
#endif
	Disconnect();
	InitializeGameClient(false);
	if(m_client == nullptr)
		return;
	networking::Error err;
	if(m_client->Connect(steamId, err) == false)
		Con::CWAR << "Unable to connect to host with Steam ID " << steamId << ": " << err.GetMessage() << "!" << Con::endl;
}
