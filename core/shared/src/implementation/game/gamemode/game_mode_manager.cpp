// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.game_mode_manager;

std::unordered_map<std::string, pragma::game::GameModeInfo> pragma::game::GameModeManager::m_gameModes;
void pragma::game::GameModeManager::Initialize()
{
	std::vector<std::string> directories;
	std::string dir = "lua/gamemodes/components/";
	fs::find_files((dir + "*"), nullptr, &directories);
	for(auto it = directories.begin(); it != directories.end(); ++it) {
		auto path = dir;
		path += *it;
		path += "/info.udm";
		std::string err;
		auto udmData = util::load_udm_asset(path, &err);
		if(udmData == nullptr)
			continue;
		auto udm = udmData->GetAssetData().GetData();
		for(auto &pair : udm.ElIt()) {
			if(!pair.property.IsType(udm::Type::Element))
				continue;
			auto udmGm = pair.property;
			if(udmGm["hidden"].ToValue(false))
				continue;
			std::string id {pair.key};
			string::to_lower(id);
			auto it = m_gameModes.find(id);
			if(it != m_gameModes.end())
				continue;
			GameModeInfo gmInfo {};
			gmInfo.id = id;
			udmGm["component_name"](gmInfo.component_name);
			udmGm["name"](gmInfo.name);
			udmGm["author"](gmInfo.author);
			udmGm["initial_map"](gmInfo.initial_map);
			for(auto &pair : udmGm["mount_priorities"].ElIt())
				gmInfo.gameMountPriorities[std::string {pair.key}] = pair.property.ToValue<udm::Int32>(0);

			std::string version {};
			udmGm["version"](version);
			gmInfo.version = util::string_to_version(version);
			m_gameModes[id] = std::move(gmInfo);
		}
	}
}

std::unordered_map<std::string, pragma::game::GameModeInfo> &pragma::game::GameModeManager::GetGameModes() { return m_gameModes; }
pragma::game::GameModeInfo *pragma::game::GameModeManager::GetGameModeInfo(const std::string &id)
{
	auto it = m_gameModes.find(id);
	if(it == m_gameModes.end())
		return nullptr;
	return &it->second;
}
