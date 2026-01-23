// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.commands;
import :console.output;

void pragma::console::commands::lua_run(lua::State *l, const std::string &chunkName, BasePlayerComponent *, std::vector<std::string> &argv, float)
{
	std::string lua = argv[0];
	for(auto i = 1; i < argv.size(); i++) {
		lua += " ";
		lua += argv[i];
	}
	scripting::lua_core::run_string(l, lua, chunkName);
}

void pragma::console::commands::lua_run(NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float)
{
	if(argv.empty()) {
		Con::CWAR << "No argument given to execute!" << Con::endl;
		return;
	}
	if(!state->IsGameActive() || state->GetGameState() == nullptr) {
		Con::CWAR << "No game is active! Lua code cannot be executed without an active game!" << Con::endl;
		return;
	}

	std::string lua = argv[0];
	for(auto i = 1; i < argv.size(); i++) {
		lua += " ";
		lua += argv[i];
	}
	state->GetGameState()->RunLua(lua);
}

void pragma::console::commands::lua_exec(NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float)
{
	if(argv.empty()) {
		Con::CWAR << "No argument given to execute!" << Con::endl;
		return;
	}
	if(!state->IsGameActive() || state->GetGameState() == nullptr) {
		Con::CWAR << "No game is active! Lua code cannot be executed without an active game!" << Con::endl;
		return;
	}

	auto fname = argv.at(0);
	auto result = pragma::scripting::lua_core::include(state->GetLuaState(), fname, scripting::lua_core::IncludeFlags::AddToCache);
	Lua::Pop(state->GetLuaState(), result.numResults);
	if(result.statusCode != Lua::StatusCode::Ok)
		scripting::lua_core::submit_error(state->GetLuaState(), result.errorMessage);
}
void pragma::console::commands::lua_exec_autocomplete(const std::string &arg, std::vector<std::string> &autoCompleteOptions)
{
	std::vector<std::string> resFiles;
	auto path = Lua::SCRIPT_DIRECTORY_SLASH + arg;
	fs::find_files((path + "*." + Lua::FILE_EXTENSION), &resFiles, nullptr);
	fs::find_files((path + "*." + Lua::FILE_EXTENSION_PRECOMPILED), &resFiles, nullptr);
	autoCompleteOptions.reserve(resFiles.size());
	path = ufile::get_path_from_filename(path.substr(4));
	for(auto &mapName : resFiles) {
		auto fullPath = path + mapName;
		string::replace(fullPath, "\\", "/");
		autoCompleteOptions.push_back(fullPath);
	}
}
