// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#undef CreateFile

module pragma.shared;

import :console.commands;
import :engine;
import :locale;
import :network_state;

import util_zip;
import pragma.doc;
// import pragma.scripting.lua;

static std::optional<std::string> udm_convert(const std::string &fileName)
{
	std::string err;
	auto formatType = udm::Data::GetFormatType(fileName, err);
	if(formatType.has_value() == false) {
		Con::CWAR << "Unable to load UDM data: " << err << Con::endl;
		return {};
	}

	std::optional<std::string> newFileName {};
	switch(*formatType) {
	case udm::FormatType::Ascii:
		newFileName = pragma::util::convert_udm_file_to_binary(fileName, err);
		break;
	case udm::FormatType::Binary:
		newFileName = pragma::util::convert_udm_file_to_ascii(fileName, err);
		break;
	}

	if(!newFileName.has_value()) {
		Con::CWAR << "Failed to convert UDM file: " << err << Con::endl;
		return {};
	}

	std::string rpath;
	if(pragma::fs::find_absolute_path(*newFileName, rpath) == false) {
		Con::CWAR << "Unable to locate converted UDM file on disk!" << Con::endl;
		return {};
	}
	return rpath;
}

static void install_binary_module(const std::string &module, const std::optional<std::string> &version = {});
void pragma::Engine::RegisterSharedConsoleCommands(console::ConVarMap &map)
{
	map.RegisterConVar<udm::String>("rcon_password", "", console::ConVarFlags::Password, "Specifies a password which can be used to run console commands remotely on a server. If no password is specified, this feature is disabled.");
	map.RegisterConCommand(
	  "exec",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty())
			  return;
		  ExecConfig(argv[0]);
	  },
	  console::ConVarFlags::None, "Executes a config file. Usage exec <fileName>",
	  [](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		  std::vector<std::string> resFiles;
		  fs::find_files(("cfg\\" + arg + "*.cfg"), &resFiles, nullptr);
		  autoCompleteOptions.reserve(resFiles.size());
		  for(auto &mapName : resFiles) {
			  ufile::remove_extension_from_filename(mapName);
			  autoCompleteOptions.push_back(mapName);
		  }
	  });
	map.RegisterConCommand(
	  "udm_convert",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::CWAR << "No file specified to convert!" << Con::endl;
			  return;
		  }
		  auto &fileName = argv.front();
		  if(fs::is_dir(fileName)) {
			  auto f = fileName + "/*";
			  std::vector<std::string> files;
			  fs::find_files(f, &files, nullptr);
			  for(auto &f : files)
				  udm_convert(fileName + "/" + f);
			  return;
		  }
		  auto rpath = udm_convert(fileName);
		  if(!rpath)
			  return;
		  auto absPath = *rpath;
		  util::open_path_in_explorer(ufile::get_path_from_filename(absPath), ufile::get_file_from_filename(absPath));
	  },
	  console::ConVarFlags::None, "Converts a UDM file from binary to ASCII or the other way around.");
	map.RegisterConCommand(
	  "udm_validate",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::CWAR << "No file specified to convert!" << Con::endl;
			  return;
		  }
		  auto &fileName = argv.front();
		  std::string err;
		  auto udmData = util::load_udm_asset(fileName, &err);
		  if(udmData)
			  Con::COUT << "No validation errors found, file is a valid UDM file!" << Con::endl;
		  else
			  Con::CERR << "Validation failed: " << err << Con::endl;
	  },
	  console::ConVarFlags::None, "Validates the specified UDM file.");
	map.RegisterConVar<std::string>("phys_engine", "bullet", console::ConVarFlags::Archive | console::ConVarFlags::Replicated, "The underlying physics engine to use.", "<physEngie>", [](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		auto physEngines = physics::IEnvironment::GetAvailablePhysicsEngines();
		auto it = physEngines.begin();
		std::vector<std::string_view> similarCandidates {};
		string::gather_similar_elements(
		  arg,
		  [&it, &physEngines]() -> std::optional<std::string_view> {
			  if(it == physEngines.end())
				  return {};
			  auto &name = *it;
			  ++it;
			  return name;
		  },
		  similarCandidates, 15);

		autoCompleteOptions.reserve(similarCandidates.size());
		for(auto &candidate : similarCandidates) {
			auto strOption = std::string {candidate};
			ufile::remove_extension_from_filename(strOption);
			autoCompleteOptions.push_back(strOption);
		}
	});
	map.RegisterConVar<std::string>("net_library", "game_networking", console::ConVarFlags::Archive | console::ConVarFlags::Replicated, "The underlying networking library to use for multiplayer games.", "<netLibrary>",
	  [](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		  auto netLibs = networking::GetAvailableNetworkingModules();
		  auto it = netLibs.begin();
		  std::vector<std::string_view> similarCandidates {};
		  string::gather_similar_elements(
		    arg,
		    [&it, &netLibs]() -> std::optional<std::string_view> {
			    if(it == netLibs.end())
				    return {};
			    auto &name = *it;
			    ++it;
			    return name;
		    },
		    similarCandidates, 15);

		  autoCompleteOptions.reserve(similarCandidates.size());
		  for(auto &candidate : similarCandidates) {
			  auto strOption = std::string {candidate};
			  ufile::remove_extension_from_filename(strOption);
			  autoCompleteOptions.push_back(strOption);
		  }
	  });
	map.RegisterConVar<bool>("sv_require_authentication", false, console::ConVarFlags::Archive | console::ConVarFlags::Replicated, "If enabled, clients will have to authenticate via steam to join the server.");

	map.RegisterConVar<bool>("asset_multithreading_enabled", true, console::ConVarFlags::Archive, "If enabled, assets will be loaded in the background.");
	map.RegisterConVarCallback("asset_multithreading_enabled", std::function<void(NetworkState *, const console::ConVar &, bool, bool)> {[this](NetworkState *nw, const console::ConVar &cv, bool oldVal, bool newVal) -> void { SetAssetMultiThreadedLoadingEnabled(newVal); }});

	map.RegisterConVar<bool>("asset_file_cache_enabled", true, console::ConVarFlags::Archive, "If enabled, all Pragma files will be indexed to improve lookup times.");
	map.RegisterConVarCallback("asset_file_cache_enabled", std::function<void(NetworkState *, const console::ConVar &, bool, bool)> {[this](NetworkState *nw, const console::ConVar &cv, bool oldVal, bool newVal) -> void { fs::set_use_file_index_cache(newVal); }});

	map.RegisterConVarCallback("sv_gravity", std::function<void(NetworkState *, const console::ConVar &, std::string, std::string)> {[](NetworkState *state, const console::ConVar &, std::string prev, std::string val) {
		if(!state->IsGameActive())
			return;
		Vector3 gravity = uvec::create(val);
		Game *game = state->GetGameState();
		game->SetGravity(gravity);
	}});
	map.RegisterConCommand(
	  "list",
	  +[](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &, float) {
		  auto &convars = state->GetConVars();
		  std::vector<std::string> cvars(convars.size());
		  size_t idx = 0;
		  for(auto it = convars.begin(); it != convars.end(); ++it) {
			  cvars[idx] = it->first;
			  idx++;
		  }
		  std::sort(cvars.begin(), cvars.end());
		  std::vector<std::string>::iterator it;
		  for(it = cvars.begin(); it != cvars.end(); it++) {
			  if(*it != "credits")
				  Con::COUT << *it << Con::endl;
		  }
	  },
	  console::ConVarFlags::None, "Prints a list of all serverside console commands to the console.");

	map.RegisterConCommand(
	  "find",
	  +[](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::CWAR << "No argument given!" << Con::endl;
			  return;
		  }
		  auto similar = state->FindSimilarConVars(argv.front());
		  if(similar.empty()) {
			  Con::COUT << "No potential candidates found!" << Con::endl;
			  return;
		  }
		  Con::COUT << "Found " << similar.size() << " potential candidates:" << Con::endl;
		  for(auto &name : similar)
			  Con::COUT << "- " << name << Con::endl;
	  },
	  console::ConVarFlags::None, "Finds similar console commands to whatever was given as argument.");
}

