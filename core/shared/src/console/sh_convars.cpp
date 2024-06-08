/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/environment.hpp"
#include <pragma/engine.h>
#include <pragma/console/convars.h>
#include <pragma/console/s_convars.h>
#include <pragma/console/c_convars.h>
#include <pragma/lua/luaapi.h>
#include <pragma/game/game.h>
#include <fsys/filesystem.h>
#include <mathutil/uvec.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util_file.h>
#include <pragma/engine_version.h>
#include <pragma/asset/util_asset.hpp>
#include <map>

#define DLLSPEC_ISTEAMWORKS DLLNETWORK
#include "pragma/game/isteamworks.hpp"

extern DLLNETWORK Engine *engine;

//////////////// LOGGING ////////////////
REGISTER_ENGINE_CONCOMMAND(
  log,
  [](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv) {
	  if(argv.empty())
		  return;
	  engine->WriteToLog(argv[0]);
  },
  ConVarFlags::None, "Adds the specified message to the engine log. Usage: log <msg>.");
REGISTER_ENGINE_CONCOMMAND(
  clear_cache,
  [](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv) {
	  if(argv.empty())
		  return;
	  engine->ClearCache();
  },
  ConVarFlags::None, "Deletes all cache files.");
REGISTER_ENGINE_CONVAR(cache_version, udm::Type::String, "", ConVarFlags::Archive, "The engine version that the cache files are associated with. If this version doesn't match the current engine version, the cache will be cleared.");
REGISTER_ENGINE_CONVAR(cache_version_target, udm::Type::UInt32, "12", ConVarFlags::None, "If cache_version does not match this value, the cache files will be cleared and it will be set to it.");
REGISTER_ENGINE_CONVAR(debug_profiling_enabled, udm::Type::Boolean, "0", ConVarFlags::None, "Enables profiling timers.");
REGISTER_ENGINE_CONVAR(sh_mount_external_game_resources, udm::Type::Boolean, "1", ConVarFlags::Archive, "If set to 1, the game will attempt to load missing resources from external games.");
REGISTER_ENGINE_CONVAR(sh_lua_remote_debugging, udm::Type::UInt8, "0", ConVarFlags::Archive,
  "0 = Remote debugging is disabled; 1 = Remote debugging is enabled serverside; 2 = Remote debugging is enabled clientside.\nCannot be changed during an active game. Also requires the \"-luaext\" launch parameter.\nRemote debugging cannot be enabled clientside and serverside at the same time.");
REGISTER_ENGINE_CONVAR(lua_open_editor_on_error, udm::Type::Boolean, "1", ConVarFlags::Archive, "1 = Whenever there's a Lua error, the engine will attempt to automatically open a Lua IDE and open the file and line which caused the error.");
REGISTER_ENGINE_CONVAR(steam_steamworks_enabled, udm::Type::Boolean, "1", ConVarFlags::Archive, "Enables or disables steamworks.");
static void cvar_steam_steamworks_enabled(bool val)
{
	static std::weak_ptr<util::Library> wpSteamworks = {};
	static std::unique_ptr<ISteamworks> isteamworks = nullptr;
	auto *nwSv = engine->GetServerNetworkState();
	auto *nwCl = engine->GetClientState();
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
}
REGISTER_ENGINE_CONVAR_CALLBACK(steam_steamworks_enabled, [](NetworkState *, const ConVar &, bool prev, bool val) { cvar_steam_steamworks_enabled(val); });

REGISTER_ENGINE_CONVAR_CALLBACK(sh_mount_external_game_resources, [](NetworkState *, const ConVar &, bool prev, bool val) { engine->SetMountExternalGameResources(val); });
REGISTER_ENGINE_CONCOMMAND(
  toggle,
  [](NetworkState *nw, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv) {
	  if(argv.empty() == true)
		  return;
	  auto &cvName = argv.front();
	  auto *cf = engine->GetConVar(cvName);
	  if(cf == nullptr || cf->GetType() != ConType::Var)
		  return;
	  auto *cvar = static_cast<ConVar *>(cf);
	  std::vector<std::string> args = {(cvar->GetBool() == true) ? "0" : "1"};
	  engine->RunConsoleCommand(cvName, args);
  },
  ConVarFlags::None, "Toggles the specified console variable between 0 and 1.");

