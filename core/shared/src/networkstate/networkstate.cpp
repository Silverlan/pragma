/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/console/convars.h>
#include <pragma/engine.h>
#include "materialmanager.h"
#include "pragma/console/convarhandle.h"
#include <sharedutils/functioncallback.h>
#include <pragma/level/mapinfo.h>
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/addonsystem/addonsystem.h"
#include "pragma/audio/soundscript.h"
#include <sharedutils/util_string.h>
#include <sharedutils/scope_guard.h>
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/util/resource_watcher.h"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/model/modelmanager.h"
#include "pragma/debug/intel_vtune.hpp"
#include "pragma/debug/debug_performance_profiler.hpp"
#include <material_manager2.hpp>
#include <pragma/console/s_cvar_global_functions.h>
#include <pragma/lua/luaapi.h>
#include <luainterface.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>
#include <pragma/util/util_module.hpp>
#include "pragma/asset/util_asset.hpp"
#include "pragma/logging.hpp"

#define DLLSPEC_ISTEAMWORKS DLLNETWORK
#include "pragma/game/isteamworks.hpp"

ConVarHandle NetworkState::GetConVarHandle(std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &ptrs, std::string scvar) { return CVarHandler::GetConVarHandle(ptrs, scvar); }

UInt8 NetworkState::STATE_COUNT = 0;

decltype(NetworkState::s_loadedLibraries) NetworkState::s_loadedLibraries = {};
extern DLLNETWORK Engine *engine;

NetworkState::NetworkState() : CallbackHandler(), CVarHandler()
{
	m_ctReal.Reset(static_cast<int64_t>(engine->GetTickCount()));
	m_tReal = CDouble(m_ctReal());
	m_tLast = m_tReal;
	m_tDelta = 0;
	if(STATE_COUNT == 0) {
		AddonSystem::MountAddons();
		GameModeManager::Initialize();
	}
	STATE_COUNT++;

	RegisterCallback<void>("Think");
	RegisterCallback<void>("Tick");
	RegisterCallback<void>("OnClose");

	RegisterCallback<void, std::reference_wrapper<struct ISteamworks>>("OnSteamworksInitialized");
	RegisterCallback<void>("OnSteamworksShutdown");

	m_cbProfilingHandle = engine->AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		std::string postFix = IsClient() ? " (CL)" : " (SV)";
		auto &cpuProfiler = engine->GetProfiler();
		m_profilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage>>();
		m_profilingStageManager->InitializeProfilingStageManager(cpuProfiler);
	});
}
NetworkState::~NetworkState()
{
	STATE_COUNT--;
	if(STATE_COUNT == 0)
		AddonSystem::UnmountAddons();
	m_soundScriptManager = nullptr;
	m_mapInfo = nullptr;

	m_soundsPrecached.clear();

	for(unsigned int i = 0; i < m_thinkCallbacks.size(); i++) {
		if(m_thinkCallbacks[i].IsValid())
			m_thinkCallbacks[i].Remove();
	}
	for(unsigned int i = 0; i < m_tickCallbacks.size(); i++) {
		if(m_tickCallbacks[i].IsValid())
			m_tickCallbacks[i].Remove();
	}
	if(m_cbProfilingHandle.IsValid())
		m_cbProfilingHandle.Remove();

	m_libHandles.clear();
	for(auto it = s_loadedLibraries.begin(); it != s_loadedLibraries.end();) {
		auto &hLib = it->second.library;
		if(hLib.use_count() > 1) {
			++it;
			continue; // Library is still in use by another network state
		}
		auto *ptrDetach = (*hLib)->FindSymbolAddress<void (*)()>("pragma_detach");
		if(ptrDetach != nullptr)
			ptrDetach();
		it = s_loadedLibraries.erase(it);
	}

	for(auto &hCb : m_luaEnumRegisterCallbacks) {
		if(hCb.IsValid() == false)
			continue;
		hCb.Remove();
	}
}

