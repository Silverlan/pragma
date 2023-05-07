/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include <pragma/serverstate/serverstate.h>
#include "pragma/networking/netmessages.h"
#include "pragma/networking/resourcemanager.h"
#include "pragma/game/s_game.h"
#include "pragma/entities/player.h"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/standard_server.hpp"
#include "pragma/console/convarhandle.h"
#include <pragma/audio/soundscript.h>
#include "luasystem.h"
#include "pragma/networking/local_server.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <material_manager2.hpp>
#include <pragma/game/gamemode/gamemodemanager.h>
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/console/convars.h>
#include "pragma/model/s_modelmanager.h"
#include "pragma/entities/components/s_resource_watcher.hpp"
#include <pragma/asset/util_asset.hpp>
#include "pragma/networking/networking_modules.hpp"
#include "pragma/networking/master_server.hpp"
#include <pragma/networking/game_server_data.hpp>
#include <pragma/networking/error.hpp>
#include <pragma/engine_version.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>
#include <pragma/logging.hpp>

static std::unordered_map<std::string, std::shared_ptr<PtrConVar>> *conVarPtrs = NULL;
std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &ServerState::GetConVarPtrs() { return *conVarPtrs; }
ConVarHandle ServerState::GetConVarHandle(std::string scvar)
{
	if(conVarPtrs == NULL) {
		static std::unordered_map<std::string, std::shared_ptr<PtrConVar>> ptrs;
		conVarPtrs = &ptrs;
	}
	return NetworkState::GetConVarHandle(*conVarPtrs, scvar);
}

extern DLLNETWORK Engine *engine;
DLLSERVER ServerState *server = nullptr;
DLLSERVER SGame *s_game = nullptr;
ServerState::ServerState() : NetworkState(), m_server(nullptr)
{
	m_alsoundID = 1;
	server = this;
	m_soundScriptManager = std::make_unique<SoundScriptManager>();

	m_modelManager = std::make_unique<pragma::asset::SModelManager>(*this);
	engine->InitializeAssetManager(*m_modelManager);
	pragma::asset::update_extension_cache(pragma::asset::Type::Model);

	FileManager::AddCustomMountDirectory("cache", static_cast<fsys::SearchFlags>(FSYS_SEARCH_CACHE));

	RegisterCallback<void, SGame *>("EndGame");
	RegisterCallback<void, SGame *>("OnGameStart");
	RegisterCallback<void, std::reference_wrapper<NetPacket>, std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketTCP");
	RegisterCallback<void, std::reference_wrapper<NetPacket>, std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketUDP");

	ResetGameServer();
}

ServerState::~ServerState()
{
	CloseServer();
	FileManager::RemoveCustomMountDirectory("cache");
	auto &conVarPtrs = GetConVarPtrs();
	for(auto itHandles = conVarPtrs.begin(); itHandles != conVarPtrs.end(); itHandles++)
		itHandles->second->set(NULL);
	ResourceManager::ClearResources();
	m_modelManager->Clear();
	GetMaterialManager().ClearUnused();
}

