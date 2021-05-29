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
#include <pragma/console/s_cvar_global_functions.h>
#include <pragma/lua/luaapi.h>
#include <luainterface.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>
#include <pragma/util/util_module.hpp>
#include "pragma/asset/util_asset.hpp"

#define DLLSPEC_ISTEAMWORKS DLLNETWORK
#include "pragma/game/isteamworks.hpp"

ConVarHandle NetworkState::GetConVarHandle(std::unordered_map<std::string,std::shared_ptr<PtrConVar>> &ptrs,std::string scvar) {return CVarHandler::GetConVarHandle(ptrs,scvar);}

UInt8 NetworkState::STATE_COUNT = 0;

decltype(NetworkState::s_loadedLibraries) NetworkState::s_loadedLibraries = {};
extern DLLNETWORK Engine *engine;

NetworkState::NetworkState()
	: CallbackHandler(),CVarHandler()
{
	m_ctReal.Reset(static_cast<int64_t>(engine->GetTickCount()));
	m_tReal = CDouble(m_ctReal());
	m_tLast = m_tReal;
	m_tDelta = 0;
	if(STATE_COUNT == 0)
	{
		AddonSystem::MountAddons();
		GameModeManager::Initialize();
	}
	STATE_COUNT++;

	RegisterCallback<void>("Think");
	RegisterCallback<void>("Tick");
	RegisterCallback<void>("OnClose");

	RegisterCallback<void,std::reference_wrapper<struct ISteamworks>>("OnSteamworksInitialized");
	RegisterCallback<void>("OnSteamworksShutdown");

	m_cbProfilingHandle = engine->AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false)
		{
			m_profilingStageManager = nullptr;
			return;
		}
		std::string postFix = IsClient() ? " (CL)" : " (SV)";
		auto &cpuProfiler = engine->GetProfiler();
		m_profilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage,CPUProfilingPhase>>();
		m_profilingStageManager->InitializeProfilingStageManager(cpuProfiler,{
			pragma::debug::ProfilingStage::Create(cpuProfiler,"UpdateSounds" +postFix,&engine->GetProfilingStageManager()->GetProfilerStage(Engine::CPUProfilingPhase::Think))
		});
		static_assert(umath::to_integral(CPUProfilingPhase::Count) == 1u,"Added new profiling phase, but did not create associated profiling stage!");
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

	for(unsigned int i=0;i<m_thinkCallbacks.size();i++)
	{
		if(m_thinkCallbacks[i].IsValid())
			m_thinkCallbacks[i].Remove();
	}
	for(unsigned int i=0;i<m_tickCallbacks.size();i++)
	{
		if(m_tickCallbacks[i].IsValid())
			m_tickCallbacks[i].Remove();
	}
	if(m_cbProfilingHandle.IsValid())
		m_cbProfilingHandle.Remove();

	m_libHandles.clear();
	for(auto it=s_loadedLibraries.begin();it!=s_loadedLibraries.end();)
	{
		auto &hLib = it->second;
		if(hLib.use_count() > 1)
		{
			++it;
			continue; // Library is still in use by another network state
		}
		auto *ptrDetach = (*hLib)->FindSymbolAddress<void(*)()>("pragma_detach");
		if(ptrDetach != nullptr)
			ptrDetach();
		it = s_loadedLibraries.erase(it);
	}

	for(auto &hCb : m_luaEnumRegisterCallbacks)
	{
		if(hCb.IsValid() == false)
			continue;
		hCb.Remove();
	}
}

std::vector<CallbackHandle> &NetworkState::GetLuaEnumRegisterCallbacks() {return m_luaEnumRegisterCallbacks;}

ResourceWatcherManager &NetworkState::GetResourceWatcher() {return *m_resourceWatcher;}

bool NetworkState::ShouldRemoveSound(ALSound &snd) {return snd.IsPlaying() == false;}

