// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/lua/luaapi.h"
#include "pragma/lua/s_lua_script_watcher.h"

module pragma.server.scripting.lua.script_watcher;

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