static void compile_lua_file(lua::State *l, pragma::Game &game, std::string f)
{
	pragma::string::to_lower(f);
	std::string subPath = ufile::get_path_from_filename(f);
	std::string cur = "";
	std::string path = cur + f;
	path = pragma::fs::get_normalized_path(path);
	auto s = game.LoadLuaFile(path);
	if(s != Lua::StatusCode::Ok)
		return;
	if(path.length() > 3 && path.substr(path.length() - 4) == Lua::DOT_FILE_EXTENSION)
		path = path.substr(0, path.length() - 4);
	path += Lua::DOT_FILE_EXTENSION_PRECOMPILED;
	auto r = Lua::compile_file(l, path);
	if(r == false)
		Con::CWAR << "Unable to write file '" << path.c_str() << "'..." << Con::endl;
	else
		Con::COUT << "Successfully compiled as '" << path.c_str() << "'." << Con::endl;
}

static void cmdExit(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float) { pragma::Engine::Get()->ShutDown(); }

static void debug_profiling_print(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	Con::COUT << "-------- CPU-Profiler Query Results --------" << Con::endl;
	Con::COUT << std::left << std::setw(30) << "Stage Name" << std::setw(20) << "Time (ms)" << std::setw(20) << "Time (ns)" << std::setw(10) << "Count" << std::setw(10) << "Thread" << Con::endl;
	Con::COUT << "--------------------------------------------" << Con::endl;

	std::function<void(pragma::debug::ProfilingStage &, const std::string &, bool)> fPrintResults = nullptr;
	fPrintResults = [&fPrintResults](pragma::debug::ProfilingStage &stage, const std::string &indent, bool bRoot) {
		if(bRoot == false) {

			std::string sTimeMs = "Pending";
			std::string sTimeNs = "-";
			std::string sCount = "-";
			std::string sThread = "-";

			auto result = stage.GetResult();
			if(result && result->duration.has_value()) {
				std::chrono::steady_clock::duration accDur {};
				for(auto &hChild : stage.GetChildren()) {
					if(hChild.expired())
						continue;
					auto childRes = hChild.lock()->GetResult();
					if(!childRes || !childRes->duration)
						continue;
					accDur += *childRes->duration;
				}
				auto tMsAcc = pragma::util::clock::to_milliseconds(accDur);
				auto tMs = pragma::util::clock::to_milliseconds(*result->duration);
				sTimeMs = pragma::util::round_string(tMs, 2) + "ms (" + pragma::util::round_string(tMsAcc, 2) + "ms)";
				sTimeNs = std::to_string(result->duration->count()) + "ns";
				sCount = std::to_string(stage.GetCount());

				std::stringstream ss;
				ss << stage.GetThreadId();
				sThread = ss.str();
			}

			Con::COUT << std::left << std::setw(30) << (indent + stage.GetName()) << std::setw(20) << sTimeMs << std::setw(20) << sTimeNs << std::setw(10) << sCount << std::setw(10) << sThread << Con::endl;
		}

		// for(auto &wpChild : stage.GetChildren()) {
		// 	if(wpChild.expired())
		// 		continue;
		// 	fPrintResults(*wpChild.lock(), indent + (bRoot ? "" : "  "), false);
		// }
	};

	auto &profiler = pragma::Engine::Get()->GetProfiler();
	fPrintResults(profiler.GetRootStage(), "", true);

	Con::COUT << "--------------------------------------------" << Con::endl;
}

