/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan */

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
