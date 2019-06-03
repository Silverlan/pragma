#include "stdafx_engine.h"
#include "pragma/engine.h"
#include "pragma/lua/lua_doc.hpp"
#include <pragma/console/convars.h>
#include <pragma/lua/libraries/lutil.h>
#include <sharedutils/util_file.h>
#include <util_pragma_doc.hpp>
#include <unordered_set>

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
}

void Engine::RegisterConsoleCommands()
{
	auto &conVarMap = *console_system::server::get_convar_map();
	RegisterSharedConsoleCommands(conVarMap);
	conVarMap.RegisterConCommand("map",[](NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv,float) {
		if(argv.empty())
		{
			Con::cout<<state->GetMap()<<Con::endl;
			return;
		}
		state->EndGame();
		state->LoadMap(argv[0].c_str());
	},ConVarFlags::None,"Loads the given map immediately. Usage: map <mapName>",[](const std::string &arg,std::vector<std::string> &autoCompleteOptions) {
		std::vector<std::string> resFiles;
		FileManager::FindFiles("maps\\*.wld",&resFiles,nullptr);
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
		Lua::set_ignore_include_cache(true);
			game->ExecuteLuaFile(argv[0]);
		Lua::set_ignore_include_cache(false);
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
		ConConf *cv = state->GetConVar(argv[0]);
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
}
