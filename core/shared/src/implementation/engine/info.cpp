// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :engine.info;

struct EngineInfo {
	EngineInfo();
	void Load();
	std::optional<std::string> contact;
	std::optional<std::string> website;
	std::optional<std::string> wiki;
	std::optional<std::string> github;
	std::optional<std::string> discord;
	std::optional<std::string> forum;

	std::optional<uint32_t> steamAppId;
};

EngineInfo::EngineInfo() {}

void EngineInfo::Load()
{
	auto engineInfo = udm::Data::Load("scripts/engine_info.udm");
	if(!engineInfo)
		return;
	auto data = engineInfo->GetAssetData().GetData()["engine_info"];
	contact = data["contact"].ToValue<udm::String>();
	website = data["website"].ToValue<udm::String>();
	wiki = data["wiki"].ToValue<udm::String>();
	github = data["github"].ToValue<udm::String>();
	discord = data["discord"].ToValue<udm::String>();
	forum = data["forum"].ToValue<udm::String>();
	steamAppId = data["steam_app_id"].ToValue<udm::UInt32>();
}

static EngineInfo g_engineInfo {};
static EngineInfo &get_engine_info()
{
	static std::once_flag flag;
	std::call_once(flag, []() { g_engineInfo.Load(); });
	return g_engineInfo;
}

constexpr std::string_view PRAGMA_ENGINE_NAME = "Pragma";

std::string pragma::engine_info::get_program_title()
{
	auto customTitle = pragma::get_engine()->GetLaunchSettings().Get<udm::String>("title");
	if(customTitle)
		return *customTitle;
	return get_name();
}

std::string pragma::engine_info::get_identifier()
{
	auto name = get_name();
	string::to_lower(name);
	return name;
}

std::string pragma::engine_info::get_name() { return std::string {PRAGMA_ENGINE_NAME}; }

pragma::util::Path pragma::engine_info::get_icon_path()
{
	auto iconPath = get_engine()->GetLaunchSettings().Get<udm::String>("icon");
	return util::FilePath(iconPath ? *iconPath : "");
}

std::string pragma::engine_info::get_executable_name()
{
	auto exeName = get_identifier();
	exeName += ".exe";
	return exeName;
}
std::string pragma::engine_info::get_server_executable_name()
{
	auto exeName = get_identifier();
	exeName += "_server.exe";
	return exeName;
}
std::optional<std::string> pragma::engine_info::get_author_mail_address() { return get_engine_info().contact; }
std::optional<std::string> pragma::engine_info::get_website_url() { return get_engine_info().website; }
std::optional<std::string> pragma::engine_info::get_wiki_url() { return get_engine_info().wiki; }
std::optional<std::string> pragma::engine_info::get_discord_url() { return get_engine_info().discord; }
std::optional<std::string> pragma::engine_info::get_github_url() { return get_engine_info().github; }
std::optional<std::string> pragma::engine_info::get_forum_url() { return get_engine_info().forum; }
std::optional<uint32_t> pragma::engine_info::get_steam_app_id() { return get_engine_info().steamAppId; }

const std::vector<std::string> pragma::engine_info::get_supported_audio_formats()
{
	// TODO: Merge this with asset library
	return {"ogg", "wav", "mp3"};
}

std::optional<pragma::engine_info::GitInfo> pragma::engine_info::get_git_info()
{
	auto f = fs::open_file("git_info.txt", fs::FileMode::Read, nullptr, fs::SearchFlags::Local | fs::SearchFlags::NoMounts);
	if(f == nullptr)
		return {};
	GitInfo gitInfo {};
	std::vector<std::string> lines;
	string::explode(f->ReadString(), "\n", lines);
	for(auto &l : lines) {
		auto sep = l.find(':');
		if(sep == std::string::npos)
			continue;
		auto id = l.substr(0, sep);
		auto val = l.substr(sep + 1);
		string::remove_whitespace(val);
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
