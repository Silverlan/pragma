/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LUA_SCRIPT_WATCHER_H__
#define __S_LUA_SCRIPT_WATCHER_H__

#include "pragma/serverdefinitions.h"
#include "pragma/lua/lua_script_watcher.h"

class DLLSERVER SLuaDirectoryWatcherManager : public LuaDirectoryWatcherManager {
  protected:
	virtual void OnLuaFileChanged(const std::string &path) override;
  public:
	using LuaDirectoryWatcherManager::LuaDirectoryWatcherManager;
};

#endif
