// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <sharedutils/util_string.h>
#include "fsys/filesystem.h"

module pragma.shared;

import :engine.info;

#define PRAGMA_ENGINE_NAME "Pragma"
#define PRAGMA_AUTHOR_EMAIL "mail@pragma-engine.com"
#define PRAGMA_WEBSITE_URL "https://pragma-engine.com"
#define PRAGMA_WIKI_URL "https://wiki.pragma-engine.com/"
#define PRAGMA_GITHUB_URL "https://github.com/Silverlan/pragma"
#define PRAGMA_DISCORD_URL "https://discord.gg/Ck5BcCz"
#define PRAGMA_STEAM_APP_ID 947'100

extern std::optional<std::string> g_customTitle;

std::string engine_info::get_program_title()
{
	if(g_customTitle)
		return *g_customTitle;
	return get_name();
}

std::string engine_info::get_identifier()
{
	auto name = get_name();
	ustring::to_lower(name);
	return name;
}

std::string engine_info::get_name() { return PRAGMA_ENGINE_NAME; }

extern util::Path g_programIcon;
util::Path engine_info::get_icon_path() { return g_programIcon; }

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
std::string engine_info::get_author_mail_address() { return PRAGMA_AUTHOR_EMAIL; }
std::string engine_info::get_website_url() { return PRAGMA_WEBSITE_URL; }
std::string engine_info::get_wiki_url() { return PRAGMA_WIKI_URL; }
std::string engine_info::get_discord_url() { return PRAGMA_DISCORD_URL; }
std::string engine_info::get_github_url() { return PRAGMA_GITHUB_URL; }
uint32_t engine_info::get_steam_app_id() { return PRAGMA_STEAM_APP_ID; }

const std::vector<std::string> engine_info::get_supported_audio_formats()
{
	// TODO: Merge this with asset library
	return {"ogg", "wav", "mp3"};
}

std::optional<engine_info::GitInfo> engine_info::get_git_info()
{
	auto f = filemanager::open_file("git_info.txt", filemanager::FileMode::Read, nullptr, fsys::SearchFlags::Local | fsys::SearchFlags::NoMounts);
	if(f == nullptr)
		return {};
	GitInfo gitInfo {};
	std::vector<std::string> lines;
	ustring::explode(f->ReadString(), "\n", lines);
	for(auto &l : lines) {
		auto sep = l.find(':');
		if(sep == std::string::npos)
			continue;
		auto id = l.substr(0, sep);
		auto val = l.substr(sep + 1);
		ustring::remove_whitespace(val);
		if(id.empty() || val.empty())
			continue;
		if(id == "ref")
			gitInfo.ref = val;
		else if(id == "commit")
			gitInfo.commitSha = val;
		else if(id == "build")
			gitInfo.dateTime = val;
	}
	if(gitInfo.commitSha.empty())
		return {};
	return gitInfo;
}
