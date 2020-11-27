/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include <pragma/rendering/render_apis.hpp>
#include <pragma/console/convars.h>
#include <sharedutils/util_file.h>
#include <cmaterialmanager.h>
#include <pragma/console/command_options.hpp>

extern DLLCLIENT void debug_render_stats(bool);
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
	conVarMap.RegisterConVar("render_api","opengl",ConVarFlags::Archive | ConVarFlags::Replicated,"The underlying rendering API to use.",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		auto &renderAPIs = pragma::rendering::get_available_graphics_apis();
		auto it = renderAPIs.begin();
		std::vector<std::string_view> similarCandidates {};
		ustring::gather_similar_elements(arg,[&it,&renderAPIs]() -> std::optional<std::string_view> {
			if(it == renderAPIs.end())
				return {};
			auto &name = *it;
			++it;
			return name;
		},similarCandidates,15);

		autoCompleteOptions.reserve(similarCandidates.size());
		for(auto &candidate : similarCandidates)
		{
			auto strOption = std::string{candidate};
			ufile::remove_extension_from_filename(strOption);
			autoCompleteOptions.push_back(strOption);
		}
	});
	conVarMap.RegisterConCommand("render_api_info",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		auto &renderAPI = GetRenderAPI();
		auto &context = GetRenderContext();
		Con::cout<<"Active render API: "<<renderAPI<<" ("<<context.GetAPIAbbreviation()<<")"<<Con::endl;
	},ConVarFlags::None,"Prints information about the current render API to the console.");
	conVarMap.RegisterConCommand("debug_render_stats",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		std::unordered_map<std::string,pragma::console::CommandOption> commandOptions {};
		pragma::console::parse_command_options(argv,commandOptions);
		auto full = util::to_boolean(pragma::console::get_command_option_parameter_value(commandOptions,"full","0"));
		debug_render_stats(full);
	},ConVarFlags::None,"Prints information about the next frame.");

	conVarMap.RegisterConVar("render_vsync_enabled","1",ConVarFlags::Archive,"Enables or disables vsync. OpenGL only.");
	conVarMap.RegisterConVarCallback("render_vsync_enabled",std::function<void(NetworkState*,ConVar*,bool,bool)>{[this](
		NetworkState *nw,ConVar *cv,bool oldVal,bool newVal) -> void {
			GetRenderContext().GetWindow().SetVSyncEnabled(newVal);
	}});
	
	conVarMap.RegisterConVar("render_instancing_threshold","2",ConVarFlags::Archive,"The threshold at which to start instancing entities if instanced rendering is enabled (render_instancing_threshold). Must not be lower than 2!");
	conVarMap.RegisterConVar("render_instancing_enabled","1",ConVarFlags::Archive,"Enables or disables instanced rendering.");

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
