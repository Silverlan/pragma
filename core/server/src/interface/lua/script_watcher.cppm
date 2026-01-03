// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.script_watcher;

export import pragma.shared;

export class DLLSERVER SLuaDirectoryWatcherManager : public LuaDirectoryWatcherManager {
  protected:
	virtual void OnLuaFileChanged(const std::string &path) override;
  public:
	using LuaDirectoryWatcherManager::LuaDirectoryWatcherManager;
};
