/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/lua_script_watcher.h"
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/lua/libraries/lutil.h"
#include <sharedutils/scope_guard.h>
#include <sharedutils/util_file.h>
#include <luainterface.hpp>

LuaDirectoryWatcherManager::LuaDirectoryWatcherManager(Game *game)
	: m_game(game)
{}

void LuaDirectoryWatcherManager::Poll()
{
	for(auto &watcher : m_watchers)
		watcher->Poll();
}

bool LuaDirectoryWatcherManager::IsLuaFile(const std::string &path,bool bAllowCompiled) const
{
	std::string ext;
	return (ufile::get_extension(path,&ext) == true && (ext == "lua" || (bAllowCompiled == true && ext == "clua"))) ? true : false;
}

void LuaDirectoryWatcherManager::OnLuaFileChanged(const std::string &fName)
{
	if(IsLuaFile(fName,true) == false)
		return;
	// Ignore include cache (= all include files are reloaded)
	Lua::set_ignore_include_cache(true);
	auto sg = ScopeGuard(std::bind(Lua::set_ignore_include_cache,false)); // Reset once we're done

	auto splitPath = ufile::split_path(fName);
	if(splitPath.empty())
		return;
	if(splitPath.size() >= 2)
	{
		auto nwStateDirName = m_game->GetLuaNetworkDirectoryName();
		const auto fCheckType = [this,&splitPath,&fName,&nwStateDirName](const std::string &typeName) -> bool {
			if(splitPath.size() < 2 || ustring::compare(splitPath.at(0),typeName,false) == false)
				return false;
			if(splitPath.size() == 2 || (splitPath.size() == 3 && ustring::compare(splitPath.at(1),nwStateDirName,false) == true))
			{
				// This should be a Lua-entity defined in a single script, without its own directory
				m_game->LoadLuaEntity(ufile::to_path(splitPath,0,splitPath.size() -1));
				return true;
			}
			if(ustring::compare(splitPath.at(1),"components",false))
			{
				m_game->LoadLuaComponent(ufile::to_path(splitPath,0,2));
				return true;
			}
			// Deprecated
			//m_game->LoadLuaEntity(ufile::to_path(splitPath,0,1));
			return true;
		};
		for(auto &dirName : m_game->GetLuaEntityDirectories())
		{
			if(fCheckType(dirName))
				return;
		}
	}
	// Game Mode
	auto *info = m_game->GetGameMode();
	if(info != nullptr && splitPath.at(0) == "gamemodes" && splitPath.size() >= 2)
	{
		if(ustring::compare(splitPath.at(1),info->id,false)) // Is this the current gamemode?
		{
			m_game->ReloadGameModeScripts();
			return;
		}
	}

	// Probably a regular Lua file; Check if it was included previously, and if so, reload it
	auto &includeCache = m_game->GetLuaInterface().GetIncludeCache();
	auto it = std::find_if(includeCache.begin(),includeCache.end(),[&fName](const std::string &cachedPath) {
		return FileManager::ComparePath(fName,cachedPath);
	});
	if(it != includeCache.end())
	{
		auto lpath = *it;
		m_game->ExecuteLuaFile(lpath);
		return;
	}
}

bool LuaDirectoryWatcherManager::MountDirectory(const std::string &path,bool bAbsolutePath)
{
	try
	{
		auto watchFlags = DirectoryWatcherCallback::WatchFlags::WatchSubDirectories;
		if(bAbsolutePath)
			watchFlags |= DirectoryWatcherCallback::WatchFlags::AbsolutePath;
		m_watchers.push_back(std::make_shared<DirectoryWatcherCallback>(path,[this](const std::string &fName) {
			OnLuaFileChanged(fName);
		},watchFlags));
		return true;
	}
	catch(const DirectoryWatcher::ConstructException&)
	{
		return false;
	}
}
