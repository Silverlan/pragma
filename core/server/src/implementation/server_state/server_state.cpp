// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :server_state;

import :console.register_commands;
import :core;
import :entities;
import :entities.registration;
import :game;
import :model_manager;
import :networking;

#undef GetMessage

static std::unordered_map<std::string, std::shared_ptr<pragma::console::PtrConVar>> *conVarPtrs = nullptr;
std::unordered_map<std::string, std::shared_ptr<pragma::console::PtrConVar>> &pragma::ServerState::GetConVarPtrs() { return *conVarPtrs; }
pragma::console::ConVarHandle pragma::ServerState::GetConVarHandle(std::string scvar)
{
	if(conVarPtrs == nullptr) {
		static std::unordered_map<std::string, std::shared_ptr<console::PtrConVar>> ptrs;
		conVarPtrs = &ptrs;
	}
	return NetworkState::GetConVarHandle(*conVarPtrs, scvar);
}

static pragma::ServerState *g_server = nullptr;
pragma::ServerState *pragma::ServerState::Get() { return g_server; }
pragma::ServerState::ServerState() : NetworkState(), m_server(nullptr)
{
	m_alsoundID = 1;
	g_server = this;
	m_soundScriptManager = std::make_unique<audio::SoundScriptManager>();

	m_modelManager = std::make_unique<asset::SModelManager>(*this);
	Engine::Get()->InitializeAssetManager(*m_modelManager);
	pragma::asset::update_extension_cache(asset::Type::Model);

	fs::add_custom_mount_directory("cache", static_cast<fs::SearchFlags>(FSYS_SEARCH_CACHE));

	RegisterCallback<void, SGame *>("EndGame");
	RegisterCallback<void, SGame *>("OnGameStart");
	RegisterCallback<void, std::reference_wrapper<NetPacket>, std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketTCP");
	RegisterCallback<void, std::reference_wrapper<NetPacket>, std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketUDP");

	ResetGameServer();
}

pragma::ServerState::~ServerState()
{
	CloseServer();
	fs::remove_custom_mount_directory("cache");
	auto &conVarPtrs = GetConVarPtrs();
	for(auto itHandles = conVarPtrs.begin(); itHandles != conVarPtrs.end(); itHandles++)
		itHandles->second->set(nullptr);
	networking::ResourceManager::ClearResources();
	m_modelManager->Clear();
	GetMaterialManager().ClearUnused();

	ClearCommands();
}

