// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "fsys/directory_watcher.h"
#include "stdafx_shared.h"
#include "pragma/lua/lua_script_watcher.h"
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/lua/util.hpp"
#include <scripting/lua/lua.hpp>
#include <sharedutils/scope_guard.h>
#include <sharedutils/util_file.h>
#include <luainterface.hpp>

module pragma.client;

import :scripting.lua.script_watcher;

//import pragma.scripting.lua;

LuaDirectoryWatcherManager::LuaDirectoryWatcherManager(Game *game) : m_game(game) { m_watcherManager = filemanager::create_directory_watcher_manager(); }

void LuaDirectoryWatcherManager::Poll()
{
	for(auto &watcher : m_watchers)
		watcher->Poll();
}

bool LuaDirectoryWatcherManager::IsLuaFile(const std::string &path, bool bAllowCompiled) const
{
	std::string ext;
	return (ufile::get_extension(path, &ext) == true && (ext == Lua::FILE_EXTENSION || (bAllowCompiled == true && ext == Lua::FILE_EXTENSION_PRECOMPILED))) ? true : false;
}

void LuaDirectoryWatcherManager::OnLuaFileChanged(const std::string &fName)
{
	if(IsLuaFile(fName, true) == false)
		return;
	// Ignore include cache (= all include files are reloaded)
	Lua::set_ignore_include_cache(true);
	auto sg = util::ScopeGuard(std::bind(Lua::set_ignore_include_cache, false)); // Reset once we're done

	auto splitPath = ufile::split_path(fName);
	if(splitPath.empty())
		return;
	if(splitPath.size() >= 2) {
		auto nwStateDirName = m_game->GetLuaNetworkDirectoryName();
		const auto fCheckType = [this, &splitPath, &fName, &nwStateDirName](const std::string &typeName) -> bool {
			if(splitPath.size() < 2 || ustring::compare(splitPath.at(0), typeName, false) == false)
				return false;
			if(splitPath.size() == 2 || (splitPath.size() == 3 && ustring::compare(splitPath.at(1), nwStateDirName, false) == true)) {
				// This should be a Lua-entity defined in a single script, without its own directory
				m_game->LoadLuaEntity(ufile::to_path(splitPath, 0, splitPath.size() - 1));
				return true;
			}
			if(ustring::compare<std::string>(splitPath.at(1), "components", false)) {
				m_game->LoadLuaComponent(ufile::to_path(splitPath, 0, 2));
				return true;
			}
			// Deprecated
			//m_game->LoadLuaEntity(ufile::to_path(splitPath,0,1));
			return true;
		};
		for(auto &dirName : m_game->GetLuaEntityDirectories()) {
			if(fCheckType(dirName))
				return;
		}
	}
	// Game Mode
	auto *info = m_game->GetGameMode();
	if(info != nullptr && splitPath.at(0) == "gamemodes" && splitPath.size() >= 2) {
		if(ustring::compare(splitPath.at(1), info->id, false)) // Is this the current gamemode?
		{
			m_game->ReloadGameModeScripts();
			return;
		}
	}

	// Probably a regular Lua file; Check if it was included previously, and if so, reload it
	auto &luaInterface = m_game->GetLuaInterface();
	auto &includeCache = luaInterface.GetIncludeCache();
	if(includeCache.Contains(fName)) {
		auto res = pragma::scripting::lua::include(luaInterface.GetState(), fName, pragma::scripting::lua::IncludeFlags::IgnoreGlobalCache);
		if(res.statusCode != Lua::StatusCode::Ok)
			pragma::scripting::lua::submit_error(luaInterface.GetState(), res.errorMessage);
		return;
	}
}

bool LuaDirectoryWatcherManager::MountDirectory(const std::string &path, bool stripBaseBath)
{
	try {
		auto watchFlags = DirectoryWatcherCallback::WatchFlags::WatchSubDirectories;
		auto basePath = util::DirPath(path);
		m_watchers.push_back(std::make_shared<DirectoryWatcherCallback>(
		  path,
		  [this, basePath = std::move(basePath)](const std::string &fName) {
			  auto relName = util::FilePath(fName);
			  relName.MakeRelative(basePath);
			  OnLuaFileChanged(relName.GetString());
		  },
		  watchFlags, m_watcherManager.get()));
		return true;
	}
	catch(const DirectoryWatcher::ConstructException &) {
		return false;
	}
}