void ServerState::InitializeGameServer(bool singlePlayerLocalGame)
{
	m_server = nullptr;
	m_serverReg = nullptr;

	pragma::networking::ServerEventInterface eventInterface {};
	eventInterface.onClientDropped = [this](pragma::networking::IServerClient &client, pragma::networking::DropReason reason) {
		auto *game = GetGameState();
		if(game == nullptr)
			return;
		game->OnClientDropped(client, reason);
	};
	eventInterface.onClientConnected = [](pragma::networking::IServerClient &client) {

	};
	eventInterface.handlePacket = [this](pragma::networking::IServerClient &client, NetPacket &packet) { HandlePacket(client, packet); };

	if(singlePlayerLocalGame == false) {
		auto netLibName = GetConVarString("net_library");
		auto netModPath = pragma::networking::GetNetworkingModuleLocation(netLibName, true);
		auto port = GetConVarInt("sv_port_tcp");
		auto usePeerToPeer = GetConVarBool("sv_use_p2p_if_available");
		std::string err;
		auto dllHandle = InitializeLibrary(netModPath, &err);
		if(dllHandle) {
			auto *fInitNetLib = dllHandle->FindSymbolAddress<void (*)(NetworkState &, std::unique_ptr<pragma::networking::IServer> &)>("initialize_game_server");
			if(fInitNetLib != nullptr) {
				fInitNetLib(*this, m_server);
				if(m_server) {
					m_server->SetEventInterface(eventInterface);
					pragma::networking::Error err;
					if(m_server->Start(err, port, usePeerToPeer) == false) {
						m_server = nullptr;
						m_serverReg = nullptr;
						Con::cerr << "Unable to start " << netLibName << " server: " << err.GetMessage() << Con::endl;
					}
				}
			}
			else
				Con::cerr << "Unable to initialize networking system '" << netLibName << "': Function 'initialize_game_server' not found in module!" << Con::endl;
		}
		else
			Con::cerr << "Unable to initialize networking system '" << netLibName << "': " << err << Con::endl;

		if(m_server) {
			std::string err;
			auto hLib = InitializeLibrary("steamworks/pr_steamworks", &err);
			if(hLib) {
				auto *game = static_cast<SGame *>(GetGameState());
				auto *gameMode = game ? game->GetGameMode() : nullptr;
				GameModeInfo;
				pragma::networking::GameServerInfo serverInfo {};
				serverInfo.port = port;
				serverInfo.gameName = engine_info::get_name();
				serverInfo.gameDirectory = serverInfo.gameName;
				serverInfo.gameMode = gameMode ? gameMode->name : "";

				// Note: This version has to match the version specified in steamworks
				serverInfo.version = get_engine_version();

				serverInfo.name = engine_info::get_name(); // Temporary name until actual server name has been set
				serverInfo.maxPlayers = 0;
				serverInfo.botCount = 0;
				serverInfo.mapName = game ? game->GetMapName() : "";
				serverInfo.passwordProtected = false;
				serverInfo.networkLayerIdentifier = m_server->GetNetworkLayerIdentifier();
				serverInfo.peer2peer = m_server->IsPeerToPeer();
				serverInfo.steamId = m_server->GetSteamId();
				m_serverReg = pragma::networking::MasterServerRegistration::Register(*hLib, serverInfo);
				if(m_serverReg) {
					pragma::networking::MasterServerRegistration::CallbackEvents cbEvents {};
					cbEvents.onAuthCompleted = [this](pragma::networking::MasterServerRegistration::SteamId steamId, bool authSuccess) {
						if(m_server == nullptr)
							return;
						auto &clients = m_server->GetClients();
						auto itCl = std::find_if(clients.begin(), clients.end(), [steamId](const std::shared_ptr<pragma::networking::IServerClient> &client) { return client->GetSteamId() == steamId; });
						if(itCl == clients.end())
							return;
						OnClientAuthenticated(**itCl, authSuccess);
					};
					m_serverReg->SetCallbackEvents(cbEvents);
				}
			}
			else
				Con::cerr << "Steamworks module could not be loaded! Server will not show up in steam server browser!" << Con::endl;
		}
	}
	else {
		m_server = std::make_unique<pragma::networking::LocalServer>();
		pragma::networking::Error err;
		m_server->Start(err, 0);
	}
	if(m_server == nullptr)
		ResetGameServer();
}
void ServerState::OnClientAuthenticated(pragma::networking::IServerClient &session, std::optional<bool> wasAuthenticationSuccessful)
{
	if(wasAuthenticationSuccessful.has_value() && wasAuthenticationSuccessful == false) {
		spdlog::info("Authentication for client with steam id '{}' has failed, dropping client...", session.GetSteamId());
		DropClient(session, pragma::networking::DropReason::AuthenticationFailed);
		return;
	}
	NetPacket p;
	unsigned int numResources = ResourceManager::GetResourceCount();
	p->Write<unsigned int>(numResources);
	SendPacket("start_resource_transfer", p, pragma::networking::Protocol::SlowReliable, session);
}
bool ServerState::ConnectLocalHostPlayerClient()
{
	if(m_server == nullptr)
		return false;
	m_localClient = std::make_shared<pragma::networking::LocalServerClient>();
	m_server->AddClient(m_localClient);
	return true;
}
void ServerState::ResetGameServer()
{
	m_server = std::make_unique<pragma::networking::LocalServer>();
	//if(m_localClient == nullptr)
	//	m_localClient = std::make_shared<pragma::networking::LocalServerClient>();
	//m_server->AddClient(m_localClient);
}