std::vector<CallbackHandle> &NetworkState::GetLuaEnumRegisterCallbacks() { return m_luaEnumRegisterCallbacks; }

ResourceWatcherManager &NetworkState::GetResourceWatcher() { return *m_resourceWatcher; }

bool NetworkState::ShouldRemoveSound(ALSound &snd) { return snd.IsPlaying() == false; }

void NetworkState::UpdateSounds(std::vector<std::shared_ptr<ALSound>> &sounds)
{
	for(auto i = static_cast<int32_t>(sounds.size()) - 1; i >= 0; --i) {
		auto psnd = sounds.at(i);
		if(psnd != nullptr) {
			auto &snd = *psnd;
			snd.Update();
			if(psnd.use_count() <= 2 && ShouldRemoveSound(snd)) {
				psnd = nullptr; // Cleanup occurs in "OnDestroyed" callback
				continue;
			}
		}
	}
	for(auto &snd : sounds) {
		if(snd == nullptr)
			continue;
		snd->PostUpdate();
	}
}

bool NetworkState::PortMaterial(const std::string &path)
{
	auto pathWithoutExt = path;
	auto extensions = pragma::asset::get_supported_extensions(pragma::asset::Type::Material);
	extensions.push_back("vmt");
	extensions.push_back("vmat_c");
	ufile::remove_extension_from_filename(pathWithoutExt, extensions);

	// TODO: This doesn't belong here! Move it into the source module
	auto matPath = pathWithoutExt + ".vmat_c";
	if(util::port_file(this, "materials/" + matPath) == false) {
		matPath = pathWithoutExt + ".vmt";
		if(util::port_file(this, "materials/" + matPath) == false)
			return false;
	}

	auto *mat = LoadMaterial(matPath, true);
	if(mat && mat->GetDataBlock()) {
		// Port textures as well
		std::function<void(const std::shared_ptr<ds::Block> &)> fPortTextures = nullptr;
		fPortTextures = [this, &fPortTextures](const std::shared_ptr<ds::Block> &block) {
			auto *data = block->GetData();
			if(data == nullptr)
				return;
			for(auto &pair : *data) {
				auto v = pair.second;
				if(v->IsBlock() == true)
					fPortTextures(std::static_pointer_cast<ds::Block>(v));
				else {
					auto dataTex = std::dynamic_pointer_cast<ds::Texture>(v);
					if(dataTex) {
						auto path = "materials/" + dataTex->GetValue().name;
						if(FileManager::Exists(path) == false && util::port_file(this, path + ".vtf") == false && util::port_file(this, path + ".vtex_c") == false)
							Con::cwar << "Unable to port texture '" << dataTex->GetValue().name << "'!" << Con::endl;
					}
				}
			}
		};
		fPortTextures(mat->GetDataBlock());
	}
	return (mat != nullptr) ? true : false;
}

util::FileAssetManager *NetworkState::GetAssetManager(pragma::asset::Type type)
{
	switch(type) {
	case pragma::asset::Type::Model:
		return &GetModelManager();
	case pragma::asset::Type::Material:
		return &GetMaterialManager();
	}
	return nullptr;
}

void NetworkState::ClearGameConVars()
{
#pragma message("TODO: What about ConVars that have been created ingame?")
	for(auto it = m_conVars.begin(); it != m_conVars.end();) {
		if(it->second->GetType() == ConType::LuaCmd)
			it = m_conVars.erase(it);
		else
			++it;
	}
}

bool NetworkState::CheatsEnabled() const
{
	if(!IsMultiPlayer())
		return true;
	return engine->GetConVarBool("sv_cheats");
}

Material *NetworkState::PrecacheMaterial(const std::string &path) { return LoadMaterial(path, true, false); }
Material *NetworkState::LoadMaterial(const std::string &path, bool bReload) { return LoadMaterial(path, false, bReload); }

