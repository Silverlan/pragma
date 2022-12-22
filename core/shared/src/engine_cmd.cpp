/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/lua_doc.hpp"
#include "pragma/console/conout.h"
#include "pragma/game/savegame.hpp"
#include "pragma/asset/util_asset.hpp"
#include "pragma/localization.h"
#include <pragma/console/convars.h>
#include <pragma/lua/util.hpp>
#include <pragma/lua/libraries/lutil.hpp>
#include <pragma/physics/environment.hpp>
#include <pragma/networking/networking_modules.hpp>
#include <pragma/util/util_game.hpp>
#include <pragma/debug/intel_vtune.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/util_path.hpp>
#include <util_pragma_doc.hpp>
#include <unordered_set>
#include <udm.hpp>

#undef CreateFile

static void install_binary_module(const std::string &module,const std::optional<std::string> &version={});
void Engine::RegisterSharedConsoleCommands(ConVarMap &map)
{
	map.RegisterConCommand("exec",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
			return;
		ExecConfig(argv[0]);
	},ConVarFlags::None,"Executes a config file. Usage exec <fileName>",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		std::vector<std::string> resFiles;
		FileManager::FindFiles(("cfg\\" +arg +"*.cfg").c_str(),&resFiles,nullptr);
		autoCompleteOptions.reserve(resFiles.size());
		for(auto &mapName : resFiles)
		{
			ufile::remove_extension_from_filename(mapName);
			autoCompleteOptions.push_back(mapName);
		}
	});
	map.RegisterConCommand("udm_convert",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
		{
			Con::cwar<<"WARNING: No file specified to convert!"<<Con::endl;
			return;
		}
		auto &fileName = argv.front();
		std::string err;
		auto formatType = udm::Data::GetFormatType(fileName,err);
		if(formatType.has_value() == false)
		{
			Con::cwar<<"WARNING: Unable to load UDM data: "<<err<<Con::endl;
			return;
		}
		auto udmData = util::load_udm_asset(fileName,&err);
		if(udmData == nullptr)
		{
			Con::cwar<<"WARNING: Unable to load UDM data: "<<err<<Con::endl;
			return;
		}
		std::string rpath;
		if(FileManager::FindAbsolutePath(fileName,rpath) == false)
		{
			Con::cwar<<"WARNING: Unable to locate UDM file on disk!"<<Con::endl;
			return;
		}
		auto path = util::Path::CreateFile(rpath);
		path.MakeRelative(util::get_program_path());
		auto outFileName = path.GetString();
		std::string ext;
		ufile::get_extension(outFileName,&ext);
		ufile::remove_extension_from_filename(outFileName);
		if(*formatType == udm::FormatType::Binary)
		{
			if(ext.empty())
				ext = "udm_b";
			else if(ext.length() > 2)
			{
				if(ext.at(ext.length() -1) == 'b' && ext.at(ext.length() -2) == '_')
					ext = ext.substr(0,ext.length() -2);
			}
			outFileName += '.' +ext;
			try
			{
				udmData->SaveAscii(outFileName,udm::AsciiSaveFlags::IncludeHeader | udm::AsciiSaveFlags::DontCompressLz4Arrays);
			}
			catch(const udm::Exception &e)
			{
				Con::cwar<<"WARNING: Unable to save UDM data: "<<e.what()<<Con::endl;
			}
		}
		else
		{
			if(ext.empty())
				ext = "udm_a";
			else
				ext += "_b";
			outFileName += '.' +ext;
			try
			{
				udmData->Save(outFileName);
			}
			catch(const udm::Exception &e)
			{
				Con::cwar<<"WARNING: Unable to save UDM data: "<<e.what()<<Con::endl;
			}
		}
		auto absPath = util::get_program_path() +'/' +outFileName;
		util::open_path_in_explorer(ufile::get_path_from_filename(absPath),ufile::get_file_from_filename(absPath));
	},ConVarFlags::None,"Converts a UDM file from binary to ASCII or the other way around.");
	map.RegisterConCommand("udm_validate",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
		{
			Con::cwar<<"WARNING: No file specified to convert!"<<Con::endl;
			return;
		}
		auto &fileName = argv.front();
		std::string err;
		auto udmData = util::load_udm_asset(fileName,&err);
		if(udmData)
			Con::cout<<"No validation errors found, file is a valid UDM file!"<<err<<Con::endl;
	},ConVarFlags::None,"Validates the specified UDM file.");
	map.RegisterConVar<std::string>("phys_engine","bullet",ConVarFlags::Archive | ConVarFlags::Replicated,"The underlying physics engine to use.","<physEngie>",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		auto physEngines = pragma::physics::IEnvironment::GetAvailablePhysicsEngines();
		auto it = physEngines.begin();
		std::vector<std::string_view> similarCandidates {};
		ustring::gather_similar_elements(arg,[&it,&physEngines]() -> std::optional<std::string_view> {
			if(it == physEngines.end())
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
	map.RegisterConVar<std::string>("net_library","steam_networking",ConVarFlags::Archive | ConVarFlags::Replicated,"The underlying networking library to use for multiplayer games.","<netLibrary>",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		auto netLibs = pragma::networking::GetAvailableNetworkingModules();
		auto it = netLibs.begin();
		std::vector<std::string_view> similarCandidates {};
		ustring::gather_similar_elements(arg,[&it,&netLibs]() -> std::optional<std::string_view> {
			if(it == netLibs.end())
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
	map.RegisterConVar<bool>("sv_require_authentication",true,ConVarFlags::Archive | ConVarFlags::Replicated,"If enabled, clients will have to authenticate via steam to join the server.");
	
	map.RegisterConVar<bool>("asset_multithreading_enabled",true,ConVarFlags::Archive,"If enabled, assets will be loaded in the background.");
	map.RegisterConVarCallback("asset_multithreading_enabled",std::function<void(NetworkState*,ConVar*,bool,bool)>{[this](
		NetworkState *nw,ConVar *cv,bool oldVal,bool newVal) -> void {
		SetAssetMultiThreadedLoadingEnabled(newVal);
	}});

	map.RegisterConVar<bool>("asset_file_cache_enabled",true,ConVarFlags::Archive,"If enabled, all Pragma files will be indexed to improve lookup times.");
	map.RegisterConVarCallback("asset_file_cache_enabled",std::function<void(NetworkState*,ConVar*,bool,bool)>{[this](
		NetworkState *nw,ConVar *cv,bool oldVal,bool newVal) -> void {
		filemanager::set_use_file_index_cache(newVal);
	}});
}

void Engine::RegisterConsoleCommands()
{
	auto &conVarMap = *console_system::server::get_convar_map();
	RegisterSharedConsoleCommands(conVarMap);
	// Note: Serverside ConVars HAVE to be registered shared if the command is replicated!
	conVarMap.RegisterConCommand("map",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
		{
			Con::cout<<state->GetMap()<<Con::endl;
			return;
		}
		auto map = argv.front();
		// We can't change the map within a console command callback, so
		// we'll delay it to a safe point.
		AddTickEvent([this,map]() {
			StartDefaultGame(map);
		});
	},ConVarFlags::None,"Loads the given map immediately. Usage: map <mapName>",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		std::vector<std::string> resFiles;
		auto exts = pragma::asset::get_supported_extensions(pragma::asset::Type::Map);
		for(auto &ext : exts)
			filemanager::find_files("maps/*." +ext,&resFiles,nullptr);
		for(auto &f : resFiles)
			ufile::remove_extension_from_filename(f,exts);
		auto it = resFiles.begin();
		std::vector<std::string_view> similarCandidates {};
		ustring::gather_similar_elements(arg,[&it,&resFiles]() -> std::optional<std::string_view> {
			if(it == resFiles.end())
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
	conVarMap.RegisterConCommand("lua_exec",[](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty() || !state->IsGameActive())
			return;
		Game *game = state->GetGameState();
		if(game == NULL)
			return;
		auto fname = argv.at(0);
		if(argv.size() > 1 && argv[1] == "nocache")
		{
			Lua::set_ignore_include_cache(true);
				game->ExecuteLuaFile(fname);
			Lua::set_ignore_include_cache(false);
			return;
		}
        Lua::global::include(state->GetLuaState(),fname,nullptr,true,false);
	},ConVarFlags::None,"Opens and executes a lua-file on the server.",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
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
	conVarMap.RegisterConCommand("save",[](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		auto *game = state ? state->GetGameState() : nullptr;
		if(game == nullptr)
		{
			Con::cwar<<"WARNING: Cannot create savegame: No active game!"<<Con::endl;
			return;
		}
		auto path = "savegames/" +util::get_date_time("%Y-%m-%d_%H-%M-%S") +".psav_b";
		FileManager::CreatePath(ufile::get_path_from_filename(path).c_str());
		std::string err;
		auto result = pragma::savegame::save(*game,path,err);
		if(result == false)
			Con::cwar<<"WARNING: Cannot create savegame: "<<err<<Con::endl;
		else
			Con::cout<<"Created savegame as '"<<path<<"'!"<<Con::endl;
	},ConVarFlags::None);
	conVarMap.RegisterConCommand("load",[](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
		{
			Con::cwar<<"WARNING: Cannot load savegame: No savegame specified!"<<Con::endl;
			return;
		}
		state->EndGame();
		state->StartGame(true);
		auto *game = state ? state->GetGameState() : nullptr;
		if(game == nullptr)
		{
			Con::cwar<<"WARNING: Cannot load savegame: No active game!"<<Con::endl;
			return;
		}
		auto path = "savegames/" +util::get_date_time() +".psav";
		FileManager::CreatePath(ufile::get_path_from_filename(path).c_str());
		std::string err;
		auto result = pragma::savegame::load(*game,path,err);
		if(result == false)
			Con::cwar<<"WARNING: Cannot load savegame: "<<err<<Con::endl;
	},ConVarFlags::None);

	conVarMap.RegisterConCommand("lua_help",[](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
			return;
		Lua::doc::print_documentation(argv.front());
	},ConVarFlags::None,"Prints information about the specified function, library or enum (or the closest candiate). Usage: lua_help <function/library/enum>.",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		std::vector<const pragma::doc::BaseCollectionObject*> candidates {};
		Lua::doc::find_candidates(arg,candidates,15u);
		autoCompleteOptions.reserve(candidates.size());
		for(auto *pCandidate : candidates)
			autoCompleteOptions.push_back(pCandidate->GetFullName());
	});

	conVarMap.RegisterConCommand("help",[](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
		{
			Con::cout<<"Usage: help <cvarname>"<<Con::endl;
			return;
		}
		auto *en = pragma::get_engine();
		auto *cl = en ? en->GetClientState() : nullptr;
		auto *sv = en ? en->GetServerNetworkState() : nullptr;
		ConConf *cv = cl ? cl->GetConVar(argv[0]) : nullptr;
		if(!cv)
			cv = sv->GetConVar(argv[0]);
		if(cv == NULL)
		{
			Con::cout<<"help: no cvar or command named "<<argv[0]<<Con::endl;
			return;
		}
		cv->Print(argv[0]);
	},ConVarFlags::None,"Find help about a convar/concommand.",[this](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		auto *cvMap = GetConVarMap();
		std::vector<std::string_view> similarCandidates {};
		std::vector<float> similarities {};
		std::unordered_set<std::string_view> iteratedCvars = {};
		auto fItConVarMap = [&arg,&similarCandidates,&similarities,&iteratedCvars](ConVarMap &cvMap) {
			auto &conVars = cvMap.GetConVars();
			auto it = conVars.begin();

			ustring::gather_similar_elements(arg,[&it,&conVars,&iteratedCvars]() -> std::optional<std::string_view> {
				if(it == conVars.end())
					return {};
				auto itIterated = iteratedCvars.find(it->first);
				while(itIterated != iteratedCvars.end() && it != conVars.end())
				{
					++it;
					itIterated = iteratedCvars.find(it->first);
				}
				if(it == conVars.end())
					return {};
				auto &pair = *it;
				++it;
				iteratedCvars.insert(pair.first);
				return pair.first;
			},similarCandidates,15,&similarities);
		};
		fItConVarMap(*cvMap);
		auto *svState = GetServerNetworkState();
		if(svState)
			fItConVarMap(*svState->GetConVarMap());
		auto *clState = GetClientState();
		if(clState)
			fItConVarMap(*clState->GetConVarMap());

		autoCompleteOptions.reserve(similarCandidates.size());
		for(auto &candidate : similarCandidates)
			autoCompleteOptions.push_back(std::string{candidate});
	});

	conVarMap.RegisterConCommand("clear_cache",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		ClearCache();
	},ConVarFlags::None,"Clears all of the cached engine files.");

	conVarMap.RegisterConCommand("loc_reload",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		Locale::ReloadFiles();
	},ConVarFlags::None,"Reloads all localization files.");

	conVarMap.RegisterConCommand("loc_find",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
		{
			Con::cwar<<"WARNING: No argument specified!"<<Con::endl;
			return;
		}
		auto &texts = Locale::GetTexts();
		std::vector<std::string> baseTexts;
		std::vector<std::string> ids;
		baseTexts.reserve(texts.size());
		ids.reserve(texts.size());
		for(auto &pair : texts)
		{
			ids.push_back(pair.first);
			baseTexts.push_back(pair.second.cpp_str());
		}
		std::vector<size_t> similarElements {};
		std::vector<float> similarities {};
		ustring::gather_similar_elements(argv.front(),baseTexts,similarElements,6,&similarities);
		Con::cout<<"Found "<<similarElements.size()<<" similar matches:"<<Con::endl;
		for(auto idx : similarElements)
			Con::cout<<ids[idx]<<": "<<baseTexts[idx]<<Con::endl;
		Con::cout<<Con::endl;
	},ConVarFlags::None,"Searches for the specified text in all currently loaded text strings.");

	conVarMap.RegisterConCommand("asset_clear_unused_models",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		ClearUnusedAssets(pragma::asset::Type::Model,true);
	},ConVarFlags::None,"Clears all unused models from memory.");
	conVarMap.RegisterConCommand("asset_clear_unused_materials",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		ClearUnusedAssets(pragma::asset::Type::Material,true);
	},ConVarFlags::None,"Clears all unused materials from memory.");
	conVarMap.RegisterConCommand("asset_clear_unused",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		std::vector<pragma::asset::Type> types;
		auto n = umath::to_integral(pragma::asset::Type::Count);
		types.reserve(n);
		for(auto i=decltype(n){0u};i<n;++i)
			types.push_back(static_cast<pragma::asset::Type>(i));
		ClearUnusedAssets(types,true);
	},ConVarFlags::None,"Clears all unused assets from memory.");
	conVarMap.RegisterConCommand("install_module",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
		{
			Con::cwar<<"WARNING: No module specified!"<<Con::endl;
			return;
		}
		std::optional<std::string> version {};
		if(argv.size() > 1)
			version = argv[1];
		install_binary_module(argv[0],version);
	},ConVarFlags::None,"Install the specified binary module. Usage: install_module <module> <version>. If no version is specified, the latest version will be downloaded.");
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	conVarMap.RegisterConCommand("debug_vtune_prof_start",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		std::string name = "cmd";
		if(!argv.empty())
			name = argv.front();
		::debug::get_domain().BeginTask(name);
	},ConVarFlags::None,"Start the VTune profiler.");
	conVarMap.RegisterConCommand("debug_vtune_prof_end",[this](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		::debug::get_domain().EndTask();
	},ConVarFlags::None,"End the VTune profiler.");
