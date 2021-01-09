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
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_dynamic_resizable_buffer.hpp>

extern DLLCLIENT void debug_render_stats(bool enabled,bool full,bool print,bool continuous);
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
		debug_render_stats(true,full,true,false);
	},ConVarFlags::None,"Prints information about the next frame.");
	conVarMap.RegisterConVar("render_multithreaded_rendering_enabled","1",ConVarFlags::Archive,"Enables or disables multi-threaded rendering. Some renderers (like OpenGL) don't support multi-threaded rendering and will ignore this flag.");
	conVarMap.RegisterConVarCallback("render_multithreaded_rendering_enabled",std::function<void(NetworkState*,ConVar*,bool,bool)>{[this](
		NetworkState *nw,ConVar *cv,bool,bool enabled) -> void {
			GetRenderContext().SetMultiThreadedRenderingEnabled(enabled);
	}});
	conVarMap.RegisterConCommand("debug_dump_shader_code",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
		{
			Con::cwar<<"No shader specified!"<<Con::endl;
			return;
		}
		auto &shaderName = argv.front();
		auto shader = GetShader(shaderName);
		if(shader.expired())
		{
			Con::cwar<<"WARNING:: Shader '"<<shaderName<<"' is invalid!"<<Con::endl;
			return;
		}
		std::vector<std::string> glslCodePerStage;
		std::vector<prosper::ShaderStage> glslCodeStages;
		std::string infoLog,debugInfoLog;
		prosper::ShaderStage errStage;
		auto result = GetRenderContext().GetParsedShaderSourceCode(*shader,glslCodePerStage,glslCodeStages,infoLog,debugInfoLog,errStage);
		if(result == false)
		{
			Con::cwar<<"WARNING: Parsing shader '"<<shaderName<<"' has failed:"<<Con::endl;
			Con::cwar<<"Info Log: "<<infoLog<<Con::endl;
			Con::cwar<<"Debug info Log: "<<debugInfoLog<<Con::endl;
			Con::cwar<<"Stage: "<<prosper::util::to_string(errStage)<<Con::endl;
			return;
		}
		std::string path = "shader_dump/" +shaderName +"/";
		FileManager::CreatePath(path.c_str());
		for(auto i=decltype(glslCodeStages.size()){0u};i<glslCodeStages.size();++i)
		{
			auto &glslCode = glslCodePerStage[i];
			auto stage = glslCodeStages[i];
			std::string stageName;
			switch(stage)
			{
			case prosper::ShaderStage::Compute:
				stageName = "compute";
				break;
			case prosper::ShaderStage::Fragment:
				stageName = "fragment";
				break;
			case prosper::ShaderStage::Geometry:
				stageName = "geometry";
				break;
			case prosper::ShaderStage::TessellationControl:
				stageName = "tessellation_control";
				break;
			case prosper::ShaderStage::TessellationEvaluation:
				stageName = "tessellation_evaluation";
				break;
			case prosper::ShaderStage::Vertex:
				stageName = "vertex";
				break;
			}
			static_assert(umath::to_integral(prosper::ShaderStage::Count) == 6);
			auto stageFileName = path +stageName +".gls";
			auto f = FileManager::OpenFile<VFilePtrReal>(stageFileName.c_str(),"w");
			if(f)
			{
				f->WriteString(glslCode);
				f = nullptr;
			}
			else
				Con::cwar<<"WARNING: Unable to write file '"<<stageFileName<<"'!"<<Con::endl;
		}
		Con::cout<<"Done! Written shader files to '"<<path<<"'!"<<Con::endl;
	},ConVarFlags::None,"Dumps the glsl code for the specified shader.");
	conVarMap.RegisterConVar("debug_hide_gui","0",ConVarFlags::None,"Disables GUI rendering.");

	conVarMap.RegisterConVar("render_vsync_enabled","1",ConVarFlags::Archive,"Enables or disables vsync. OpenGL only.");
	conVarMap.RegisterConVarCallback("render_vsync_enabled",std::function<void(NetworkState*,ConVar*,bool,bool)>{[this](
		NetworkState *nw,ConVar *cv,bool oldVal,bool newVal) -> void {
			GetRenderContext().GetWindow().SetVSyncEnabled(newVal);
	}});
	
	conVarMap.RegisterConVar("render_instancing_threshold","2",ConVarFlags::Archive,"The threshold at which to start instancing entities if instanced rendering is enabled (render_instancing_threshold). Must not be lower than 2!");
	conVarMap.RegisterConVar("render_instancing_enabled","0",ConVarFlags::Archive,"Enables or disables instanced rendering.");
	conVarMap.RegisterConVar("render_queue_worker_thread_count","3",ConVarFlags::Archive,"Number of threads to use for generating render queues.");
	conVarMap.RegisterConVar("render_queue_entities_per_worker_job","5",ConVarFlags::Archive,"Number of entities for each job processed by a worker thread.");
	conVarMap.RegisterConVar("render_queue_worker_jobs_per_batch","2",ConVarFlags::Archive,"Number of worker jobs to accumulate in a batch before assigning a worker.");
	
	conVarMap.RegisterConCommand("debug_textures",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		auto &texManager = static_cast<CMaterialManager&>(static_cast<ClientState*>(GetClientState())->GetMaterialManager()).GetTextureManager();
		auto textures = texManager.GetTextures();
		std::vector<prosper::DeviceSize> textureSizes;
		std::vector<size_t> sortedIndices {};
		textureSizes.reserve(textures.size());
		sortedIndices.reserve(textures.size());
		for(auto &tex : textures)
		{
			prosper::DeviceSize size = 0;
			auto &vkTex = tex->GetVkTexture();
			if(vkTex)
			{
				auto &img = vkTex->GetImage();
				auto *buf = img.GetMemoryBuffer();
				if(buf)
					size = buf->GetSize();
			}
			sortedIndices.push_back(textureSizes.size());
			textureSizes.push_back(size);
		}
		std::sort(sortedIndices.begin(),sortedIndices.end(),[&textureSizes](size_t idx0,size_t idx1) {
			return textureSizes[idx0] > textureSizes[idx1];
		});
		prosper::DeviceSize totalSize = 0;
		Con::cout<<textures.size()<<" textures are currently loaded:"<<Con::endl;
		for(auto idx : sortedIndices)
		{
			auto &tex = textures[idx];
			auto useCount = tex.use_count() -2;
			Con::cout<<tex->GetName()<<":"<<Con::endl;

			if(useCount == 0)
				util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
			Con::cout<<"\tUse count: "<<useCount<<Con::endl;
			if(useCount == 0)
				util::reset_console_color();

			Con::cout<<"\tResolution: "<<tex->GetWidth()<<"x"<<tex->GetHeight()<<Con::endl;

			auto &img = tex->GetVkTexture()->GetImage();
			Con::cout<<"\tLayers: "<<img.GetLayerCount()<<Con::endl;
			
			auto numMipmaps = img.GetMipmapCount();
			if(numMipmaps <= 1)
				util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
			Con::cout<<"\tMipmaps: "<<numMipmaps<<Con::endl;
			if(numMipmaps <= 1)
				util::reset_console_color();

			auto tiling = img.GetTiling();
			auto optimal = tiling == prosper::ImageTiling::Optimal;
			if(!optimal)
				util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
			Con::cout<<"\tTiling: "<<prosper::util::to_string(tiling)<<Con::endl;
			if(!optimal)
				util::reset_console_color();

			auto format = img.GetFormat();
			auto isCompressed = prosper::util::is_compressed_format(format);
			if(!isCompressed)
				util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
			Con::cout<<"\tFormat: "<<prosper::util::to_string(format)<<Con::endl;
			if(!isCompressed)
				util::reset_console_color();

			auto size = textureSizes[idx];
			Con::cout<<"\tSize: "<<util::get_pretty_bytes(size)<<Con::endl;
			totalSize += size;

			auto deviceLocal = umath::is_flag_set(img.GetCreateInfo().memoryFeatures,prosper::MemoryFeatureFlags::DeviceLocal);
			if(!deviceLocal)
			{
				util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
				Con::cout<<"\tPerformance Warning: Image memory is not device local!"<<Con::endl;
				util::reset_console_color();
			}
		}
		Con::cout<<"Total memory: "<<util::get_pretty_bytes(totalSize)<<Con::endl;

		auto &imgBufs = GetRenderContext().GetDeviceImageBuffers();
		totalSize = 0;
		for(auto &imgBuf : imgBufs)
			totalSize += imgBuf->GetSize() -imgBuf->GetFreeSize();
		Con::cout<<"Total device image memory: "<<util::get_pretty_bytes(totalSize)<<Con::endl;
	},ConVarFlags::None,"Prints information about the currently loaded textures.");
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
