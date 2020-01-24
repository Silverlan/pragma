#ifndef __ENGINE_INFO_HPP__
#define __ENGINE_INFO_HPP__

#include "pragma/definitions.h"
#include <string>
#include <vector>

namespace engine_info
{
	DLLENGINE std::string get_identifier();
	DLLENGINE std::string get_name();
	DLLENGINE std::string get_executable_name();
	DLLENGINE std::string get_server_executable_name();
	DLLENGINE std::string get_author_mail_address();
	DLLENGINE std::string get_website_url();
	DLLENGINE std::string get_modding_hub_url();
	DLLENGINE std::string get_wiki_url();
	DLLENGINE std::string get_forums_url();
	DLLENGINE std::string get_patreon_url();
	DLLENGINE std::vector<std::string> get_patrons();
	DLLENGINE uint32_t get_total_patron_count();
	DLLENGINE std::string get_discord_url();
	DLLENGINE std::string get_twitter_url();
	DLLENGINE std::string get_reddit_url();
	DLLENGINE uint32_t get_steam_app_id();

	constexpr uint16_t DEFAULT_SERVER_PORT = 29150;
	constexpr uint16_t DEFAULT_QUERY_PORT = 29155;
	constexpr uint16_t DEFAULT_AUTH_PORT = 8766;

	// Returns the extensions for the supported audio formats
	DLLENGINE const std::vector<std::string> get_supported_audio_formats();
};

#endif