#endif
}

#include "pragma/util/curl_query_handler.hpp"
#include <util_zip.h>

class ModuleInstallJob
	: public util::ParallelWorker<bool>
{
public:
	ModuleInstallJob(const std::string &module,const std::optional<std::string> &version)
		: m_module{module},m_version{version}
	{
		AddThread([this]() {Install();});
	}
	virtual bool GetResult() override {return m_success;}
private:
	virtual void DoCancel(const std::string &resultMsg,std::optional<int32_t> resultCode) override
	{
		m_curl.CancelDownload();
	}
	void Install();
	template<typename TJob,typename... TARGS>
		friend util::ParallelJob<typename TJob::RESULT_TYPE> util::create_parallel_job(TARGS&& ...args);

	std::atomic<bool> m_success = false;
	std::chrono::steady_clock::time_point m_lastProgressTime;
	pragma::CurlQueryHandler m_curl {};
	std::string m_module;
	std::optional<std::string> m_version {};
};

void ModuleInstallJob::Install()
{
#ifdef _WIN32
	std::string archiveName = "binaries_windows64.zip";
#else
	std::string archiveName = "binaries_linux64.tar.gz";
#endif
	std::string url = std::string{"https://github.com/"} +m_module +"/releases/download/";
	if(m_version.has_value())
		url += "v" +*m_version;
	else
		url += "latest";
	url += "/" +archiveName;

#ifdef __linux__
	{
		Con::cwar<<"WARNING: Automatic installation of binary modules using the 'install_module' console command is currently not supported on Linux! You will have to install the module manually."<<Con::endl;
		Con::cwar<<"The download should automatically start through your browser. If not, you can download the module here: "<<url<<Con::endl;
		Con::cwar<<"Once downloaded, simply extract the archive over your Pragma installation."<<Con::endl;
		util::open_url_in_browser(url);
		UpdateProgress(1.f);
		SetStatus(util::JobStatus::Successful);
		return;
	}
#endif

	filemanager::create_directory("temp");
	auto archivePath = "temp/" +archiveName;
	m_lastProgressTime = std::chrono::steady_clock::now();
	Con::cout<<"Downloading module from '"<<url<<"'..."<<Con::endl;
	m_curl.AddResource(url,archivePath,[this](int64_t dltotal,int64_t dlnow,int64_t ultotal,int64_t ulnow) {
		if(dltotal == 0)
			return;
		auto t = std::chrono::steady_clock::now();
		auto dt = t -m_lastProgressTime;
		if(dt < std::chrono::seconds{5})
			return;
		m_lastProgressTime = t;
		auto fprogress = dlnow /static_cast<float>(dltotal);
		auto progress = util::round_string(fprogress *100.f,2);
		Con::cout<<"Module download at "<<progress<<"%"<<Con::endl;
		UpdateProgress(fprogress *0.9f);
	},[this,archivePath](int code) {
		UpdateProgress(0.9f);
		if(code == 0)
		{
			auto zip = ZIPFile::Open(archivePath,ZIPFile::OpenMode::Read);
			if(!zip)
			{
				std::string msg = "Failed to open module archive '" +archivePath +"'!";
				Con::cwar<<"WARNING: "<<msg<<Con::endl;
				SetStatus(util::JobStatus::Failed,msg);
				return;
			}
			
			Con::cout<<"Extracting module archive '"<<archivePath<<"'..."<<Con::endl;
			std::string err;
			if(!zip->ExtractFiles(util::get_program_path(),err))
			{
				std::string msg = "Failed to extract module archive '" +archivePath +"'!";
				Con::cwar<<"WARNING: "<<msg<<Con::endl;
				SetStatus(util::JobStatus::Failed,msg);
				return;
			}
		}
		else
		{
			std::string msg = "Failed to download module '" +m_module +"'!";
			Con::cwar<<"WARNING: "<<msg<<Con::endl;
			SetStatus(util::JobStatus::Failed,msg);
			return;
		}
		filemanager::remove_file(archivePath);

		UpdateProgress(1.f);
		SetStatus(util::JobStatus::Successful);
		Con::cout<<"Binary module '"<<archivePath<<"' has been installed successfully!"<<Con::endl;
	});
	m_curl.StartDownload();
	while(!m_curl.IsComplete())
		std::this_thread::sleep_for(std::chrono::milliseconds{500});
}

void install_binary_module(const std::string &module,const std::optional<std::string> &version)
{
	auto job = util::create_parallel_job<ModuleInstallJob>(module,version);
	job.Start();
	pragma::get_engine()->AddParallelJob(job,"Install Binary Module '" +module +"'");
}
