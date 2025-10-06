// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_LUA_SCRIPT_WATCHER_H__
#define __S_LUA_SCRIPT_WATCHER_H__

#include "pragma/serverdefinitions.h"

class DLLSERVER SLuaDirectoryWatcherManager : public LuaDirectoryWatcherManager {
  protected:
	virtual void OnLuaFileChanged(const std::string &path) override;
  public:
	using LuaDirectoryWatcherManager::LuaDirectoryWatcherManager;
};

#endif
