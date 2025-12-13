// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.script_watcher;

export import :types;
import pragma.filesystem;

export {
	class DLLNETWORK LuaDirectoryWatcherManager {
	  private:
		std::unordered_map<std::string, std::function<void()>> m_watchFiles;
		std::vector<std::shared_ptr<pragma::fs::DirectoryWatcherCallback>> m_watchers;
		std::shared_ptr<pragma::fs::DirectoryWatcherManager> m_watcherManager;
		pragma::Game *m_game;
	  protected:
		virtual void OnLuaFileChanged(const std::string &path);
		bool IsLuaFile(const std::string &path, bool bAllowCompiled = false) const;
	  public:
		LuaDirectoryWatcherManager(pragma::Game *game);
		bool MountDirectory(const std::string &path, bool stripBaseBath = false);
		void Poll();
	};
};
