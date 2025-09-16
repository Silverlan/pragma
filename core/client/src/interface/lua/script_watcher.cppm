// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/lua_script_watcher.h"

export module pragma.client.scripting.lua.script_watcher;

export class DLLCLIENT CLuaDirectoryWatcherManager : public LuaDirectoryWatcherManager {
  protected:
	virtual void OnLuaFileChanged(const std::string &path) override;
  public:
	using LuaDirectoryWatcherManager::LuaDirectoryWatcherManager;
};
