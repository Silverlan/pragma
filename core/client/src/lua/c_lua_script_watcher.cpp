// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/lua/c_lua_script_watcher.hpp"

import pragma.client.scripting.lua;

void CLuaDirectoryWatcherManager::OnLuaFileChanged(const std::string &path)
{
	auto filePath = util::FilePath(path);
	auto it = filePath.begin();
	auto end = filePath.end();
	if(it != end && ustring::compare<std::string_view>(*it, "gui", false) == true) {
		++it;
		if(it != end && ustring::compare<std::string_view>(*it, "skins", false) == true) {
			++it;
			if(it != end) {
				std::string skinName {*it};
				ufile::remove_extension_from_filename(skinName, std::array<std::string, 1> {"lua"});
				Lua::gui::load_skin(skinName);
			}
		}
	}
	LuaDirectoryWatcherManager::OnLuaFileChanged(path);
}
