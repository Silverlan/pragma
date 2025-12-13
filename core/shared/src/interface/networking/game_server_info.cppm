// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:networking.game_server_info;

export import :engine.info;

export namespace pragma::networking {
	struct DLLNETWORK GameServerInfo {
		uint16_t port = 0u;
		util::Version version = {};
		std::string gameName = "";
		std::string gameDirectory = "";
		std::string gameMode = "";

		std::string name = "";
		uint32_t maxPlayers = 0u;
		uint32_t botCount = 0u;
		std::string mapName = "";
		bool passwordProtected = false;

		std::string networkLayerIdentifier = "";

		bool peer2peer = false;
		std::optional<uint64_t> steamId = {}; // Only if peer2peer

		uint16_t masterServerPort = engine_info::DEFAULT_QUERY_PORT;
	};
};