Material *NetworkState::LoadMaterial(const std::string &path, bool precache, bool bReload)
{
	static auto bSkipPort = false;
	auto &matManager = GetMaterialManager();
	auto success = true;
	Material *mat = nullptr;
	if(precache) {
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().BeginTask("preload_material");
#endif
		success = matManager.PreloadAsset(path);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().EndTask();
#endif
		if(success)
			return nullptr;
	}
	else {
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().BeginTask("load_material");
#endif
		auto asset = bReload ? matManager.ReloadAsset(path) : matManager.LoadAsset(path);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		debug::get_domain().EndTask();
#endif
		success = (asset != nullptr);
		mat = asset.get();
	}
	if(!success) {
		if(bSkipPort == false) {
			auto vmtPath = path;
			ufile::remove_extension_from_filename(vmtPath);
			vmtPath += ".vmt";
			if(util::port_file(this, "materials/" + vmtPath) == true) {
				bSkipPort = true;
				mat = LoadMaterial(path, false);
				bSkipPort = false;
				if(mat != nullptr) {

					// TODO: Load textures
				}
				return mat;
			}
		}
		Con::cwar << "Unable to load material '" << path << "': File not found!" << Con::endl;
	}
	return mat;
}

MapInfo *NetworkState::GetMapInfo() { return m_mapInfo.get(); }
std::string NetworkState::GetMap()
{
	if(m_mapInfo == nullptr)
		return "";
	return m_mapInfo->name;
}

float NetworkState::GetSoundDuration(std::string snd)
{
	auto it = m_soundsPrecached.find(snd);
	if(it == m_soundsPrecached.end())
		return 0.0f;
	return it->second->duration;
}

void NetworkState::StartNewGame(const std::string &map, bool singlePlayer)
{
	if(!IsGameActive()) {
		spdlog::info("Starting new {} game...", singlePlayer ? "single-player" : "multi-player");
		StartGame(singlePlayer);
	}
	ChangeLevel(map);
}

void NetworkState::ChangeLevel(const std::string &map)
{
	spdlog::info("Changing map to '{}'...", map);
	m_mapInfo = std::make_unique<MapInfo>();
	m_mapInfo->name = map;
	Game *game = GetGameState();
	if(IsServer()) // Hack: These are already called clientside in ClientState::HandleReceiveGameInfo
	{
		game->CallCallbacks("OnPreLoadMap");
		game->CallLuaCallbacks("OnPreLoadMap");
	}
	game->LoadMap(map);
}

void NetworkState::Close()
{
	EndGame();
	CallCallbacks("OnClose");
}

lua_State *NetworkState::GetLuaState()
{
	if(!IsGameActive())
		return NULL;
	return GetGameState()->GetLuaState();
}

void NetworkState::StartGame(bool singlePlayer)
{
	if(IsGameActive())
		EndGame();
}

void NetworkState::EndGame() { ClearGameConVars(); }

bool NetworkState::IsGameActive() { return false; }

Game *NetworkState::GetGameState() { return m_game.get(); }

