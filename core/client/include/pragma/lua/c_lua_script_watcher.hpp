// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LUA_SCRIPT_WATCHER_HPP__
#define __C_LUA_SCRIPT_WATCHER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/lua_script_watcher.h"

class DLLCLIENT CLuaDirectoryWatcherManager : public LuaDirectoryWatcherManager {
  protected:
	virtual void OnLuaFileChanged(const std::string &path) override;
  public:
	using LuaDirectoryWatcherManager::LuaDirectoryWatcherManager;
};

#endif