void ServerState::Initialize()
{
	ClearConCommands();

	/*Con::cwar<<"Server NetMessages:"<<Con::endl;
	ServerMessageMap *mapMsgs = GetNetMessageMap();
	std::unordered_map<std::string,unsigned int> *msgs;
	mapMsgs->GetNetMessages(&msgs);
	std::unordered_map<std::string,unsigned int>::iterator i;
	for(i=msgs->begin();i!=msgs->end();i++)
	{
		Con::cout<<"Name: "<<i->first<<Con::endl;
	}*/

	//m_luaNetMessageIndex.clear();
	//m_luaNetMessageIndex.push_back("invalid");
	//m_stateServer->RegisterLua(); // WEAVETODO
	NetworkState::Initialize();
}

bool ServerState::LoadSoundScripts(const char *file, bool bPrecache)
{
	auto r = NetworkState::LoadSoundScripts(file, bPrecache);
	if(s_game != nullptr && r != false)
		s_game->RegisterGameResource(SoundScriptManager::GetSoundScriptPath() + std::string(file));
	return r;
}

void ServerState::InitializeResourceManager() { m_resourceWatcher = std::make_unique<SResourceWatcherManager>(this); }

void ServerState::Close()
{
	engine->SaveEngineConfig();
	engine->SaveServerConfig();
	NetworkState::Close();
}

NwStateType ServerState::GetType() const { return NwStateType::Server; }

void ServerState::Think()
{
	NetworkState::Think();
	if(m_server) {
		if(m_server->IsRunning()) {
			pragma::networking::Error err;
			if(m_server->PollEvents(err) == false)
				Con::cwar << "Server polling failed: " << err.GetMessage() << Con::endl;
		}
		if(m_serverReg)
			m_serverReg->UpdateServerData();
	}
}

void ServerState::Tick() { NetworkState::Tick(); }

void ServerState::implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const
{
	NetworkState::implFindSimilarConVars(input, similarCmds);

	auto *svMap = console_system::server::get_convar_map();
	NetworkState::FindSimilarConVars(input, svMap->GetConVars(), similarCmds);
}

SGame *ServerState::GetGameState() { return static_cast<SGame *>(NetworkState::GetGameState()); }

bool ServerState::IsGameActive() { return m_game != nullptr; }

void ServerState::EndGame()
{
	if(!IsGameActive())
		return;
	CallCallbacks<void, SGame *>("EndGame", GetGameState());
	m_game->CallCallbacks<void>("EndGame");

	// Game::OnRemove requires that NetworkState::GetGameState returns
	// a valid game instance, but this is not the case if we destroy
	// m_game directly. Instead we move it into a temporary
	// variable and destroy that instead.
	// TODO: This is really ugly, do it another way!
	auto game = std::move(m_game);
	m_game = {game.get(), [](Game *) {}};
	game = nullptr;
	m_game = nullptr;

	s_game = nullptr;

	NetworkState::EndGame();
	ClearConCommands();
}