void NetworkState::UpdateSounds(std::vector<std::shared_ptr<ALSound>> &sounds)
{
	for(auto i=static_cast<int32_t>(sounds.size()) -1;i>=0;--i)
	{
		auto psnd = sounds.at(i);
		if(psnd != nullptr)
		{
			auto &snd = *psnd;
			snd.Update();
			if(psnd.use_count() <= 2 && ShouldRemoveSound(snd))
			{
				psnd = nullptr; // Cleanup occurs in "OnDestroyed" callback
				continue;
			}
		}
	}
	for(auto &snd : sounds)
	{
		if(snd == nullptr)
			continue;
		snd->PostUpdate();
	}
}

bool NetworkState::PortMaterial(const std::string &path,const std::function<Material*(const std::string&,bool)> &fLoadMaterial)
{
	auto pathWithoutExt = path;
	auto extensions = pragma::asset::get_supported_extensions(pragma::asset::Type::Material);
	extensions.push_back("vmt");
	extensions.push_back("vmat_c");
	ufile::remove_extension_from_filename(pathWithoutExt,extensions);

	// TODO: This doesn't belong here! Move it into the source module
	auto matPath = pathWithoutExt +".vmat_c";
	if(util::port_file(this,"materials\\" +matPath) == false)
	{
		matPath = pathWithoutExt +".vmt";
		if(util::port_file(this,"materials\\" +matPath) == false)
			return false;
	}
	if(fLoadMaterial == nullptr)
		return true;
	auto *mat = fLoadMaterial(matPath,true);
	if(mat && mat->GetDataBlock())
	{
		// Port textures as well
		std::function<void(const std::shared_ptr<ds::Block>&)> fPortTextures = nullptr;
		fPortTextures = [this,&fPortTextures](const std::shared_ptr<ds::Block> &block) {
			auto *data = block->GetData();
			if(data == nullptr)
				return;
			for(auto &pair : *data)
			{
				auto v = pair.second;
				if(v->IsBlock() == true)
					fPortTextures(std::static_pointer_cast<ds::Block>(v));
				else
				{
					auto dataTex = std::dynamic_pointer_cast<ds::Texture>(v);
					if(dataTex)
					{
						auto path = "materials\\" +dataTex->GetValue().name;
						if(FileManager::Exists(path) == false && util::port_file(this,path +".vtf") == false && util::port_file(this,path +".vtex_c") == false)
							Con::cwar<<"WARNING: Unable to port texture '"<<dataTex->GetValue().name<<"'!"<<Con::endl;
					}
				}
			}
		};
		fPortTextures(mat->GetDataBlock());
	}
	return (mat != nullptr) ? true : false;
}

