/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/game/gamemode/gamemodemanager.h"
#include <fsys/filesystem.h>
#include <sharedutils/util.h>
#include <sharedutils/scope_guard.h>

std::unordered_map<std::string,GameModeInfo> GameModeManager::m_gameModes;
void GameModeManager::Initialize()
{
	std::vector<std::string> directories;
	std::string dir = "lua/gamemodes/";
	FileManager::FindFiles((dir +"*").c_str(),nullptr,&directories);
	for(auto it=directories.begin();it!=directories.end();++it)
	{
		auto path = dir;
		path += *it;
		path += "/info.txt";
		auto f = FileManager::OpenFile(path.c_str(),"r");
		if(f != nullptr)
		{
			auto root = ds::System::ReadData(f);
			if(root != nullptr)
			{
				auto id = *it;
				ustring::to_lower(id);
				auto it = m_gameModes.find(id);
				if(it != m_gameModes.end())
					continue;
				auto block = root->GetBlock(id,0);
				if(block != nullptr)
				{
					auto res = m_gameModes.insert(std::make_pair(id,GameModeInfo()));
					if(res.second == true)
					{
						auto &info = res.first->second;
						info.id = id;
						info.class_name = block->GetString("class");
						info.name = block->GetString("name");
						info.author = block->GetString("author");
						info.version = util::string_to_version(block->GetString("version"));
					}
				}
			}
		}
	}
}

std::unordered_map<std::string,GameModeInfo> &GameModeManager::GetGameModes() {return m_gameModes;}
GameModeInfo *GameModeManager::GetGameModeInfo(const std::string &id)
{
	auto it = m_gameModes.find(id);
	if(it == m_gameModes.end())
		return nullptr;
	return &it->second;
}