std::shared_ptr<ALSound> ServerState::GetSoundByIndex(unsigned int idx)
{
	auto &snds = GetSounds();
	auto it = std::find_if(snds.begin(), snds.end(), [idx](const ALSoundRef &rsound) { return (rsound.get().GetIndex() == idx) ? true : false; });
	if(it == snds.end())
		return nullptr;
	return it->get().shared_from_this();
}

void ServerState::StartGame(bool singlePlayer)
{
	NetworkState::StartGame(singlePlayer);
	StartServer(singlePlayer);
	m_game = {new SGame {this}, [](Game *game) {
		          game->OnRemove();
		          delete game;
	          }};
	s_game = static_cast<SGame *>(m_game.get());

	// Register sound-scripts as game resources
	for(auto &f : m_soundScriptManager->GetSoundScriptFiles())
		GetGameState()->RegisterGameResource(f);
	for(auto &pair : m_soundScriptManager->GetScripts()) {
		auto &script = pair.second;
		std::function<void(SoundScriptEventContainer &)> fIterateScript = nullptr;
		fIterateScript = [this, &fIterateScript](SoundScriptEventContainer &c) {
			for(auto &ev : c.GetEvents()) {
				auto *evPlaySound = dynamic_cast<SSEPlaySound *>(ev.get());
				if(evPlaySound != nullptr) {
					for(auto &src : evPlaySound->sources)
						GetGameState()->RegisterGameResource("sounds\\" + src);
				}
				fIterateScript(*ev);
			}
		};
		fIterateScript(*script);
	}

	CallCallbacks<void, SGame *>("OnGameStart", GetGameState());
	m_game->Initialize();
	m_game->OnInitialized();
	if(IsServerRunning())
		RegisterServerInfo();
}

std::string ServerState::GetMessagePrefix() const { return std::string {Con::PREFIX_SERVER}; }

void ServerState::ChangeLevel(const std::string &map)
{
	NetworkState::ChangeLevel(map);
	auto *game = GetGameState();
	if(game != nullptr) {
		auto &mapInfo = game->GetMapInfo();
		ResourceManager::AddResource(mapInfo.fileName);
	}
	RegisterServerInfo();
	game->OnGameReady();
}

bool ServerState::IsMultiPlayer() const { return m_server && typeid(*m_server) != typeid(pragma::networking::LocalServer); }
bool ServerState::IsSinglePlayer() const { return !IsMultiPlayer(); }

ConVar *ServerState::SetConVar(std::string scmd, std::string value, bool bApplyIfEqual)
{
	auto *cvar = NetworkState::SetConVar(scmd, value, bApplyIfEqual);
	if(cvar == nullptr)
		return nullptr;
	auto flags = cvar->GetFlags();
	if(((flags & ConVarFlags::Replicated) == ConVarFlags::Replicated || (flags & ConVarFlags::Notify) == ConVarFlags::Notify)) {
		auto *cl = engine->GetClientState();
		if(cl != nullptr) {
			// This is a locally hosted game, just inform the client directly
			engine->SetReplicatedConVar(scmd, cvar->GetString());
			return cvar;
		}
		NetPacket p;
		p->WriteString(scmd);
		p->WriteString(cvar->GetString());
		SendPacket("cvar_set", p, pragma::networking::Protocol::SlowReliable);
	}
	return cvar;
}

pragma::SPlayerComponent *ServerState::GetPlayer(const pragma::networking::IServerClient &session) { return session.GetPlayer(); }
bool ServerState::IsServer() const { return true; }
ConVarMap *ServerState::GetConVarMap() { return console_system::server::get_convar_map(); }

void ServerState::ClearConCommands()
{
	m_luaConCommands.clear();
	m_conCommandIDs.clear();
	ConVarMap *map = GetConVarMap();
	if(map == NULL)
		m_conCommandID = 0;
	else
		m_conCommandID = map->GetConVarCount() + 1;
}

