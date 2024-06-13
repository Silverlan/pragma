/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ENGINE_INFO_HPP__
#define __ENGINE_INFO_HPP__

#include "pragma/definitions.h"
#include <string>
#include <vector>
#include <optional>

namespace engine_info {
	DLLNETWORK std::string get_identifier();
	DLLNETWORK std::string get_name();
	DLLNETWORK std::string get_executable_name();
	DLLNETWORK std::string get_server_executable_name();
	DLLNETWORK std::string get_author_mail_address();
	DLLNETWORK std::string get_website_url();
	DLLNETWORK std::string get_wiki_url();
	DLLNETWORK std::string get_discord_url();
	DLLNETWORK std::string get_github_url();
	DLLNETWORK uint32_t get_steam_app_id();

	constexpr uint16_t DEFAULT_SERVER_PORT = 29150;
	constexpr uint16_t DEFAULT_QUERY_PORT = 29155;
	constexpr uint16_t DEFAULT_AUTH_PORT = 8766;

	// Returns the extensions for the supported audio formats
	DLLNETWORK const std::vector<std::string> get_supported_audio_formats();

	struct DLLNETWORK GitInfo {
		std::string ref;
		std::string commitSha;
		std::string dateTime;
	};
	DLLNETWORK std::optional<GitInfo> get_git_info();
};

#endif