REGISTER_ENGINE_CONVAR_CALLBACK(log_enabled, [](NetworkState *, const ConVar &, int prev, int val) {
	//if(!engine->IsActiveState(state))
	//	return;
	if(prev == 0 && val != 0)
		engine->StartLogging();
	else if(prev != 0 && val == 0)
		engine->EndLogging();
});

REGISTER_ENGINE_CONVAR_CALLBACK(log_file, [](NetworkState *state, const ConVar &, std::string prev, std::string val) {
	//if(!engine->IsActiveState(state))
	//	return;
	std::string lprev = prev;
	std::string lval = val;
	std::transform(lprev.begin(), lprev.end(), lprev.begin(), ::tolower);
	std::transform(lval.begin(), lval.end(), lval.begin(), ::tolower);
	if(lprev == lval)
		return;
	if(!state->GetConVarBool("log_enabled"))
		return;
	engine->StartLogging();
});

REGISTER_SHARED_CONVAR_CALLBACK(sv_gravity, [](NetworkState *state, const ConVar &, std::string prev, std::string val) {
	if(!state->IsGameActive())
		return;
	Vector3 gravity = uvec::create(val);
	Game *game = state->GetGameState();
	game->SetGravity(gravity);
});

////////////////////////////////
////////////////////////////////

static void compile_lua_file(lua_State *l, Game &game, std::string f)
{
	StringToLower(f);
	std::string subPath = ufile::get_path_from_filename(f);
	std::string cur = "";
	std::string path = cur + f;
	path = FileManager::GetNormalizedPath(path);
	auto s = game.LoadLuaFile(path);
	if(s != Lua::StatusCode::Ok)
		return;
	if(path.length() > 3 && path.substr(path.length() - 4) == Lua::DOT_FILE_EXTENSION)
		path = path.substr(0, path.length() - 4);
	path += Lua::DOT_FILE_EXTENSION_PRECOMPILED;
	auto r = Lua::compile_file(l, path);
	if(r == false)
		Con::cwar << "Unable to write file '" << path.c_str() << "'..." << Con::endl;
	else
		Con::cout << "Successfully compiled as '" << path.c_str() << "'." << Con::endl;
}