void NetworkState::ClearGameConVars()
{
#pragma message ("TODO: What about ConVars that have been created ingame?")
	for(auto it=m_conVars.begin();it!=m_conVars.end();)
	{
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

Material *NetworkState::LoadMaterial(const std::string &path,bool bReload)
{
	static auto bSkipPort = false;
	bool bFirstTimeError;
	auto *mat = GetMaterialManager().Load(path,bReload,&bFirstTimeError);
	if(bFirstTimeError == true)
	{
		if(bSkipPort == false)
		{
			auto vmtPath = path;
			ufile::remove_extension_from_filename(vmtPath);
			vmtPath += ".vmt";
			if(util::port_file(this,"materials\\" +vmtPath) == true)
			{
				bSkipPort = true;
				mat = LoadMaterial(path,bReload);
				bSkipPort = false;
				if(mat != nullptr)
				{

					// TODO: Load textures
				}
				return mat;
			}
		}
		Con::cwar<<"WARNING: Unable to load material '"<<path<<"': File not found!"<<Con::endl;
	}
	return mat;
}

MapInfo *NetworkState::GetMapInfo() {return m_mapInfo.get();}
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

void NetworkState::StartNewGame(const std::string &map,bool singlePlayer)
{
	if(!IsGameActive())
		StartGame(singlePlayer);
	ChangeLevel(map);
}

void NetworkState::ChangeLevel(const std::string &map)
{
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

void NetworkState::EndGame()
{
	ClearGameConVars();
}

bool NetworkState::IsGameActive() {return false;}

Game *NetworkState::GetGameState() {return m_game.get();}

void NetworkState::Initialize()
{
	CVarHandler::Initialize();
	if(IsClient())
		Con::cout<<"Initializing client state..."<<Con::endl;
	else
	{
		Con::cout<<"Initializing server state..."<<Con::endl;
		if(engine->IsServerOnly())
		{
			Con::cout<<"If you encounter problems, such as the server not showing up in the server browser, or clients not being able to connect to it, please make sure the following ports are forwarded:"<<Con::endl;
			Con::cout<<engine_info::DEFAULT_SERVER_PORT<<" (TCP): Required if the boost asio networking layer is used"<<Con::endl;
			Con::cout<<engine_info::DEFAULT_SERVER_PORT<<" (UDP): Required for clients to be able to connect to the server"<<Con::endl;
			Con::cout<<engine_info::DEFAULT_QUERY_PORT<<" (UDP): Required for the server to be registered with the master server and show up in the server browser"<<Con::endl;
			Con::cout<<Con::endl<<"Here's a list of useful console commands:"<<Con::endl;
			Con::cout<<"map <mapName>: Loads the specified map and starts the server."<<Con::endl;
			Con::cout<<"sv_maxplayers <maxPlayers>: Specifies the maximum number of players that can play on the server at a time."<<Con::endl;
			Con::cout<<"sv_gamemode <gameMode>: The gamemode to use. Has to be specified before a map is loaded."<<Con::endl;
			Con::cout<<"net_library <netLib>: The networking library module to use. The name has to match one of the modules located in the 'pragma/modules/networking' directory."<<Con::endl;
			Con::cout<<"sv_use_p2p_if_available <1/0>: Only has an effect if the steam networking module is used. Network communication will be relayed through the steam servers, clients and the server will NOT communicate directly."<<Con::endl;
		}
	}

	InitializeResourceManager();
	m_resourceWatcher->MountDirectory("");
	auto &addons = AddonSystem::GetMountedAddons();
	for(auto &info : addons)
		m_resourceWatcher->MountDirectory(info.GetAbsolutePath() +"/",true);
}

bool NetworkState::IsClient() const {return false;}
bool NetworkState::IsServer() const {return false;}

void NetworkState::InitializeResourceManager() {m_resourceWatcher = std::make_unique<ResourceWatcherManager>(this);}

ConVar *NetworkState::SetConVar(std::string scmd,std::string value,bool bApplyIfEqual)
{
	auto *cv = GetConVar(scmd);
	if(cv == nullptr)
		return nullptr;
	if(cv->GetType() != ConType::Var)
		return nullptr;
	auto *cvar = static_cast<ConVar*>(cv);
	auto prev = cvar->GetString();
	if(bApplyIfEqual == false && prev == value)
		return nullptr;
	cvar->SetValue(value);
	auto *game = GetGameState();
	std::array<const std::unordered_map<std::string,std::vector<std::shared_ptr<CvarCallback>>>*,2> cvarCallbackList = {&m_cvarCallbacks,nullptr};
	if(game != nullptr)
		cvarCallbackList.at(1) = &game->GetConVarCallbacks();
	for(auto *cvarCallbacks : cvarCallbackList)
	{
		if(cvarCallbacks == nullptr)
			continue;
		auto it = cvarCallbacks->find(scmd);
		if(it != cvarCallbacks->end())
		{
			auto *game = GetGameState();
			for(auto &cb : it->second)
			{
				if(cb->IsLuaFunction())
				{
					if(game != nullptr)
					{
						auto *luaFunc = cb->GetLuaFunction();
						if(luaFunc != nullptr)
						{
							game->ProtectedLuaCall([&prev,&value,luaFunc](lua_State *l) {
								luaFunc->GetLuaObject().push(l);
								Lua::PushString(l,prev);
								Lua::PushString(l,value);
								return Lua::StatusCode::Ok;
							},0);
						}
					}
				}
				else
				{ // WEAVETODO: Move this into the game-class
					auto *fc = cb->GetFunction();
					fc->Call(this,cvar,prev);
				}
			}
		}
	}
	return cvar;
}

void NetworkState::implFindSimilarConVars(const std::string &input,std::vector<SimilarCmdInfo> &similarCmds) const
{
	engine->implFindSimilarConVars(input,similarCmds);
	CVarHandler::implFindSimilarConVars(input,similarCmds);
}

void NetworkState::TranslateConsoleCommand(std::string &cmd)
{
	auto it = m_conOverrides.find(cmd);
	if(it == m_conOverrides.end())
		return;
	cmd = it->second;
}
void NetworkState::SetConsoleCommandOverride(const std::string &src,const std::string &dst) {m_conOverrides[src] = dst;}
void NetworkState::ClearConsoleCommandOverride(const std::string &src)
{
	auto it = m_conOverrides.find(src);
	if(it == m_conOverrides.end())
		return;
	m_conOverrides.erase(it);
}
void NetworkState::ClearConsoleCommandOverrides() {m_conOverrides.clear();}

bool NetworkState::RunConsoleCommand(std::string scmd,std::vector<std::string> &argv,pragma::BasePlayerComponent *pl,KeyState pressState,float magnitude,const std::function<bool(ConConf*,float&)> &callback)
{
	TranslateConsoleCommand(scmd);
	auto *cv = GetConVar(scmd);
	auto bEngine = ((cv == nullptr) ? true : false);
	if(bEngine == true)
		cv = engine->CVarHandler::GetConVar(scmd);
	if(cv == nullptr)
		return false;
	if(callback != nullptr && callback(cv,magnitude) == false)
		return true;

	auto type = cv->GetType();
	if(type == ConType::Var)
	{
		auto *cvar = static_cast<ConVar*>(cv);
		if(argv.empty())
		{
			cvar->Print(scmd);
			return true;
		}
		auto flags = cvar->GetFlags();
		auto bReplicated = ((flags &ConVarFlags::Replicated) == ConVarFlags::Replicated) ? true : false;
		if(IsClient())
		{
			if(bReplicated)
			{
				Con::cout<<"Can't change replicated ConVar "<<scmd<<" from console of client, only server operator can change its value"<<Con::endl;
				return true;
			}
		}
		if((flags &ConVarFlags::Cheat) == ConVarFlags::Cheat)
			CHECK_CHEATS(scmd,this,true);
		if(bEngine)
			engine->CVarHandler::SetConVar(scmd,argv[0]);
		else
			SetConVar(scmd,argv[0]);
		return true;
	}
	auto *cmd = static_cast<ConCommand*>(cv);
	if(type == ConType::Cmd)
	{
		std::function<void(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)> func = nullptr;
		cmd->GetFunction(func);
		if(scmd.empty() == false && scmd.front() == '-')
			magnitude = 0.f;
		func(this,pl,argv,magnitude);
		return true;
	}
	auto *game = GetGameState();
	if(game == nullptr)
		return false;
	//auto *l = game->GetLuaState();
	auto bJoystick = (cmd->GetFlags() &(ConVarFlags::JoystickAxisSingle | ConVarFlags::JoystickAxisContinuous)) != ConVarFlags::None;
	LuaFunction func = nullptr;
	cmd->GetFunction(func);
	game->ProtectedLuaCall([&func,&pl,&argv,magnitude,bJoystick](lua_State *l) {
		func.GetLuaObject().push(l);
		if(pl == nullptr)
			Lua::PushNil(l);
		else
			pl->PushLuaObject(l);

		if(bJoystick == true)
			Lua::PushNumber(l,magnitude);

		auto argc = argv.size();
		for(auto i=decltype(argc){0};i<argc;++i)
			Lua::PushString(l,argv[i]);
		return Lua::StatusCode::Ok;
	},0);
	return true;
}

std::shared_ptr<util::Library> NetworkState::LoadLibraryModule(const std::string &lib,const std::vector<std::string> &additionalSearchDirectories,std::string *err)
{
	return util::load_library_module(lib,additionalSearchDirectories,{},err);
}

std::shared_ptr<util::Library> NetworkState::GetLibraryModule(const std::string &library) const
{
	auto lib = util::get_normalized_module_path(library,IsClient());
	auto it = s_loadedLibraries.find(lib);
	if(it == s_loadedLibraries.end())
		return nullptr;
	return *it->second;
}

void NetworkState::InitializeLuaModules(lua_State *l)
{
	for(auto &pair : s_loadedLibraries)
		InitializeDLLModule(l,*pair.second);
}

void NetworkState::InitializeDLLModule(lua_State *l,std::shared_ptr<util::Library> module)
{
	auto it = m_initializedLibraries.find(l);
	if(it != m_initializedLibraries.end())
	{
		auto itMod = std::find(it->second.begin(),it->second.end(),module);
		if(itMod != it->second.end())
			return; // Module was already initialized for this lua state
	}
	else
		it = m_initializedLibraries.insert(std::make_pair(l,std::vector<std::shared_ptr<util::Library>>{})).first;
	it->second.push_back(module);

	auto *luaInterface = engine->GetLuaInterface(l);
	if(luaInterface != nullptr)
	{
		auto *ptrInitLua = module->FindSymbolAddress<void(*)(Lua::Interface&)>("pragma_initialize_lua");
		if(ptrInitLua != nullptr)
			ptrInitLua(*luaInterface);
	}
}

std::shared_ptr<util::Library> NetworkState::InitializeLibrary(std::string library,std::string *err,lua_State *l)
{
	if(l == nullptr)
		l = GetLuaState();
	auto libAbs = util::get_normalized_module_path(library,IsClient());
	auto brLast = libAbs.find_last_of('\\');

	std::shared_ptr<util::Library> dllHandle = nullptr;
	auto it = s_loadedLibraries.find(libAbs);
	if(it == s_loadedLibraries.end())
	{
		auto additionalSearchDirectories = util::get_default_additional_library_search_directories(libAbs);
		dllHandle = LoadLibraryModule(libAbs.substr(8),additionalSearchDirectories,err);
		m_lastModuleHandle = dllHandle;
		if(dllHandle != NULL)
		{
			std::string errMsg;
			std::function<bool(std::string&)> attach = nullptr;
			auto *ptrAttachPragma = dllHandle->FindSymbolAddress<bool(*)(std::string&)>("pragma_attach");
			if(ptrAttachPragma != nullptr)
				attach = ptrAttachPragma;
			if(attach != nullptr)
			{
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
				if(bSuccess == false)
				{
					if(err != nullptr)
						*err = errMsg;
					return nullptr;
				}
			}
			auto ptrDllHandle = std::make_shared<std::shared_ptr<util::Library>>(dllHandle);
			m_libHandles.push_back(ptrDllHandle);
			s_loadedLibraries.insert(decltype(s_loadedLibraries)::value_type(libAbs,ptrDllHandle));
		}
	}
	else
	{
		if(err != nullptr)
			*err = "";
		auto ptrDllHandle = it->second;
		dllHandle = *ptrDllHandle;
		auto it = std::find(m_libHandles.begin(),m_libHandles.end(),ptrDllHandle);
		if(it != m_libHandles.end())
			return dllHandle; // DLL has already been initialized for this network state
		m_libHandles.push_back(ptrDllHandle);
	}
	if(dllHandle != nullptr)
	{
		if(l != nullptr)
			InitializeDLLModule(l,dllHandle);
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
	if(luaInterface != nullptr)
	{
		for(auto &dllHandle : it->second)
		{
			auto *ptrInitLua = dllHandle->FindSymbolAddress<void(*)(Lua::Interface&)>("pragma_terminate_lua");
			if(ptrInitLua != nullptr)
				ptrInitLua(*luaInterface);
		}
	}
}
void NetworkState::DeregisterLuaModules(void *l,const std::string &identifier)
{
	auto it = m_initializedLibraries.find(reinterpret_cast<lua_State*>(l));
	if(it == m_initializedLibraries.end())
		return;
	for(auto &dllHandle : it->second)
	{
		auto *ptrInitLua = dllHandle->FindSymbolAddress<void(*)(const std::string&,const void*)>("pragma_post_terminate_lua");
		if(ptrInitLua != nullptr)
			ptrInitLua(identifier,l);
	}
	m_initializedLibraries.erase(it);
}

ConVarMap *NetworkState::GetConVarMap() {return nullptr;}

ConVar *NetworkState::CreateConVar(const std::string &scmd,const std::string &value,ConVarFlags flags,const std::string &help)
{
	auto lcmd = scmd;
	ustring::to_lower(lcmd);
	auto it = m_conVars.find(scmd);
	if(it != m_conVars.end())
	{
		auto &cf = it->second;
		if(cf->GetType() != ConType::Var)
			return nullptr;
		return static_cast<ConVar*>(cf.get());
	}
	auto itNew = m_conVars.insert(decltype(m_conVars)::value_type(scmd,std::make_shared<ConVar>(value,flags,help)));
	return static_cast<ConVar*>(itNew.first->second.get());
}

std::unordered_map<std::string,unsigned int> &NetworkState::GetConCommandIDs() {return m_conCommandIDs;}

ConCommand *NetworkState::CreateConCommand(const std::string &scmd,LuaFunction fc,ConVarFlags flags,const std::string &help)
{
	auto lcmd = scmd;
	ustring::to_lower(lcmd);
	if(m_conVars.find(lcmd) != m_conVars.end())
		return nullptr;
	auto *cmd = new ConCommand(fc,flags,help);
	m_conVars.insert(std::unordered_map<std::string,ConConf*>::value_type(scmd,static_cast<ConConf*>(cmd)));
	return cmd;
}

pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage,NetworkState::CPUProfilingPhase> *NetworkState::GetProfilingStageManager() {return m_profilingStageManager.get();}
bool NetworkState::StartProfilingStage(CPUProfilingPhase stage)
{
	return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage);
}
bool NetworkState::StopProfilingStage(CPUProfilingPhase stage)
{
	return m_profilingStageManager && m_profilingStageManager->StopProfilerStage(stage);
}

const double MS_THOUSAND = 1000;
void NetworkState::Think()
{
	m_ctReal.Update();
	m_tReal = CDouble(m_ctReal());
	m_tDelta = m_tReal -m_tLast;
	if(m_tDelta > 1.0f)
	{
		Con::cwar<<"WARNING: Delta time surpassed 0.5 seconds. Clamping..."<<Con::endl;
		m_tDelta = 0.5f;
	}
	StartProfilingStage(CPUProfilingPhase::UpdateSounds);
	UpdateSounds();
	StopProfilingStage(CPUProfilingPhase::UpdateSounds);
	CallCallbacks<void>("Think");
	Game *game = GetGameState();
	if(game != NULL)
		game->Think();
	m_resourceWatcher->Poll();
	m_tLast = m_tReal;
	for(unsigned int i=0;i<m_thinkCallbacks.size();i++)
		m_thinkCallbacks[i]();
}

const std::unordered_map<std::string,std::shared_ptr<Model>> &NetworkState::GetCachedModels() const {return m_modelManager->GetCache();}

const pragma::asset::ModelManager &NetworkState::GetModelManager() const {return const_cast<NetworkState*>(this)->GetModelManager();}
pragma::asset::ModelManager &NetworkState::GetModelManager() {return *m_modelManager;}

void NetworkState::Tick()
{
	CallCallbacks<void>("Tick");
	Game *game = GetGameState();
	if(game != NULL)
		game->Tick();
	for(unsigned int i=0;i<m_tickCallbacks.size();i++)
		m_tickCallbacks[i]();
}

void NetworkState::AddThinkCallback(CallbackHandle callback)
{
	m_thinkCallbacks.push_back(callback);
}

void NetworkState::AddTickCallback(CallbackHandle callback)
{
	m_tickCallbacks.push_back(callback);
}
