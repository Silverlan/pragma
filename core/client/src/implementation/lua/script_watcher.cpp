// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.script_watcher;
import :gui;

void CLuaDirectoryWatcherManager::OnLuaFileChanged(const std::string &path)
{
	auto filePath = pragma::util::FilePath(path);
	auto it = filePath.begin();
	auto end = filePath.end();
	if(it != end && pragma::string::compare<std::string_view>(*it, "gui", false) == true) {
		++it;
		if(it != end && pragma::string::compare<std::string_view>(*it, "skins", false) == true) {
			++it;
			if(it != end) {
				std::string skinName {*it};
				ufile::remove_extension_from_filename(skinName, std::array<std::string, 1> {"lua"});
				load_skin(skinName);
			}
		}
	}
	LuaDirectoryWatcherManager::OnLuaFileChanged(path);
}