void pragma::ServerState::InitializeGameServer(bool singlePlayerLocalGame)
{
	m_server = nullptr;
	m_serverReg = nullptr;

	networking::ServerEventInterface eventInterface {};
	eventInterface.onClientDropped = [this](networking::IServerClient &client, networking::DropReason reason) {
		auto *game = GetGameState();
		if(game == nullptr)
			return;
		game->OnClientDropped(client, reason);
	};
	eventInterface.onClientConnected = [](networking::IServerClient &client) {

	};
	eventInterface.handlePacket = [this](networking::IServerClient &client, NetPacket &packet) { HandlePacket(client, packet); };

	if(singlePlayerLocalGame == false) {
		auto netLibName = GetConVarString("net_library");
		auto netModPath = networking::GetNetworkingModuleLocation(netLibName, true);
		auto port = GetConVarInt("sv_port_tcp");
		auto usePeerToPeer = GetConVarBool("sv_use_p2p_if_available");
		std::string err;
		auto dllHandle = InitializeLibrary(netModPath, &err);
		if(dllHandle) {
			auto *fInitNetLib = dllHandle->FindSymbolAddress<void (*)(NetworkState &, std::unique_ptr<networking::IServer> &)>("initialize_game_server");
			if(fInitNetLib != nullptr) {
				fInitNetLib(*this, m_server);
				if(m_server) {
					m_server->SetEventInterface(eventInterface);
					networking::Error err;
					if(m_server->Start(err, port, usePeerToPeer) == false) {
						m_server = nullptr;
						m_serverReg = nullptr;
						Con::CERR << "Unable to start " << netLibName << " server: " << err.GetMessage() << Con::endl;
					}
				}
			}
			else
				Con::CERR << "Unable to initialize networking system '" << netLibName << "': Function 'initialize_game_server' not found in module!" << Con::endl;
		}
		else
			Con::CERR << "Unable to initialize networking system '" << netLibName << "': " << err << Con::endl;

		if(m_server) {
			std::string err;
			auto hLib = InitializeLibrary("steamworks/pr_steamworks", &err);
			if(hLib) {
				auto *game = static_cast<SGame *>(GetGameState());
				auto *gameMode = game ? game->GetGameMode() : nullptr;
				game::GameModeInfo;
				networking::GameServerInfo serverInfo {};
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
				m_serverReg = networking::MasterServerRegistration::Register(*hLib, serverInfo);
				if(m_serverReg) {
					networking::MasterServerRegistration::CallbackEvents cbEvents {};
					cbEvents.onAuthCompleted = [this](networking::MasterServerRegistration::SteamId steamId, bool authSuccess) {
						if(m_server == nullptr)
							return;
						auto &clients = m_server->GetClients();
						auto itCl = std::find_if(clients.begin(), clients.end(), [steamId](const std::shared_ptr<networking::IServerClient> &client) { return client->GetSteamId() == steamId; });
						if(itCl == clients.end())
							return;
						OnClientAuthenticated(**itCl, authSuccess);
					};
					m_serverReg->SetCallbackEvents(cbEvents);
				}
			}
			else
				Con::CERR << "Steamworks module could not be loaded! Server will not show up in steam server browser!" << Con::endl;
		}
	}
	else {
		m_server = std::make_unique<networking::LocalServer>();
		networking::Error err;
		m_server->Start(err, 0);
	}
	if(m_server == nullptr)
		ResetGameServer();
}
void pragma::ServerState::OnClientAuthenticated(networking::IServerClient &session, std::optional<bool> wasAuthenticationSuccessful)
{
	if(wasAuthenticationSuccessful.has_value() && *wasAuthenticationSuccessful == false) {
		spdlog::info("Authentication for client with steam id '{}' has failed, dropping client...", session.GetSteamId());
		DropClient(session, networking::DropReason::AuthenticationFailed);
		return;
	}
	NetPacket p;
	unsigned int numResources = networking::ResourceManager::GetResourceCount();
	p->Write<unsigned int>(numResources);
	SendPacket(networking::net_messages::client::START_RESOURCE_TRANSFER, p, networking::Protocol::SlowReliable, session);
}
bool pragma::ServerState::ConnectLocalHostPlayerClient()
{
	if(m_server == nullptr)
		return false;
	m_localClient = pragma::util::make_shared<networking::LocalServerClient>();
	m_server->AddClient(m_localClient);
	return true;
}
void pragma::ServerState::ResetGameServer()
{
	m_server = std::make_unique<networking::LocalServer>();
	//if(m_localClient == nullptr)
	//	m_localClient = pragma::util::make_shared<pragma::networking::LocalServerClient>();
	//m_server->AddClient(m_localClient);
}

void pragma::ServerState::Initialize()
{
	ClearConCommands();

	/*Con::CWAR<<"Server NetMessages:"<<Con::endl;
	ServerMessageMap *mapMsgs = GetNetMessageMap();
	std::unordered_map<std::string,unsigned int> *msgs;
	mapMsgs->GetNetMessages(&msgs);
	std::unordered_map<std::string,unsigned int>::iterator i;
	for(i=msgs->begin();i!=msgs->end();i++)
	{
		Con::COUT<<"Name: "<<i->first<<Con::endl;
	}*/

	//m_luaNetMessageIndex.clear();
	//m_luaNetMessageIndex.push_back("invalid");
	//m_stateServer->RegisterLua(); // WEAVETODO
	NetworkState::Initialize();
}

bool pragma::ServerState::LoadSoundScripts(const char *file, bool bPrecache)
{
	auto r = NetworkState::LoadSoundScripts(file, bPrecache);
	if(SGame::Get() != nullptr && r != false)
		SGame::Get()->RegisterGameResource(audio::SoundScriptManager::GetSoundScriptPath() + std::string(file));
	return r;
}

void pragma::ServerState::InitializeResourceManager() { m_resourceWatcher = std::make_unique<util::SResourceWatcherManager>(this); }

void pragma::ServerState::Close()
{
	Engine::Get()->SaveEngineConfig();
	Engine::Get()->SaveServerConfig();
	NetworkState::Close();
}

pragma::NwStateType pragma::ServerState::GetType() const { return NwStateType::Server; }

void pragma::ServerState::Think()
{
	NetworkState::Think();
	if(m_server) {
		if(m_server->IsRunning()) {
			networking::Error err;
			if(m_server->PollEvents(err) == false)
				Con::CWAR << "Server polling failed: " << err.GetMessage() << Con::endl;
		}
		if(m_serverReg)
			m_serverReg->UpdateServerData();
	}
}