static void debug_profiling_physics_start(pragma::NetworkState *nw, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *game = nw->GetGameState();
	auto *physEnv = game ? game->GetPhysicsEnvironment() : nullptr;
	if(physEnv == nullptr)
		return;
	physEnv->StartProfiling();
}
static void debug_profiling_physics_end(pragma::NetworkState *nw, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *game = nw->GetGameState();
	auto *physEnv = game ? game->GetPhysicsEnvironment() : nullptr;
	if(physEnv == nullptr)
		return;
	physEnv->EndProfiling();
}

static void debug_dump_scene_graph(pragma::NetworkState *nw)
{
	auto *game = nw->GetGameState();
	if(!game)
		return;
	std::vector<pragma::ecs::BaseEntity *> *ents;
	game->GetEntities(&ents);

	std::vector<pragma::ecs::BaseEntity *> rootEnts;
	rootEnts.reserve(ents->size());
	for(auto *ent : *ents) {
		if(!ent)
			continue;
		auto *childC = ent->GetChildComponent();
		if(!childC || !childC->HasParent())
			rootEnts.push_back(ent);
	}

	std::function<void(pragma::ecs::BaseEntity &, const std::string &, bool)> printGraph = nullptr;
	printGraph = [&printGraph](pragma::ecs::BaseEntity &ent, const std::string &prefix, bool isLast) {
		Con::COUT << prefix;
		if(isLast)
			Con::COUT << "\\-- ";
		else
			Con::COUT << "+-- ";
		Con::COUT << ent;
		Con::COUT << Con::endl;

		auto parentC = ent.GetComponent<pragma::ParentComponent>();
		if(parentC.expired())
			return;
		auto &children = parentC->GetChildren();
		for(size_t i = 0; i < children.size(); ++i) {
			if(children[i].expired())
				continue;
			bool isLastChild = (i == children.size() - 1);
			printGraph(children[i].get()->GetEntity(), prefix + (isLast ? "    " : "|   "), isLastChild);
		}
	};

	Con::COUT << (game->IsClient() ? "Client " : "Server ");
	Con::COUT << "Scene Graph:" << Con::endl;
	for(size_t i = 0; i < rootEnts.size(); ++i) {
		bool isLastRoot = (i == rootEnts.size() - 1);
		printGraph(*rootEnts[i], "", isLastRoot);
	}
	Con::COUT << Con::endl;
}

static void debug_dump_scene_graph(pragma::NetworkState *nw, pragma::BasePlayerComponent *, std::vector<std::string> &, float)
{
	auto *sv = pragma::Engine::Get()->GetServerNetworkState();
	if(sv)
		debug_dump_scene_graph(sv);

	auto *cl = pragma::Engine::Get()->GetClientState();
	if(cl)
		debug_dump_scene_graph(cl);
}