void NetworkState::Initialize()
{
	CVarHandler::Initialize();
	if(IsClient())
		Con::cout << "Initializing client state..." << Con::endl;
	else {
		Con::cout << "Initializing server state..." << Con::endl;
		if(engine->IsServerOnly()) {
			Con::cout << "If you encounter problems, such as the server not showing up in the server browser, or clients not being able to connect to it, please make sure the following ports are forwarded:" << Con::endl;
			Con::cout << engine_info::DEFAULT_SERVER_PORT << " (TCP): Required if the boost asio networking layer is used" << Con::endl;
			Con::cout << engine_info::DEFAULT_SERVER_PORT << " (UDP): Required for clients to be able to connect to the server" << Con::endl;
			Con::cout << engine_info::DEFAULT_QUERY_PORT << " (UDP): Required for the server to be registered with the master server and show up in the server browser" << Con::endl;
			Con::cout << Con::endl << "Here's a list of useful console commands:" << Con::endl;
			Con::cout << "map <mapName>: Loads the specified map and starts the server." << Con::endl;
			Con::cout << "sv_maxplayers <maxPlayers>: Specifies the maximum number of players that can play on the server at a time." << Con::endl;
			Con::cout << "sv_gamemode <gameMode>: The gamemode to use. Has to be specified before a map is loaded." << Con::endl;
			Con::cout << "net_library <netLib>: The networking library module to use. The name has to match one of the modules located in the 'pragma/modules/networking' directory." << Con::endl;
			Con::cout << "sv_use_p2p_if_available <1/0>: Only has an effect if the steam networking module is used. Network communication will be relayed through the steam servers, clients and the server will NOT communicate directly." << Con::endl;
		}
	}

	InitializeResourceManager();
	m_resourceWatcher->MountDirectory("");
	auto &addons = AddonSystem::GetMountedAddons();
	for(auto &info : addons)
		m_resourceWatcher->MountDirectory(info.GetAbsolutePath() + "/", true);
}

bool NetworkState::IsClient() const { return false; }
bool NetworkState::IsServer() const { return false; }

void NetworkState::InitializeResourceManager() { m_resourceWatcher = std::make_unique<ResourceWatcherManager>(this); }

ConVar *NetworkState::SetConVar(std::string scmd, std::string value, bool bApplyIfEqual)
{
	auto *cv = GetConVar(scmd);
	if(cv == nullptr)
		return nullptr;
	if(cv->GetType() != ConType::Var)
		return nullptr;
	auto *cvar = static_cast<ConVar *>(cv);
	auto prev = cvar->GetString();
	if(bApplyIfEqual == false && prev == value)
		return nullptr;
	std::array<const std::unordered_map<std::string, std::vector<CvarCallback>> *, 2> cvarCallbackList = {&m_cvarCallbacks, nullptr};
	auto *game = GetGameState();
	if(game != nullptr)
		cvarCallbackList.at(1) = &game->GetConVarCallbacks();
	udm::visit(cvar->GetVarType(), [this, &cvarCallbackList, &scmd, cvar, &value](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(console::is_valid_convar_type_v<T>) {
			auto &rawValPrev = cvar->GetRawValue();
			auto prevVal = rawValPrev ? *static_cast<T *>(rawValPrev.get()) : T {};
			cvar->SetValue(value);
			auto &rawValNew = cvar->GetRawValue();
			if(!rawValNew)
				return;
			auto &newVal = *static_cast<T *>(rawValNew.get());
			for(auto &cvList : cvarCallbackList) {
				if(!cvList)
					continue;
				auto it = cvList->find(scmd);
				if(it != cvList->end()) {
					auto &cvarCallbacks = const_cast<std::vector<CvarCallback> &>(it->second);
					for(auto itCb = cvarCallbacks.begin(); itCb != cvarCallbacks.end();) {
						auto &ptrCb = *itCb;
						auto &fc = const_cast<CvarCallback &>(ptrCb).GetFunction();
						if(!fc.IsValid())
							itCb = cvarCallbacks.erase(itCb);
						else {
							fc.Call<void, NetworkState *, const ConVar &, const void *, const void *>(this, *cvar, &prevVal, &newVal);
							++itCb;
						}
					}
				}
			}
		}
	});
	return cvar;
}

void NetworkState::implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const
{
	engine->implFindSimilarConVars(input, similarCmds);
	CVarHandler::implFindSimilarConVars(input, similarCmds);
}

void NetworkState::TranslateConsoleCommand(std::string &cmd)
{
	auto it = m_conOverrides.find(cmd);
	if(it == m_conOverrides.end())
		return;
	cmd = it->second;
}
void NetworkState::SetConsoleCommandOverride(const std::string &src, const std::string &dst) { m_conOverrides[src] = dst; }
void NetworkState::ClearConsoleCommandOverride(const std::string &src)
{
	auto it = m_conOverrides.find(src);
	if(it == m_conOverrides.end())
		return;
	m_conOverrides.erase(it);
}
void NetworkState::ClearConsoleCommandOverrides() { m_conOverrides.clear(); }

