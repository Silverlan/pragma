/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_engine.h"
#include "pragma/engine_info.hpp"
#include <sharedutils/util_string.h>

#define PRAGMA_ENGINE_NAME "Pragma"
#define PRAGMA_AUTHOR_EMAIL "mail@pragma-engine.com"
#define PRAGMA_WEBSITE_URL "https://pragma-engine.com"
#define PRAGMA_FORUMS_URL "https://forums.pragma-engine.com/"
#define PRAGMA_MODDING_HUB_URL "https://mods.pragma-engine.com/"
#define PRAGMA_WIKI_URL "https://wiki.pragma-engine.com/"
#define PRAGMA_PATREON_URL "https://www.patreon.com/silverlan"
#define PRAGMA_TWITTER_URL "https://twitter.com/SilverlanPFM"
#define PRAGMA_REDDIT_URL "https://www.reddit.com/r/PRFM/"
#define PRAGMA_DISCORD_URL "https://discord.gg/Ck5BcCz"
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
std::vector<std::string> engine_info::get_patrons()
{
	return {
		"PalmliX",
		"Dark Dreams",
		"Borland",
		"Archangel470",
		"The Tveit",
		"Netfuta",
		"Meehoyminoy",
		"Alex Roman Nunez (EIREXE)",
		"Alkaza1996",
		"ArclightReflex",
		"Bertbert",
		"Brandon K.",
		"Luminious",
		"Melvin Hood",
		"Siejax",
		"SquarePeg",
		"_HappyGoLucky",
		"Gnomp",
		"RED_EYE",

		"Ruthoranium",
		"CodaAnim",
		"Googleygareth",
		"Leeroy Plambeck",
		"Roach",
		"Artur 'atrblizzard' Tacacs",
		"branpurn",
		"Elektrospy",
		"Syrr",
		"Tanooki_Jon",
		"Xrdexst",
		"Noah The Troll-Hunting Christian Spy",
		"Boris \"SunRade\" Serousov",
		"Prof. Purble",
		"Antoine Guillo",
		"ff7sfm",
		"Kybrid96",
		"PixMedia"
	};
}
uint32_t engine_info::get_total_patron_count() {return 56;}
std::string engine_info::get_discord_url() {return PRAGMA_DISCORD_URL;}
std::string engine_info::get_twitter_url() {return PRAGMA_TWITTER_URL;}
std::string engine_info::get_reddit_url() {return PRAGMA_REDDIT_URL;}
uint32_t engine_info::get_steam_app_id() {return PRAGMA_STEAM_APP_ID;}

const std::vector<std::string> engine_info::get_supported_audio_formats()
{
	return {
		"ogg","wav","mp3"
	};
}