void pragma::Engine::RegisterConsoleCommands()
{
	auto &conVarMap = *console::server::get_convar_map();
	RegisterSharedConsoleCommands(conVarMap);
	// Note: Serverside ConVars HAVE to be registered shared if the command is replicated!
	conVarMap.RegisterConCommand(
	  "map",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::COUT << state->GetMap() << Con::endl;
			  return;
		  }
		  auto map = argv.front();
		  // We can't change the map within a console command callback, so
		  // we'll delay it to a safe point.
		  AddTickEvent([this, map]() { StartDefaultGame(map); });
	  },
	  console::ConVarFlags::None, "Loads the given map immediately. Usage: map <mapName>",
	  [](const std::string &arg, std::vector<std::string> &autoCompleteOptions, bool exactPrefix) {
		  std::vector<std::string> resFiles;
		  auto exts = pragma::asset::get_supported_extensions(asset::Type::Map);
		  for(auto &ext : exts)
			  fs::find_files("maps/*." + ext, &resFiles, nullptr);
		  for(auto &f : resFiles)
			  ufile::remove_extension_from_filename(f, exts);
		  auto it = resFiles.begin();
		  std::vector<std::string_view> similarCandidates {};
		  if(exactPrefix) {
			  for(auto &f : resFiles) {
				  if(!string::compare(f.c_str(), arg.c_str(), false, arg.length()))
					  continue;
				  similarCandidates.push_back(f);
			  }
		  }
		  else {
			  string::gather_similar_elements(
			    arg,
			    [&it, &resFiles]() -> std::optional<std::string_view> {
				    if(it == resFiles.end())
					    return {};
				    auto &name = *it;
				    ++it;
				    return name;
			    },
			    similarCandidates, 15);
		  }

		  autoCompleteOptions.reserve(similarCandidates.size());
		  for(auto &candidate : similarCandidates) {
			  auto strOption = std::string {candidate};
			  ufile::remove_extension_from_filename(strOption);
			  autoCompleteOptions.push_back(strOption);
		  }
	  });
	conVarMap.RegisterConCommand("lua_exec", &console::commands::lua_exec, console::ConVarFlags::None, "Opens and executes a lua-file on the server.", &console::commands::lua_exec_autocomplete);
	conVarMap.RegisterConCommand("lua_run", static_cast<void (*)(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)>(&console::commands::lua_run), console::ConVarFlags::None, "Runs a lua command on the server lua state.",
	  [](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		  auto *sv = get_engine()->GetServerNetworkState();
		  auto *game = sv ? sv->GetGameState() : nullptr;
		  if(!game)
			  return;
		  auto *l = game->GetLuaState();
		  if(!l)
			  return;
		  console::commands::lua_run_autocomplete(l, arg, autoCompleteOptions);
	  });
	conVarMap.RegisterConCommand(
	  "save",
	  [](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  auto *game = state ? state->GetGameState() : nullptr;
		  if(game == nullptr) {
			  Con::CWAR << "Cannot create savegame: No active game!" << Con::endl;
			  return;
		  }
		  auto path = "savegames/" + util::get_date_time("%Y-%m-%d_%H-%M-%S") + ".psav_b";
		  fs::create_path(ufile::get_path_from_filename(path));
		  std::string err;
		  auto result = game::savegame::save(*game, path, err);
		  if(result == false)
			  Con::CWAR << "Cannot create savegame: " << err << Con::endl;
		  else
			  Con::COUT << "Created savegame as '" << path << "'!" << Con::endl;
	  },
	  console::ConVarFlags::None);
	conVarMap.RegisterConCommand(
	  "load",
	  [](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::CWAR << "Cannot load savegame: No savegame specified!" << Con::endl;
			  return;
		  }
		  state->EndGame();
		  state->StartGame(true);
		  auto *game = state ? state->GetGameState() : nullptr;
		  if(game == nullptr) {
			  Con::CWAR << "Cannot load savegame: No active game!" << Con::endl;
			  return;
		  }
		  auto path = "savegames/" + argv.front();
		  if(fs::exists(path + ".psav_b"))
			  path += ".psav_b";
		  else
			  path += ".psav";
		  fs::create_path(ufile::get_path_from_filename(path));
		  std::string err;
		  auto result = game::savegame::load(*game, path, err);
		  if(result == false)
			  Con::CWAR << "Cannot load savegame: " << err << Con::endl;
	  },
	  console::ConVarFlags::None);

	conVarMap.RegisterConCommand(
	  "lua_help",
	  [](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty())
			  return;
		  std::stringstream ss;
		  Lua::doc::print_documentation(argv.front(), ss);
		  Con::COUT << ss.str() << Con::endl;
	  },
	  console::ConVarFlags::None, "Prints information about the specified function, library or enum (or the closest candiate). Usage: lua_help <function/library/enum>.",
	  [](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		  std::vector<const doc::BaseCollectionObject *> candidates {};
		  Lua::doc::find_candidates(arg, candidates, 15u);
		  autoCompleteOptions.reserve(candidates.size());
		  for(auto *pCandidate : candidates)
			  autoCompleteOptions.push_back(pCandidate->GetFullName());
	  });

	conVarMap.RegisterConCommand(
	  "help",
	  [](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::COUT << "Usage: help <cvarname>" << Con::endl;
			  Con::COUT << "Run \"list\" to get a list of all available console commands and variables." << Con::endl;
			  return;
		  }
		  auto *en = get_engine();
		  auto *cl = en ? en->GetClientState() : nullptr;
		  auto *sv = en ? en->GetServerNetworkState() : nullptr;
		  console::ConConf *cv = cl ? cl->GetConVar(argv[0]) : nullptr;
		  if(!cv)
			  cv = sv->GetConVar(argv[0]);
		  if(cv == nullptr) {
			  Con::COUT << "help: no cvar or command named " << argv[0] << Con::endl;
			  return;
		  }
		  cv->Print(argv[0]);
	  },
	  console::ConVarFlags::None, "Find help about a convar/concommand.",
	  [this](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		  auto *cvMap = GetConVarMap();
		  std::vector<std::string_view> similarCandidates {};
		  std::vector<float> similarities {};
		  std::unordered_set<std::string_view> iteratedCvars = {};
		  auto fItConVarMap = [&arg, &similarCandidates, &similarities, &iteratedCvars](console::ConVarMap &cvMap) {
			  auto &conVars = cvMap.GetConVars();
			  auto it = conVars.begin();

			  string::gather_similar_elements(
			    arg,
			    [&it, &conVars, &iteratedCvars]() -> std::optional<std::string_view> {
				    if(it == conVars.end())
					    return {};
				    auto itIterated = iteratedCvars.find(it->first);
				    while(itIterated != iteratedCvars.end() && it != conVars.end()) {
					    ++it;
					    itIterated = iteratedCvars.find(it->first);
				    }
				    if(it == conVars.end())
					    return {};
				    auto &pair = *it;
				    ++it;
				    iteratedCvars.insert(pair.first);
				    return pair.first;
			    },
			    similarCandidates, 15, &similarities);
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
			  autoCompleteOptions.push_back(std::string {candidate});
	  });

	conVarMap.RegisterConCommand("clear_cache", [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) { ClearCache(); }, console::ConVarFlags::None, "Clears all of the cached engine files.");

	conVarMap.RegisterConCommand("loc_reload", [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) { locale::reload_files(); }, console::ConVarFlags::None, "Reloads all localization files.");

	conVarMap.RegisterConCommand(
	  "loc_find",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::CWAR << "No argument specified!" << Con::endl;
			  return;
		  }
		  auto &texts = locale::get_texts();
		  std::vector<std::string> baseTexts;
		  std::vector<std::string> ids;
		  baseTexts.reserve(texts.size());
		  ids.reserve(texts.size());
		  for(auto &pair : texts) {
			  ids.push_back(pair.first);
			  baseTexts.push_back(pair.second.cpp_str());
		  }
		  std::vector<size_t> similarElements {};
		  std::vector<float> similarities {};
		  string::gather_similar_elements(argv.front(), baseTexts, similarElements, 6, &similarities);
		  Con::COUT << "Found " << similarElements.size() << " similar matches:" << Con::endl;
		  for(auto idx : similarElements)
			  Con::COUT << ids[idx] << ": " << baseTexts[idx] << Con::endl;
		  Con::COUT << Con::endl;
	  },
	  console::ConVarFlags::None, "Searches for the specified text in all currently loaded text strings.");

	conVarMap.RegisterConCommand("asset_clear_unused_models", [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) { ClearUnusedAssets(asset::Type::Model, true); }, console::ConVarFlags::None, "Clears all unused models from memory.");
	conVarMap.RegisterConCommand("asset_clear_unused_materials", [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) { ClearUnusedAssets(asset::Type::Material, true); }, console::ConVarFlags::None, "Clears all unused materials from memory.");
	conVarMap.RegisterConCommand(
	  "asset_clear_unused",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  std::vector<asset::Type> types;
		  auto n = math::to_integral(asset::Type::Count);
		  types.reserve(n);
		  for(auto i = decltype(n) {0u}; i < n; ++i)
			  types.push_back(static_cast<asset::Type>(i));
		  ClearUnusedAssets(types, true);
	  },
	  console::ConVarFlags::None, "Clears all unused assets from memory.");
	conVarMap.RegisterConCommand(
	  "install_module",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::CWAR << "No module specified!" << Con::endl;
			  return;
		  }
		  std::optional<std::string> version {};
		  if(argv.size() > 1)
			  version = argv[1];
		  install_binary_module(argv[0], version);
	  },
	  console::ConVarFlags::None, "Install the specified binary module. Usage: install_module <module> <version>. If no version is specified, the latest version will be downloaded.");
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	conVarMap.RegisterConCommand(
	  "debug_vtune_prof_start",
	  [this](pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  std::string name = "cmd";
		  if(!argv.empty())
			  name = argv.front();
		  ::debug::get_domain().BeginTask(name);
	  },
	  pragma::console::ConVarFlags::None, "Start the VTune profiler.");
	conVarMap.RegisterConCommand("debug_vtune_prof_end", [this](pragma::NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) { ::debug::get_domain().EndTask(); }, pragma::console::ConVarFlags::None, "End the VTune profiler.");