bool NetworkState::RunConsoleCommand(std::string scmd, std::vector<std::string> &argv, pragma::BasePlayerComponent *pl, KeyState pressState, float magnitude, const std::function<bool(ConConf *, float &)> &callback)
{
	TranslateConsoleCommand(scmd);
	auto *cv = GetConVar(scmd);
	auto bEngine = ((cv == nullptr) ? true : false);
	if(bEngine == true)
		cv = engine->CVarHandler::GetConVar(scmd);
	if(cv == nullptr)
		return false;
	if(callback != nullptr && callback(cv, magnitude) == false)
		return true;

	auto type = cv->GetType();
	if(type == ConType::Var) {
		auto *cvar = static_cast<ConVar *>(cv);
		if(argv.empty()) {
			cvar->Print(scmd);
			return true;
		}
		auto flags = cvar->GetFlags();
		auto bReplicated = ((flags & ConVarFlags::Replicated) == ConVarFlags::Replicated) ? true : false;
		if(IsClient()) {
			if(bReplicated) {
				Con::cout << "Can't change replicated ConVar " << scmd << " from console of client, only server operator can change its value" << Con::endl;
				return true;
			}
		}
		if((flags & ConVarFlags::Cheat) == ConVarFlags::Cheat)
			CHECK_CHEATS(scmd, this, true);
		if(bEngine)
			engine->CVarHandler::SetConVar(scmd, argv[0]);
		else
			SetConVar(scmd, argv[0]);
		return true;
	}
	auto *cmd = static_cast<ConCommand *>(cv);
	if(type == ConType::Cmd) {
		std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> func = nullptr;
		cmd->GetFunction(func);
		if(scmd.empty() == false && scmd.front() == '-')
			magnitude = 0.f;
		func(this, pl, argv, magnitude);
		return true;
	}
	auto *game = GetGameState();
	if(game == nullptr)
		return false;
	//auto *l = game->GetLuaState();
	auto bJoystick = (cmd->GetFlags() & (ConVarFlags::JoystickAxisSingle | ConVarFlags::JoystickAxisContinuous)) != ConVarFlags::None;
	LuaFunction func = nullptr;
	cmd->GetFunction(func);
	game->ProtectedLuaCall(
	  [&func, &pl, &argv, magnitude, bJoystick](lua_State *l) {
		  func.GetLuaObject().push(l);
		  if(pl == nullptr)
			  Lua::PushNil(l);
		  else
			  pl->PushLuaObject(l);

		  if(bJoystick == true)
			  Lua::PushNumber(l, magnitude);

		  auto argc = argv.size();
		  for(auto i = decltype(argc) {0}; i < argc; ++i)
			  Lua::PushString(l, argv[i]);
		  return Lua::StatusCode::Ok;
	  },
	  0);
	return true;
}

std::shared_ptr<util::Library> NetworkState::LoadLibraryModule(const std::string &lib, const std::vector<std::string> &additionalSearchDirectories, std::string *err)
{
	auto pathLib = util::Path::CreateFile(lib);
	static std::unordered_set<std::string> cache;
	auto it = cache.find(pathLib.GetString());
	if(it != cache.end())
		return nullptr;

	auto libModule = util::load_library_module(lib, additionalSearchDirectories, {}, err);
	if(!libModule)
		cache.insert(pathLib.GetString()); // Cache libraries we weren't able to load to avoid continuous error messages
	return libModule;
}

std::shared_ptr<util::Library> NetworkState::GetLibraryModule(const std::string &library) const
{
	auto lib = util::get_normalized_module_path(library, IsClient());
	auto it = s_loadedLibraries.find(lib);
	if(it == s_loadedLibraries.end())
		return nullptr;
	if(!it->second.WasLoadedInState(*this))
		return nullptr;
	return *it->second.library;
}