ConCommand *ServerState::CreateConCommand(const std::string &scmd, LuaFunction fc, ConVarFlags flags, const std::string &help)
{
	auto lscmd = scmd;
	ustring::to_lower(lscmd);
	auto *cmd = NetworkState::CreateConCommand(scmd, fc, flags, help);
	if(cmd == nullptr)
		return nullptr;
	cmd->m_ID = m_conCommandID;
	m_conCommandID++;
	m_luaConCommands.insert(decltype(m_luaConCommands)::value_type(scmd, cmd));
	m_conCommandIDs.insert(decltype(m_conCommandIDs)::value_type(scmd, cmd->GetID()));

	NetPacket packet;
	packet->WriteString(scmd);
	packet->Write<unsigned int>(cmd->GetID());
	SendPacket("luacmd_reg", packet, pragma::networking::Protocol::SlowReliable);
	return cmd;
}
WMServerData &ServerState::GetServerData() { return m_serverData; }

void ServerState::GetLuaConCommands(std::unordered_map<std::string, ConCommand *> **cmds) { *cmds = &m_luaConCommands; }

Material *ServerState::LoadMaterial(const std::string &path, bool precache, bool bReload)
{
	auto &matManager = GetMaterialManager();
	auto success = true;
	Material *mat = nullptr;
	if(precache) {
		success = matManager.PreloadAsset(path);
		return nullptr;
	}
	else if(bReload) {
		auto asset = matManager.ReloadAsset(path);
		success = (asset != nullptr);
		mat = asset.get();
	}
	else {
		util::FileAssetManager::PreloadResult result {};
		auto asset = matManager.LoadAsset(path, nullptr, &result);
		success = (asset != nullptr);
		mat = asset.get();
		if(!result && !result.firstTimeError)
			return nullptr;
	}
	if(!success) {
		static auto bSkipPort = false;
		if(bSkipPort == false) {
			bSkipPort = true;
			auto b = PortMaterial(path);
			bSkipPort = false;
			if(b == true)
				return mat;
		}
		Con::cwar << "Unable to load material '" << path << "': File not found!" << Con::endl;
	}
	return mat;
}

msys::MaterialManager &ServerState::GetMaterialManager() { return *engine->GetServerStateInstance().materialManager; }
ModelSubMesh *ServerState::CreateSubMesh() const { return new ModelSubMesh; }
ModelMesh *ServerState::CreateMesh() const { return new ModelMesh; }

REGISTER_CONVAR_CALLBACK_SV(sv_tickrate, [](NetworkState *, ConVar *, int, int val) {
	if(val < 0)
		val = 0;
	engine->SetTickRate(val);
});

////////////////

extern "C" {
DLLSERVER void pr_sv_create_server_state(std::unique_ptr<ServerState> &outState) { outState = std::make_unique<ServerState>(); }
DLLSERVER void pr_sv_start_server(bool singlePlayer)
{
	if(server == nullptr)
		return;
	server->StartServer(singlePlayer);
}
DLLSERVER void pr_sv_close_server()
{
	if(server == nullptr)
		return;
	server->CloseServer();
}
DLLSERVER bool pr_sv_is_server_running()
{
	if(server == nullptr)
		return false;
	return server->IsServerRunning();
}
DLLSERVER void pr_sv_get_server_steam_id(std::optional<uint64_t> &outSteamId)
{
	outSteamId = {};
	if(server == nullptr)
		return;
	auto *sv = server->GetServer();
	outSteamId = sv ? sv->GetSteamId() : std::optional<uint64_t> {};
}
DLLSERVER ServerState *pr_sv_get_server_state() { return server; }
DLLSERVER void pr_sv_clear_server_state() { server = nullptr; }
DLLSERVER void pr_sv_handle_local_host_player_server_packet(NetPacket &packet)
{
	if(server == nullptr)
		return;
	server->HandlePacket(*server->GetLocalClient(), packet);
}
DLLSERVER bool pr_sv_connect_local_host_player_client()
{
	if(server == nullptr)
		return false;
	return server->ConnectLocalHostPlayerClient();
}
}