#endif
	conVarMap.RegisterConCommand(
	  "log_level_console",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  auto logLevel = get_console_log_level();
			  Con::COUT << "Current console log level: " << magic_enum::enum_name(logLevel) << Con::endl;
			  return;
		  }
		  set_console_log_level(static_cast<util::LogSeverity>(util::to_int(argv[0])));
	  },
	  console::ConVarFlags::None, "Changes the console logging level. Usage: log_level_con <level>. Level can be: 0 = trace, 1 = debug, 2 = info, 3 = warning, 4 = error, 5 = critical, 6 = disabled.");
	conVarMap.RegisterConCommand(
	  "log_level_file",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  auto logLevel = get_file_log_level();
			  Con::COUT << "Current file log level: " << magic_enum::enum_name(logLevel) << Con::endl;
			  return;
		  }
		  set_file_log_level(static_cast<util::LogSeverity>(util::to_int(argv[0])));
	  },
	  console::ConVarFlags::None, "Changes the file logging level. Usage: log_level_file <level>. Level can be: 0 = trace, 1 = debug, 2 = info, 3 = warning, 4 = error, 5 = critical, 6 = disabled.");
	conVarMap.RegisterConCommand(
	  "debug_start_lua_debugger_server_sv",
	  [this](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  auto *l = state->GetLuaState();
		  if(!l) {
			  Con::CWAR << "Unable to start debugger server: No active Lua state!" << Con::endl;
			  return;
		  }
		  Lua::util::start_debugger_server(l);
	  },
	  console::ConVarFlags::None, "Starts the Lua debugger server for the serverside lua state.");

	auto &conVarMapEn = *console::engine::get_convar_map();
	conVarMapEn.RegisterConCommand(
	  "log",
	  [](NetworkState *, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty())
			  return;
		  Get()->WriteToLog(argv[0]);
	  },
	  console::ConVarFlags::None, "Adds the specified message to the engine log. Usage: log <msg>.");
	conVarMapEn.RegisterConCommand(
	  "clear_cache",
	  [](NetworkState *, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty())
			  return;
		  Get()->ClearCache();
	  },
	  console::ConVarFlags::None, "Deletes all cache files.");

	conVarMapEn.RegisterConVar<udm::String>("cache_version", "", console::ConVarFlags::Archive, "The engine version that the cache files are associated with. If this version doesn't match the current engine version, the cache will be cleared.");
	conVarMapEn.RegisterConVar<udm::UInt32>("cache_version_target", 17, console::ConVarFlags::None, "If cache_version does not match this value, the cache files will be cleared and it will be set to it.");
	conVarMapEn.RegisterConVar<udm::Boolean>("debug_profiling_enabled", false, console::ConVarFlags::None, "Enables profiling timers.");
	conVarMapEn.RegisterConVar<udm::Boolean>("debug_disable_animation_updates", false, console::ConVarFlags::None, "Disables animation updates.");
	conVarMapEn.RegisterConVar<udm::Boolean>("sh_mount_external_game_resources", true, console::ConVarFlags::Archive, "If set to 1, the game will attempt to load missing resources from external games.");
	conVarMapEn.RegisterConVar<udm::UInt8>("sh_lua_remote_debugging", 0, console::ConVarFlags::Archive,
	  "0 = Remote debugging is disabled; 1 = Remote debugging is enabled serverside; 2 = Remote debugging is enabled clientside.\nCannot be changed during an active game. Also requires the \"-luaext\" launch parameter.\nRemote debugging cannot be enabled clientside and serverside at the same time.");
	conVarMapEn.RegisterConVar<udm::Boolean>("lua_open_editor_on_error", true, console::ConVarFlags::Archive, "1 = Whenever there's a Lua error, the engine will attempt to automatically open a Lua IDE and open the file and line which caused the error.");
	conVarMapEn.RegisterConVar<udm::Boolean>("steam_steamworks_enabled", true, console::ConVarFlags::Archive, "Enables or disables steamworks.");
	conVarMapEn.RegisterConVarCallback("steam_steamworks_enabled", std::function<void(NetworkState *, const console::ConVar &, bool, bool)> {[](NetworkState *, const console::ConVar &, bool prev, bool val) {
		static std::weak_ptr<util::Library> wpSteamworks = {};
		static std::unique_ptr<ISteamworks> isteamworks = nullptr;
		auto *nwSv = Get()->GetServerNetworkState();
		auto *nwCl = Get()->GetClientState();
		if(val == true) {
			if(wpSteamworks.expired() == false && isteamworks != nullptr)
				return;
			const std::string libSteamworksPath {"steamworks/pr_steamworks"};
			std::shared_ptr<util::Library> libSteamworks = nullptr;
			if(nwSv != nullptr)
				libSteamworks = nwSv->InitializeLibrary(libSteamworksPath);
			if(nwCl != nullptr) {
				auto libCl = nwCl->InitializeLibrary(libSteamworksPath);
				if(libSteamworks == nullptr)
					libSteamworks = libCl;
			}
			if(libSteamworks != nullptr) {
				isteamworks = std::make_unique<ISteamworks>(*libSteamworks);
				if(isteamworks->initialize() == true) {
					isteamworks->subscribe_item(1684401267); // Automatically subscribe to pragma demo addon
					isteamworks->update_subscribed_items();
					if(nwSv != nullptr)
						nwSv->CallCallbacks<void, std::reference_wrapper<ISteamworks>>("OnSteamworksInitialized", *isteamworks);
					if(nwCl != nullptr)
						nwCl->CallCallbacks<void, std::reference_wrapper<ISteamworks>>("OnSteamworksInitialized", *isteamworks);
				}
				else
					isteamworks = nullptr;
			}
			else
				isteamworks = nullptr;
			wpSteamworks = libSteamworks;
			return;
		}
		if(wpSteamworks.expired() || isteamworks == nullptr)
			return;
		isteamworks->shutdown();
		wpSteamworks = {};
		isteamworks = nullptr;
		if(nwSv != nullptr)
			nwSv->CallCallbacks<void>("OnSteamworksShutdown");
		if(nwCl != nullptr)
			nwCl->CallCallbacks<void>("OnSteamworksShutdown");
	}});

	conVarMapEn.RegisterConVarCallback("sh_mount_external_game_resources", std::function<void(NetworkState *, const console::ConVar &, bool, bool)> {[](NetworkState *, const console::ConVar &, bool prev, bool val) { Get()->SetMountExternalGameResources(val); }});
	conVarMapEn.RegisterConCommand(
	  "toggle",
	  [](NetworkState *nw, BasePlayerComponent *pl, std::vector<std::string> &argv, float) {
		  if(argv.empty() == true)
			  return;
		  auto &cvName = argv.front();
		  auto *cf = Get()->GetConVar(cvName);
		  if(cf == nullptr || cf->GetType() != console::ConType::Var)
			  return;
		  auto *cvar = static_cast<console::ConVar *>(cf);
		  std::vector<std::string> args = {(cvar->GetBool() == true) ? "0" : "1"};
		  Get()->RunConsoleCommand(cvName, args);
	  },
	  console::ConVarFlags::None, "Toggles the specified console variable between 0 and 1.");

	////////////////////////////////
	////////////////////////////////

	conVarMapEn.RegisterConCommand(
	  "lua_compile",
	  +[](NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty() || !state->IsGameActive())
			  return;
		  Game *game = state->GetGameState();
		  auto *l = game->GetLuaState();
		  std::string arg = argv[0];
		  if(fs::is_dir(Lua::SCRIPT_DIRECTORY_SLASH + arg)) {
			  std::function<void(const std::string &)> fCompileFiles = nullptr;
			  fCompileFiles = [l, game, &fCompileFiles](const std::string &path) {
				  std::vector<std::string> files {};
				  std::vector<std::string> dirs {};
				  fs::find_files((Lua::SCRIPT_DIRECTORY_SLASH + path + "/*"), &files, &dirs);
				  for(auto &f : files) {
					  std::string ext;
					  if(ufile::get_extension(f, &ext) == false || pragma::string::compare<std::string>(ext, Lua::FILE_EXTENSION, false) == false)
						  continue;
					  compile_lua_file(l, *game, path + '/' + f);
				  }
				  for(auto &d : dirs)
					  fCompileFiles(path + '/' + d);
			  };
			  fCompileFiles(arg);
			  return;
		  }
		  compile_lua_file(l, *game, arg);
	  },
	  console::ConVarFlags::None, "Opens the specified lua-file and outputs a precompiled file with the same name (And the extension '" + Lua::DOT_FILE_EXTENSION_PRECOMPILED + "').");

	conVarMapEn.RegisterConCommand(
	  "toggleconsole",
	  [](NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float) {
		  if(Get()->IsServerOnly())
			  return;
		  if(Get()->IsConsoleOpen())
			  Get()->CloseConsole();
		  else
			  Get()->OpenConsole();
	  },
	  console::ConVarFlags::None, "Toggles the developer console.");

	conVarMapEn.RegisterConCommand(
	  "echo",
	  [](NetworkState *, BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty())
			  return;
		  Con::COUT << argv[0] << Con::endl;
	  },
	  console::ConVarFlags::None, "Prints something to the console. Usage: echo <message>");

	conVarMapEn.RegisterConCommand("exit", cmdExit, console::ConVarFlags::None, "Exits the game.");
	conVarMapEn.RegisterConCommand("quit", cmdExit, console::ConVarFlags::None, "Exits the game.");

	conVarMapEn.RegisterConCommand(
	  "listmaps",
	  [](NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float) {
		  std::vector<std::string> resFiles;
		  auto exts = pragma::asset::get_supported_extensions(asset::Type::Map);
		  for(auto &ext : exts)
			  fs::find_files("maps/*." + ext, &resFiles, nullptr);
		  for(auto &f : resFiles)
			  ufile::remove_extension_from_filename(f, exts);
		  for(auto &f : resFiles)
			  Con::COUT << f << Con::endl;
	  },
	  console::ConVarFlags::None, "");

	conVarMapEn.RegisterConCommand("clear", [](NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float) { Get()->ClearConsole(); }, console::ConVarFlags::None, "Clears everything in the console.");

	conVarMapEn.RegisterConCommand(
	  "credits",
	  [](NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float) {
		  Con::COUT << "Silverlan" << Con::endl;
		  Con::COUT << "Contact: " << engine_info::get_author_mail_address() << Con::endl;
		  Con::COUT << "Website: " << engine_info::get_website_url() << Con::endl;
	  },
	  console::ConVarFlags::None, "Prints a list of developers.");

	conVarMapEn.RegisterConCommand("version", [](NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float) { Con::COUT << get_pretty_engine_version() << Con::endl; }, console::ConVarFlags::None, "Prints the current engine version to the console.");
	conVarMapEn.RegisterConCommand("debug_profiling_print", debug_profiling_print, console::ConVarFlags::None, "Prints the last profiled times.");
	conVarMapEn.RegisterConCommand("debug_profiling_physics_start", debug_profiling_physics_start, console::ConVarFlags::None, "Prints physics profiling information for the last simulation step.");
	conVarMapEn.RegisterConCommand("debug_profiling_physics_end", debug_profiling_physics_end, console::ConVarFlags::None, "Prints physics profiling information for the last simulation step.");
	conVarMapEn.RegisterConCommand("debug_dump_scene_graph", static_cast<void (*)(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)>(debug_dump_scene_graph), console::ConVarFlags::None, "Prints the game scene graph.");

	conVarMap.RegisterConVarCallback("asset_multithreading_enabled", std::function<void(NetworkState *, const console::ConVar &, bool, bool)> {[this](NetworkState *nw, const console::ConVar &cv, bool oldVal, bool newVal) -> void {
		if(Get() == nullptr)
			return;
		Get()->SetProfilingEnabled(newVal);
	}});
}