static void CMD_lua_compile(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty() || !state->IsGameActive())
		return;
	Game *game = state->GetGameState();
	auto *l = game->GetLuaState();
	std::string arg = argv[0];
	if(FileManager::IsDir(Lua::SCRIPT_DIRECTORY_SLASH + arg)) {
		std::function<void(const std::string &)> fCompileFiles = nullptr;
		fCompileFiles = [l, game, &fCompileFiles](const std::string &path) {
			std::vector<std::string> files {};
			std::vector<std::string> dirs {};
			FileManager::FindFiles((Lua::SCRIPT_DIRECTORY_SLASH + path + "/*").c_str(), &files, &dirs);
			for(auto &f : files) {
				std::string ext;
				if(ufile::get_extension(f, &ext) == false || ustring::compare<std::string>(ext, Lua::FILE_EXTENSION, false) == false)
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
}
REGISTER_ENGINE_CONCOMMAND(lua_compile, CMD_lua_compile, ConVarFlags::None, "Opens the specified lua-file and outputs a precompiled file with the same name (And the extension '" + Lua::DOT_FILE_EXTENSION_PRECOMPILED + "').");

REGISTER_ENGINE_CONCOMMAND(
  toggleconsole,
  [](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) {
	  if(engine->IsServerOnly())
		  return;
	  if(engine->IsConsoleOpen())
		  engine->CloseConsole();
	  else
		  engine->OpenConsole();
  },
  ConVarFlags::None, "Toggles the developer console.");

REGISTER_ENGINE_CONCOMMAND(
  echo,
  [](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv) {
	  if(argv.empty())
		  return;
	  Con::cout << argv[0] << Con::endl;
  },
  ConVarFlags::None, "Prints something to the console. Usage: echo <message>");

static void CMD_exit(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { engine->ShutDown(); }
REGISTER_ENGINE_CONCOMMAND(exit, CMD_exit, ConVarFlags::None, "Exits the game.");
REGISTER_ENGINE_CONCOMMAND(quit, CMD_exit, ConVarFlags::None, "Exits the game.");

REGISTER_SHARED_CONCOMMAND(
  list,
  [](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &) {
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
			  Con::cout << *it << Con::endl;
	  }
  },
  ConVarFlags::None, "Prints a list of all serverside console commands to the console.");

REGISTER_SHARED_CONCOMMAND(
  find,
  [](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv) {
	  if(argv.empty()) {
		  Con::cwar << "No argument given!" << Con::endl;
		  return;
	  }
	  auto similar = state->FindSimilarConVars(argv.front());
	  if(similar.empty()) {
		  Con::cout << "No potential candidates found!" << Con::endl;
		  return;
	  }
	  Con::cout << "Found " << similar.size() << " potential candidates:" << Con::endl;
	  for(auto &name : similar)
		  Con::cout << "- " << name << Con::endl;
  },
  ConVarFlags::None, "Finds similar console commands to whatever was given as argument.");

REGISTER_ENGINE_CONCOMMAND(
  listmaps,
  [](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) {
	  std::vector<std::string> resFiles;
	  auto exts = pragma::asset::get_supported_extensions(pragma::asset::Type::Map);
	  for(auto &ext : exts)
		  filemanager::find_files("maps/*." + ext, &resFiles, nullptr);
	  for(auto &f : resFiles)
		  ufile::remove_extension_from_filename(f, exts);
	  for(auto &f : resFiles)
		  Con::cout << f << Con::endl;
  },
  ConVarFlags::None, "");

REGISTER_ENGINE_CONCOMMAND(
  clear, [](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { engine->ClearConsole(); }, ConVarFlags::None, "Clears everything in the console.");

REGISTER_ENGINE_CONCOMMAND(
  credits,
  [](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) {
	  Con::cout << "Silverlan" << Con::endl;
	  Con::cout << "Contact: " << engine_info::get_author_mail_address() << Con::endl;
	  Con::cout << "Website: " << engine_info::get_website_url() << Con::endl;
  },
  ConVarFlags::None, "Prints a list of developers.");

REGISTER_ENGINE_CONCOMMAND(
  version, [](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { Con::cout << get_pretty_engine_version() << Con::endl; }, ConVarFlags::None, "Prints the current engine version to the console.");

static void debug_profiling_print(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	Con::cout << "-------- CPU-Profiler Query Results --------" << Con::endl;
	std::function<void(pragma::debug::ProfilingStage &, const std::string &, bool)> fPrintResults = nullptr;
	fPrintResults = [&fPrintResults](pragma::debug::ProfilingStage &stage, const std::string &t, bool bRoot) {
		if(bRoot == false) {
			std::string sTime = "Pending";
			auto result = stage.GetResult();
			if(result && result->duration.has_value()) {
				auto t = util::clock::to_milliseconds(*result->duration);
				sTime = util::round_string(t, 2) + " ms";
				sTime += " (" + std::to_string(result->duration->count()) + " ns)";
			}
			Con::cout << t << stage.GetName() << ": " << sTime << Con::endl;
		}
		for(auto &wpChild : stage.GetChildren()) {
			if(wpChild.expired())
				continue;
			fPrintResults(*wpChild.lock(), t + (bRoot ? "" : "\t"), false);
		}
	};
	auto &profiler = engine->GetProfiler();
	fPrintResults(profiler.GetRootStage(), "", true);
	Con::cout << "--------------------------------------------" << Con::endl;
}
REGISTER_ENGINE_CONCOMMAND(debug_profiling_print, debug_profiling_print, ConVarFlags::None, "Prints the last profiled times.");

static void debug_profiling_physics_start(NetworkState *nw, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto *game = nw->GetGameState();
	auto *physEnv = game ? game->GetPhysicsEnvironment() : nullptr;
	if(physEnv == nullptr)
		return;
	physEnv->StartProfiling();
}
REGISTER_ENGINE_CONCOMMAND(debug_profiling_physics_start, debug_profiling_physics_start, ConVarFlags::None, "Prints physics profiling information for the last simulation step.");

static void debug_profiling_physics_end(NetworkState *nw, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto *game = nw->GetGameState();
	auto *physEnv = game ? game->GetPhysicsEnvironment() : nullptr;
	if(physEnv == nullptr)
		return;
	physEnv->EndProfiling();
}
REGISTER_ENGINE_CONCOMMAND(debug_profiling_physics_end, debug_profiling_physics_end, ConVarFlags::None, "Prints physics profiling information for the last simulation step.");

//////////////// SERVER ////////////////

REGISTER_SHARED_CONVAR(rcon_password, udm::Type::String, "", ConVarFlags::Password, "Specifies a password which can be used to run console commands remotely on a server. If no password is specified, this feature is disabled.");