void pragma::ServerState::Tick() { NetworkState::Tick(); }

void pragma::ServerState::implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const
{
	NetworkState::implFindSimilarConVars(input, similarCmds);

	auto *svMap = console::server::get_convar_map();
	FindSimilarConVars(input, svMap->GetConVars(), similarCmds);
}

pragma::SGame *pragma::ServerState::GetGameState() { return static_cast<SGame *>(NetworkState::GetGameState()); }

bool pragma::ServerState::IsGameActive() { return m_game != nullptr; }

void pragma::ServerState::EndGame()
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

	NetworkState::EndGame();
	ClearConCommands();
}

std::shared_ptr<pragma::audio::ALSound> pragma::ServerState::GetSoundByIndex(unsigned int idx)
{
	auto &snds = GetSounds();
	auto it = std::find_if(snds.begin(), snds.end(), [idx](const audio::ALSoundRef &rsound) { return (rsound.get().GetIndex() == idx) ? true : false; });
	if(it == snds.end())
		return nullptr;
	return it->get().shared_from_this();
}

void pragma::ServerState::StartGame(bool singlePlayer)
{
	NetworkState::StartGame(singlePlayer);
	StartServer(singlePlayer);
	m_game = {new SGame {this}, [](Game *game) {
		          game->OnRemove();
		          delete game;
	          }};

	// Register sound-scripts as game resources
	for(auto &f : m_soundScriptManager->GetSoundScriptFiles())
		GetGameState()->RegisterGameResource(f);
	for(auto &pair : m_soundScriptManager->GetScripts()) {
		auto &script = pair.second;
		std::function<void(audio::SoundScriptEventContainer &)> fIterateScript = nullptr;
		fIterateScript = [this, &fIterateScript](audio::SoundScriptEventContainer &c) {
			for(auto &ev : c.GetEvents()) {
				auto *evPlaySound = dynamic_cast<audio::SSEPlaySound *>(ev.get());
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

std::string pragma::ServerState::GetMessagePrefix() const { return std::string {Con::PREFIX_SERVER}; }

void pragma::ServerState::ChangeLevel(const std::string &map)
{
	NetworkState::ChangeLevel(map);
	auto *game = GetGameState();
	if(game != nullptr) {
		auto &mapInfo = game->GetMapInfo();
		networking::ResourceManager::AddResource(mapInfo.fileName);
	}
	RegisterServerInfo();
	game->OnGameReady();
}

bool pragma::ServerState::IsMultiPlayer() const { return m_server && typeid(*m_server) != typeid(networking::LocalServer); }
bool pragma::ServerState::IsSinglePlayer() const { return !IsMultiPlayer(); }

pragma::console::ConVar *pragma::ServerState::SetConVar(std::string scmd, std::string value, bool bApplyIfEqual)
{
	auto *cvar = NetworkState::SetConVar(scmd, value, bApplyIfEqual);
	if(cvar == nullptr)
		return nullptr;
	auto flags = cvar->GetFlags();
	if(((flags & console::ConVarFlags::Replicated) == console::ConVarFlags::Replicated || (flags & console::ConVarFlags::Notify) == console::ConVarFlags::Notify)) {
		auto *cl = Engine::Get()->GetClientState();
		if(cl != nullptr) {
			// This is a locally hosted game, just inform the client directly
			Engine::Get()->SetReplicatedConVar(scmd, cvar->GetString());
			return cvar;
		}
		NetPacket p;
		p->WriteString(scmd);
		p->WriteString(cvar->GetString());
		SendPacket(networking::net_messages::client::CVAR_SET, p, networking::Protocol::SlowReliable);
	}
	return cvar;
}

pragma::SPlayerComponent *pragma::ServerState::GetPlayer(const networking::IServerClient &session) { return static_cast<SPlayerComponent *>(session.GetPlayer()); }
bool pragma::ServerState::IsServer() const { return true; }
pragma::console::ConVarMap *pragma::ServerState::GetConVarMap() { return console::server::get_convar_map(); }

void pragma::ServerState::ClearConCommands()
{
	m_luaConCommands.clear();
	m_conCommandIDs.clear();
	console::ConVarMap *map = GetConVarMap();
	if(map == nullptr)
		m_conCommandID = 0;
	else
		m_conCommandID = map->GetConVarCount() + 1;
}

bool pragma::ServerState::IsClientAuthenticationRequired() const { return IsMultiPlayer() && Get()->GetConVarBool("sv_require_authentication"); }

pragma::console::ConCommand *pragma::ServerState::CreateConCommand(const std::string &scmd, LuaFunction fc, console::ConVarFlags flags, const std::string &help)
{
	auto lscmd = scmd;
	string::to_lower(lscmd);
	auto *cmd = NetworkState::CreateConCommand(scmd, fc, flags, help);
	if(cmd == nullptr)
		return nullptr;
	//cmd->m_ID = m_conCommandID;
	m_conCommandID++;
	m_luaConCommands.insert(decltype(m_luaConCommands)::value_type(scmd, cmd));
	m_conCommandIDs.insert(decltype(m_conCommandIDs)::value_type(scmd, cmd->GetID()));

	NetPacket packet;
	packet->WriteString(scmd);
	packet->Write<unsigned int>(cmd->GetID());
	SendPacket(networking::net_messages::client::LUACMD_REG, packet, networking::Protocol::SlowReliable);
	return cmd;
}
WMServerData &pragma::ServerState::GetServerData() { return m_serverData; }

void pragma::ServerState::GetLuaConCommands(std::unordered_map<std::string, console::ConCommand *> **cmds) { *cmds = &m_luaConCommands; }

pragma::material::Material *pragma::ServerState::LoadMaterial(const std::string &path, bool precache, bool bReload)
{
	auto &matManager = GetMaterialManager();
	auto success = true;
	material::Material *mat = nullptr;
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
		Con::CWAR << "Unable to load material '" << path << "': File not found!" << Con::endl;
	}
	return mat;
}

pragma::material::MaterialManager &pragma::ServerState::GetMaterialManager() { return *Engine::Get()->GetServerStateInstance().materialManager; }
pragma::geometry::ModelSubMesh *pragma::ServerState::CreateSubMesh() const { return new geometry::ModelSubMesh; }
pragma::geometry::ModelMesh *pragma::ServerState::CreateMesh() const { return new geometry::ModelMesh; }

namespace {
	auto _ = pragma::console::server::register_variable_listener<int>(
	  "sv_tickrate", +[](pragma::NetworkState *, const pragma::console::ConVar &, int, int val) {
		  if(val < 0)
			  val = 0;
		  pragma::Engine::Get()->SetTickRate(val);
	  });
}

////////////////

extern "C" {
DLLSERVER void pr_sv_register_server_entities()
{
	static auto entitiesRegistered = false;
	if(entitiesRegistered)
		return;
	entitiesRegistered = true;
	server_entities::register_entities();
}
DLLSERVER void pr_sv_register_server_net_messages()
{
	static auto netMessagesRegistered = false;
	if(netMessagesRegistered)
		return;
	netMessagesRegistered = true;
	pragma::networking::register_server_net_messages();
}
DLLSERVER void pr_sv_create_server_state(std::unique_ptr<pragma::NetworkState> &outState) { outState = std::make_unique<pragma::ServerState>(); }
DLLSERVER void pr_sv_start_server(bool singlePlayer)
{
	if(pragma::ServerState::Get() == nullptr)
		return;
	pragma::ServerState::Get()->StartServer(singlePlayer);
}
DLLSERVER void pr_sv_close_server()
{
	if(pragma::ServerState::Get() == nullptr)
		return;
	pragma::ServerState::Get()->CloseServer();
}
DLLSERVER bool pr_sv_is_server_running()
{
	if(pragma::ServerState::Get() == nullptr)
		return false;
	return pragma::ServerState::Get()->IsServerRunning();
}
DLLSERVER void pr_sv_get_server_steam_id(std::optional<uint64_t> &outSteamId)
{
	outSteamId = {};
	if(pragma::ServerState::Get() == nullptr)
		return;
	auto *sv = pragma::ServerState::Get()->GetServer();
	outSteamId = sv ? sv->GetSteamId() : std::optional<uint64_t> {};
}
DLLSERVER pragma::ServerState *pr_sv_get_server_state() { return pragma::ServerState::Get(); }
DLLSERVER void pr_sv_clear_server_state() { g_server = nullptr; }
DLLSERVER void pr_sv_handle_local_host_player_server_packet(NetPacket &packet)
{
	if(pragma::ServerState::Get() == nullptr)
		return;
	pragma::ServerState::Get()->HandlePacket(*pragma::ServerState::Get()->GetLocalClient(), packet);
}
DLLSERVER bool pr_sv_connect_local_host_player_client()
{
	if(pragma::ServerState::Get() == nullptr)
		return false;
	return pragma::ServerState::Get()->ConnectLocalHostPlayerClient();
}
}