class ModuleInstallJob : public pragma::util::ParallelWorker<bool> {
  public:
	ModuleInstallJob(const std::string &module, const std::optional<std::string> &version) : m_module {module}, m_version {version}
	{
		AddThread([this]() { Install(); });
	}
	virtual bool GetResult() override { return m_success; }
  private:
	virtual void DoCancel(const std::string &resultMsg, std::optional<int32_t> resultCode) override { m_curl.CancelDownload(); }
	void Install();
	template<typename TJob, typename... TARGS>
	friend pragma::util::ParallelJob<typename TJob::RESULT_TYPE> pragma::util::create_parallel_job(TARGS &&...args);

	std::atomic<bool> m_success = false;
	std::chrono::steady_clock::time_point m_lastProgressTime;
	pragma::CurlQueryHandler m_curl {};
	std::string m_module;
	std::optional<std::string> m_version {};
};

void ModuleInstallJob::Install()
{
	std::stringstream archiveName;
	archiveName << "binaries";
	archiveName << "-" << pragma::engine_info::get_platform_name();
	archiveName << "-" << pragma::engine_info::get_architecture_name();
	archiveName << "-" << pragma::engine_info::get_compiler_name();
#ifdef _WIN32
	archiveName << ".zip";
#else
	archiveName << ".tar.gz";
#endif
	std::string url = std::string {"https://github.com/"} + m_module + "/releases/download/";
	if(m_version.has_value())
		url += "v" + *m_version;
	else
		url += "latest";
	url += "/" + archiveName.str();

#ifdef __linux__
	{
		Con::CWAR << "Automatic installation of binary modules using the 'install_module' console command is currently not supported on Linux! You will have to install the module manually." << Con::endl;
		Con::CWAR << "The download should automatically start through your browser. If not, you can download the module here: " << url << Con::endl;
		Con::CWAR << "Once downloaded, simply extract the archive over your Pragma installation." << Con::endl;
		pragma::util::open_url_in_browser(url);
		UpdateProgress(1.f);
		SetStatus(pragma::util::JobStatus::Successful);
		return;
	}
#endif

	pragma::fs::create_directory("temp");
	auto archivePath = "temp/" + archiveName.str();
	m_lastProgressTime = std::chrono::steady_clock::now();
	Con::COUT << "Downloading module from '" << url << "'..." << Con::endl;
	m_curl.AddResource(
	  url, archivePath,
	  [this](int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow) {
		  if(dltotal == 0)
			  return;
		  auto t = std::chrono::steady_clock::now();
		  auto dt = t - m_lastProgressTime;
		  if(dt < std::chrono::seconds {5})
			  return;
		  m_lastProgressTime = t;
		  auto fprogress = dlnow / static_cast<float>(dltotal);
		  auto progress = pragma::util::round_string(fprogress * 100.f, 2);
		  Con::COUT << "Module download at " << progress << "%" << Con::endl;
		  UpdateProgress(fprogress * 0.9f);
	  },
	  [this, archivePath](int code) {
		  UpdateProgress(0.9f);
		  if(code == 0) {
			  std::string err;
			  auto zip = uzip::ZIPFile::Open(archivePath, err, uzip::OpenMode::Read);
			  if(!zip) {
				  std::string msg = "Failed to open module archive '" + archivePath + "': " + err;
				  Con::CWAR << "" << msg << Con::endl;
				  SetStatus(pragma::util::JobStatus::Failed, msg);
				  return;
			  }

			  Con::COUT << "Extracting module archive '" << archivePath << "'..." << Con::endl;
			  err = {};
			  if(!zip->ExtractFiles(pragma::fs::get_program_write_path(), err)) {
				  std::string msg = "Failed to extract module archive '" + archivePath + "'!";
				  Con::CWAR << "" << msg << Con::endl;
				  SetStatus(pragma::util::JobStatus::Failed, msg);
				  return;
			  }
		  }
		  else {
			  std::string msg = "Failed to download module '" + m_module + "'!";
			  Con::CWAR << "" << msg << Con::endl;
			  SetStatus(pragma::util::JobStatus::Failed, msg);
			  return;
		  }
		  pragma::fs::remove_file(archivePath);

		  UpdateProgress(1.f);
		  SetStatus(pragma::util::JobStatus::Successful);
		  Con::COUT << "Binary module '" << archivePath << "' has been installed successfully!" << Con::endl;
	  });
	m_curl.StartDownload();
	while(!m_curl.IsComplete())
		std::this_thread::sleep_for(std::chrono::milliseconds {500});
}

void install_binary_module(const std::string &module, const std::optional<std::string> &version)
{
	auto job = pragma::util::create_parallel_job<ModuleInstallJob>(module, version);
	job.Start();
	pragma::get_engine()->AddParallelJob(job, "Install Binary Module '" + module + "'");
}