void NetworkState::InitializeLuaModules(lua_State *l)
{
	for(auto &pair : s_loadedLibraries) {
		if(!pair.second.WasLoadedInState(*this))
			continue;
		InitializeDLLModule(l, *pair.second.library);
	}
}

void NetworkState::InitializeDLLModule(lua_State *l, std::shared_ptr<util::Library> module)
{
	auto it = m_initializedLibraries.find(l);
	if(it != m_initializedLibraries.end()) {
		auto itMod = std::find(it->second.begin(), it->second.end(), module);
		if(itMod != it->second.end())
			return; // Module was already initialized for this lua state
	}
	else
		it = m_initializedLibraries.insert(std::make_pair(l, std::vector<std::shared_ptr<util::Library>> {})).first;
	it->second.push_back(module);

	auto *luaInterface = engine->GetLuaInterface(l);
	if(luaInterface != nullptr) {
		auto *ptrInitLua = module->FindSymbolAddress<void (*)(Lua::Interface &)>("pragma_initialize_lua");
		if(ptrInitLua != nullptr)
			ptrInitLua(*luaInterface);
	}
}

bool NetworkState::UnloadLibrary(const std::string &library)
{
	auto libAbs = util::get_normalized_module_path(library, IsClient());
	auto it = s_loadedLibraries.find(libAbs);
	if(it == s_loadedLibraries.end())
		return true;
	auto lib = it->second.library;
	auto it2 = std::find_if(m_libHandles.begin(), m_libHandles.end(), [&lib](const std::shared_ptr<std::shared_ptr<util::Library>> &ptr) { return ptr->get() == lib->get(); });
	it = s_loadedLibraries.erase(it);

	auto *ptrTerminateLua = (*lib)->FindSymbolAddress<void (*)(Lua::Interface &)>("pragma_terminate_lua");
	for(auto &pair : m_initializedLibraries) {
		auto it = std::find_if(pair.second.begin(), pair.second.end(), [&lib](const std::shared_ptr<util::Library> &ptr) { return ptr.get() == lib->get(); });
		if(it != pair.second.end()) {
			if(ptrTerminateLua != nullptr)
				ptrTerminateLua(*engine->GetLuaInterface(pair.first));
			pair.second.erase(it);
		}
	}

	if(lib->get() == m_lastModuleHandle.get())
		m_lastModuleHandle = nullptr;

	auto *ptrDetach = (*lib)->FindSymbolAddress<void (*)()>("pragma_detach");
	if(ptrDetach != nullptr)
		ptrDetach();
	lib = nullptr;
	if(it2 != m_libHandles.end())
		m_libHandles.erase(it2);
	s_loadedLibraries.erase(it);
	return true;
}

