/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <pragma/console/convars.h>
#include <sharedutils/util_file.h>
#include <cmaterialmanager.h>

void CEngine::RegisterConsoleCommands()
{
	Engine::RegisterConsoleCommands();
	auto &conVarMap = *console_system::client::get_convar_map();
	RegisterSharedConsoleCommands(conVarMap);
	conVarMap.RegisterConCommand("lua_exec_cl",[](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty() || !state->IsGameActive()) return;
		Game *game = state->GetGameState();
		if(game == NULL)
			return;
		Lua::set_ignore_include_cache(true);
			auto fname = argv.at(0);
			game->ExecuteLuaFile(fname);
		Lua::set_ignore_include_cache(false);
	},ConVarFlags::None,"Opens and executes a lua-file on the client.",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		std::vector<std::string> resFiles;
		auto path = "lua\\" +arg;
		FileManager::FindFiles((path +"*.lua").c_str(),&resFiles,nullptr);
		FileManager::FindFiles((path +"*.clua").c_str(),&resFiles,nullptr);
		autoCompleteOptions.reserve(resFiles.size());
		path = ufile::get_path_from_filename(path.substr(4));
		for(auto &mapName : resFiles)
		{
			auto fullPath = path +mapName;
			ustring::replace(fullPath,"\\","/");
			autoCompleteOptions.push_back(fullPath);
		}
	});
	conVarMap.RegisterConVar("cl_downscale_imported_high_resolution_rma_textures","1",ConVarFlags::Archive,"If enabled, imported high-resolution RMA textures will be downscaled to a more memory-friendly size.");
	conVarMap.RegisterConVarCallback("cl_downscale_imported_high_resolution_rma_textures",std::function<void(NetworkState*,ConVar*,bool,bool)>{[](
		NetworkState *nw,ConVar *cv,bool oldVal,bool newVal) -> void {
			static_cast<CMaterialManager&>(static_cast<ClientState*>(nw)->GetMaterialManager()).SetDownscaleImportedRMATextures(newVal);
	}});
	conVarMap.RegisterConVar("render_debug_mode","0",ConVarFlags::None,"0 = Disabled, 1 = Ambient Occlusion, 2 = Albedo Colors, 3 = Metalness, 4 = Roughness, 5 = Diffuse Lighting, 6 = Normals, 7 = Normal Map, 8 = Reflectance, 9 = IBL Prefilter, 10 = IBL Irradiance, 11 = Emission.");
#if LUA_ENABLE_RUN_GUI == 1
	conVarMap.RegisterConCommand("lua_exec_gui",[](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty()) return;
		Lua::set_ignore_include_cache(true);
		//client->LoadGUILuaFile(argv.front());
		Lua::set_ignore_include_cache(false);
	},ConVarFlags::None,"Opens and executes a lua-file on the GUI state.",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		std::vector<std::string> resFiles;
		auto path = "lua\\" +arg;
		FileManager::FindFiles((path +"*.lua").c_str(),&resFiles,nullptr);
		FileManager::FindFiles((path +"*.clua").c_str(),&resFiles,nullptr);
		autoCompleteOptions.reserve(resFiles.size());
		for(auto &mapName : resFiles)
		{
			auto fullPath = path.substr(4) +mapName;
			ustring::replace(fullPath,"\\","/");
			autoCompleteOptions.push_back(fullPath);
		}
	});
#endif
}
