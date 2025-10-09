// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/lua/luaapi.h"

module pragma.server.scripting.lua.script_watcher;

import pragma.server.game;
import pragma.server.networking.resource_manager;
export pragma.server.server_state;

#undef FindResource

void SLuaDirectoryWatcherManager::OnLuaFileChanged(const std::string &path)
{
	if(IsLuaFile(path) == false)
		return;
	LuaDirectoryWatcherManager::OnLuaFileChanged(path);

	auto *res = ResourceManager::FindResource(Lua::SCRIPT_DIRECTORY_SLASH + path.substr(0, path.length() - 3) + Lua::FILE_EXTENSION_PRECOMPILED);
	if(res == nullptr)
		return;
	SGame::Get()->UpdateLuaCache(res->fileName);
	ServerState::Get()->SendResourceFile(res->fileName);
}