std::shared_ptr<util::Library> NetworkState::InitializeLibrary(std::string library, std::string *err, lua_State *l)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("load_library_module_" + library);
	util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	if(l == nullptr)
		l = GetLuaState();
	auto libAbs = util::get_normalized_module_path(library, IsClient());

	std::shared_ptr<util::Library> dllHandle = nullptr;
	auto it = s_loadedLibraries.find(libAbs);
	if(it == s_loadedLibraries.end()) {
		auto additionalSearchDirectories = util::get_default_additional_library_search_directories(libAbs);
		dllHandle = LoadLibraryModule(libAbs.substr(8), additionalSearchDirectories, err);
		m_lastModuleHandle = dllHandle;
		if(dllHandle != NULL) {
			std::string errMsg;
			std::function<bool(std::string &)> attach = nullptr;
			auto *ptrAttachPragma = dllHandle->FindSymbolAddress<bool (*)(std::string &)>("pragma_attach");
			if(ptrAttachPragma != nullptr)
				attach = ptrAttachPragma;
			if(attach != nullptr) {
				/*bool *dllServer = (bool*)GetProcAddress(dllHandle,"SERVER");
				if(dllServer != NULL)
					*dllServer = IsServer();
				bool *dllClient = (bool*)GetProcAddress(dllHandle,"CLIENT");
				if(dllClient != NULL)
					*dllClient = IsClient();
				char *dllState = (char*)GetProcAddress(dllHandle,"WEAVE_MODULE_STATE");
				if(dllState != NULL)
					*dllState = IsServer() ? 2 : 1;*/

				auto bSuccess = attach(errMsg);
				if(bSuccess == false) {
					if(err != nullptr)
						*err = errMsg;
					return nullptr;
				}
			}
			auto *ptrKeepAlive = dllHandle->FindSymbolAddress<bool (*)()>("pragma_should_keep_alive_until_program_shutdown");
			if(ptrKeepAlive != nullptr && ptrKeepAlive())
				dllHandle->SetDontFreeLibraryOnDestruct();

			auto ptrDllHandle = std::make_shared<std::shared_ptr<util::Library>>(dllHandle);
			m_libHandles.push_back(ptrDllHandle);
			s_loadedLibraries.insert(decltype(s_loadedLibraries)::value_type(libAbs, {ptrDllHandle, IsServer(), IsClient()}));
		}
	}
	else {
		if(err != nullptr)
			*err = "";
		auto ptrDllHandle = it->second.library;
		dllHandle = *ptrDllHandle;
		auto itHandle = std::find(m_libHandles.begin(), m_libHandles.end(), ptrDllHandle);
		if(itHandle != m_libHandles.end())
			return dllHandle; // DLL has already been initialized for this network state
		if(IsServer())
			it->second.loadedServerside = true;
		else
			it->second.loadedClientside = true;
		m_libHandles.push_back(ptrDllHandle);
	}
	if(dllHandle != nullptr) {
		if(l != nullptr)
			InitializeDLLModule(l, dllHandle);
		return dllHandle;
	}
	return nullptr;
}

void NetworkState::TerminateLuaModules(lua_State *l)
{
	auto it = m_initializedLibraries.find(l);
	if(it == m_initializedLibraries.end())
		return;
	auto *luaInterface = engine->GetLuaInterface(l);
	if(luaInterface != nullptr) {
		for(auto &dllHandle : it->second) {
			auto *ptrInitLua = dllHandle->FindSymbolAddress<void (*)(Lua::Interface &)>("pragma_terminate_lua");
			if(ptrInitLua != nullptr)
				ptrInitLua(*luaInterface);
		}
	}
}
void NetworkState::DeregisterLuaModules(void *l, const std::string &identifier)
{
	auto it = m_initializedLibraries.find(reinterpret_cast<lua_State *>(l));
	if(it == m_initializedLibraries.end())
		return;
	for(auto &dllHandle : it->second) {
		auto *ptrInitLua = dllHandle->FindSymbolAddress<void (*)(const std::string &, const void *)>("pragma_post_terminate_lua");
		if(ptrInitLua != nullptr)
			ptrInitLua(identifier, l);
	}
	m_initializedLibraries.erase(it);
}

ConVarMap *NetworkState::GetConVarMap() { return nullptr; }

void NetworkState::UnregisterConVar(const std::string &scmd)
{
	auto lcmd = scmd;
	ustring::to_lower(lcmd);
	auto it = m_conVars.find(scmd);
	if(it != m_conVars.end())
		m_conVars.erase(it);
}

