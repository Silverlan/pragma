/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __GAME_SERVER_DATA_HPP__
#define __GAME_SERVER_DATA_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/engine_info.hpp>
#include <cinttypes>
#include <string>
#include <optional>
#include <sharedutils/util_version.h>

namespace util {class Library;};
namespace pragma::networking
{
	struct DLLNETWORK GameServerInfo
	{
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

#endif
