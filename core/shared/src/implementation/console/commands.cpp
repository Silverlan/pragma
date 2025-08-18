// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include <scripting/lua/lua.hpp>

// import pragma.scripting.lua;

module pragma.console.commands;

void pragma::console::commands::lua_run(lua_State *l, const std::string &chunkName, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float)
{
	std::string lua = argv[0];
	for(auto i = 1; i < argv.size(); i++) {
		lua += " ";
		lua += argv[i];
	}
	pragma::scripting::lua::run_string(l, lua, chunkName);
}

void pragma::console::commands::lua_run(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float)
{
	if(argv.empty()) {
		Con::cwar << "No argument given to execute!" << Con::endl;
		return;
	}
	if(!state->IsGameActive() || state->GetGameState() == nullptr) {
		Con::cwar << "No game is active! Lua code cannot be executed without an active game!" << Con::endl;
		return;
	}

	std::string lua = argv[0];
	for(auto i = 1; i < argv.size(); i++) {
		lua += " ";
		lua += argv[i];
	}
	state->GetGameState()->RunLua(lua);
}

void pragma::console::commands::lua_exec(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float)
{
	if(argv.empty()) {
		Con::cwar << "No argument given to execute!" << Con::endl;
		return;
	}
	if(!state->IsGameActive() || state->GetGameState() == nullptr) {
		Con::cwar << "No game is active! Lua code cannot be executed without an active game!" << Con::endl;
		return;
	}

	auto fname = argv.at(0);
	auto result = pragma::scripting::lua::include(state->GetLuaState(), fname, pragma::scripting::lua::IncludeFlags::AddToCache);
	Lua::Pop(state->GetLuaState(), result.numResults);
	if(result.statusCode != Lua::StatusCode::Ok)
		pragma::scripting::lua::submit_error(state->GetLuaState(), result.errorMessage);
}
void pragma::console::commands::lua_exec_autocomplete(const std::string &arg, std::vector<std::string> &autoCompleteOptions)
{
	std::vector<std::string> resFiles;
	auto path = Lua::SCRIPT_DIRECTORY_SLASH + arg;
	FileManager::FindFiles((path + "*." + Lua::FILE_EXTENSION).c_str(), &resFiles, nullptr);
	FileManager::FindFiles((path + "*." + Lua::FILE_EXTENSION_PRECOMPILED).c_str(), &resFiles, nullptr);
	autoCompleteOptions.reserve(resFiles.size());
	path = ufile::get_path_from_filename(path.substr(4));
	for(auto &mapName : resFiles) {
		auto fullPath = path + mapName;
		ustring::replace(fullPath, "\\", "/");
		autoCompleteOptions.push_back(fullPath);
	}
}
