/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2025 Silverlan */

#include "stdafx_client.h"
#include "pragma/lua/c_lua_script_watcher.hpp"
#include "pragma/lua/libraries/c_lgui.h"
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
