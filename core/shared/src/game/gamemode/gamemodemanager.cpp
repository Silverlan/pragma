/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/util/util_game.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util.h>
#include <sharedutils/scope_guard.h>
#include <udm.hpp>

std::unordered_map<std::string, GameModeInfo> GameModeManager::m_gameModes;
void GameModeManager::Initialize()
{
	std::vector<std::string> directories;
	std::string dir = "lua/gamemodes/components/";
	FileManager::FindFiles((dir + "*").c_str(), nullptr, &directories);
	for(auto it = directories.begin(); it != directories.end(); ++it) {
		auto path = dir;
		path += *it;
		path += "/info.udm";
		std::string err;
		auto udmData = ::util::load_udm_asset(path, &err);
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
			ustring::to_lower(id);
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

std::unordered_map<std::string, GameModeInfo> &GameModeManager::GetGameModes() { return m_gameModes; }
GameModeInfo *GameModeManager::GetGameModeInfo(const std::string &id)
{
	auto it = m_gameModes.find(id);
	if(it == m_gameModes.end())
		return nullptr;
	return &it->second;
}
