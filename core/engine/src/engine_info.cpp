#include "stdafx_engine.h"
#include "pragma/engine_info.hpp"
#include <sharedutils/util_string.h>

#define PRAGMA_ENGINE_NAME "Pragma"
#define PRAGMA_AUTHOR_EMAIL "mail@pragma-engine.com"
#define PRAGMA_WEBSITE_URL "https://pragma-engine.com"
#define PRAGMA_FORUMS_URL "https://forums.pragma-engine.com/"
#define PRAGMA_MODDING_HUB_URL "https://mods.pragma-engine.com/"
#define PRAGMA_WIKI_URL "https://wiki.pragma-engine.com/"
#define PRAGMA_PATREON_URL "https://www.patreon.com/pragma"
#define PRAGMA_STEAM_APP_ID 947'100

std::string engine_info::get_identifier()
{
	auto name = get_name();
	ustring::to_lower(name);
	return name;
}

std::string engine_info::get_name() {return PRAGMA_ENGINE_NAME;}

std::string engine_info::get_executable_name()
{
	auto exeName = get_identifier();
	exeName += ".exe";
	return exeName;
}
std::string engine_info::get_server_executable_name()
{
	auto exeName = get_identifier();
	exeName += "_server.exe";
	return exeName;
}
std::string engine_info::get_author_mail_address() {return PRAGMA_AUTHOR_EMAIL;}
std::string engine_info::get_website_url() {return PRAGMA_WEBSITE_URL;}
std::string engine_info::get_modding_hub_url() {return PRAGMA_MODDING_HUB_URL;}
std::string engine_info::get_wiki_url() {return PRAGMA_WIKI_URL;}
std::string engine_info::get_forums_url() {return PRAGMA_FORUMS_URL;}
std::string engine_info::get_patreon_url() {return PRAGMA_PATREON_URL;}
uint32_t engine_info::get_steam_app_id() {return PRAGMA_STEAM_APP_ID;}

const std::vector<std::string> engine_info::get_supported_audio_formats()
{
	return {
		"ogg","wav","mp3"
	};
}