ConVar *NetworkState::RegisterConVar(const std::string &scmd, const std::shared_ptr<ConVar> &cvar)
{
	auto lcmd = scmd;
	ustring::to_lower(lcmd);
	auto it = m_conVars.find(scmd);
	if(it != m_conVars.end()) {
		auto &cf = it->second;
		if(cf->GetType() != ConType::Var)
			return nullptr;
		return static_cast<ConVar *>(cf.get());
	}
	auto itNew = m_conVars.insert(decltype(m_conVars)::value_type(scmd, cvar));
	auto *cv = static_cast<ConVar *>(itNew.first->second.get());
	auto &cfg = engine->GetConVarConfig(GetType());
	if(cfg) {
		// Use value from loaded config
		auto *args = cfg->Find(scmd);
		if(args && !args->empty())
			cv->SetValue((*args)[0]);
	}
	return cv;
}
ConVar *NetworkState::CreateConVar(const std::string &scmd, udm::Type type, const std::string &value, ConVarFlags flags, const std::string &help)
{
	return udm::visit(type, [this, &scmd, &value, flags, &help](auto tag) -> ConVar * {
		using T = typename decltype(tag)::type;
		if constexpr(console::is_valid_convar_type_v<T> && udm::is_convertible<std::string, T>())
			return RegisterConVar(scmd, ConVar::Create<T>(udm::convert<std::string, T>(value), flags, help));
		return nullptr;
	});
}

std::unordered_map<std::string, unsigned int> &NetworkState::GetConCommandIDs() { return m_conCommandIDs; }

ConCommand *NetworkState::CreateConCommand(const std::string &scmd, LuaFunction fc, ConVarFlags flags, const std::string &help)
{
	auto lcmd = scmd;
	ustring::to_lower(lcmd);
	if(m_conVars.find(lcmd) != m_conVars.end())
		return nullptr;
	auto *cmd = new ConCommand(fc, flags, help);
	m_conVars.insert(std::unordered_map<std::string, ConConf *>::value_type(scmd, static_cast<ConConf *>(cmd)));
	return cmd;
}

pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *NetworkState::GetProfilingStageManager() { return m_profilingStageManager.get(); }
bool NetworkState::StartProfilingStage(const char *stage) { return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage); }
bool NetworkState::StopProfilingStage() { return m_profilingStageManager && m_profilingStageManager->StopProfilerStage(); }

const double MS_THOUSAND = 1000;
void NetworkState::Think()
{
	m_ctReal.Update();
	m_tReal = CDouble(m_ctReal());
	m_tDelta = m_tReal - m_tLast;
	if(m_tDelta > 1.0f) {
		Con::cwar << "Delta time surpassed 0.5 seconds. Clamping..." << Con::endl;
		m_tDelta = 0.5f;
	}
	StartProfilingStage("UpdateSounds");
	UpdateSounds();
	StopProfilingStage(); // UpdateSounds
	CallCallbacks<void>("Think");
	Game *game = GetGameState();
	if(game != NULL)
		game->Think();
	m_resourceWatcher->Poll();
	m_tLast = m_tReal;
	for(unsigned int i = 0; i < m_thinkCallbacks.size(); i++)
		m_thinkCallbacks[i]();
}

const pragma::asset::ModelManager &NetworkState::GetModelManager() const { return const_cast<NetworkState *>(this)->GetModelManager(); }
pragma::asset::ModelManager &NetworkState::GetModelManager() { return *m_modelManager; }

void NetworkState::CallOnNextTick(const std::function<void()> &f) { m_tickCallQueue.push(f); }

void NetworkState::Tick()
{
	while(!m_tickCallQueue.empty()) {
		m_tickCallQueue.front()();
		m_tickCallQueue.pop();
	}

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("poll_model_manager");
#endif
	m_modelManager->Poll();
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif

	CallCallbacks<void>("Tick");
	Game *game = GetGameState();
	if(game != NULL)
		game->Tick();
	for(unsigned int i = 0; i < m_tickCallbacks.size(); i++)
		m_tickCallbacks[i]();
}

void NetworkState::AddThinkCallback(CallbackHandle callback) { m_thinkCallbacks.push_back(callback); }

void NetworkState::AddTickCallback(CallbackHandle callback) { m_tickCallbacks.push_back(callback); }
