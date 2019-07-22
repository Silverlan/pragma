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
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/console/convars.h>
#include "pragma/model/s_modelmanager.h"
#include "pragma/entities/components/s_resource_watcher.hpp"
#include "pragma/networking/networking_modules.hpp"
#include <pragma/networking/error.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>

static std::unordered_map<std::string,std::shared_ptr<PtrConVar>> *conVarPtrs = NULL;
std::unordered_map<std::string,std::shared_ptr<PtrConVar>> &ServerState::GetConVarPtrs() {return *conVarPtrs;}
ConVarHandle ServerState::GetConVarHandle(std::string scvar)
{
	if(conVarPtrs == NULL)
	{
		static std::unordered_map<std::string,std::shared_ptr<PtrConVar>> ptrs;
		conVarPtrs = &ptrs;
	}
	return NetworkState::GetConVarHandle(*conVarPtrs,scvar);
}

extern DLLENGINE Engine *engine;
DLLSERVER ServerState *server = nullptr;
DLLSERVER SGame *s_game = nullptr;
ServerState::ServerState()
	: NetworkState(),m_server(nullptr),
	m_game(nullptr)
{
	m_alsoundID = 1;
	server = this;
	m_soundScriptManager = std::make_unique<SoundScriptManager>();

	FileManager::AddCustomMountDirectory("cache",static_cast<fsys::SearchFlags>(FSYS_SEARCH_CACHE));

	RegisterCallback<void,SGame*>("EndGame");
	RegisterCallback<void,SGame*>("OnGameStart");
	RegisterCallback<void,std::reference_wrapper<NetPacket>,std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketTCP");
	RegisterCallback<void,std::reference_wrapper<NetPacket>,std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketUDP");

	ResetGameServer();
}

ServerState::~ServerState()
{
	CloseServer();
	FileManager::RemoveCustomMountDirectory("cache");
	auto &conVarPtrs = GetConVarPtrs();
	for(auto itHandles=conVarPtrs.begin();itHandles!=conVarPtrs.end();itHandles++)
		itHandles->second->set(NULL);
	ResourceManager::ClearResources();
	ModelManager::Clear();
	GetMaterialManager().ClearUnused();
}

void ServerState::InitializeGameServer()
{
	// TODO: Don't re-initialize server if local
	m_server = nullptr;

	pragma::networking::ServerEventInterface eventInterface {};
	eventInterface.onClientDropped = [this](pragma::networking::IServerClient &client,pragma::networking::DropReason reason) {
		auto *game = GetGameState();
		if(game == nullptr)
			return;
		game->OnClientDropped(client,reason);
	};
	eventInterface.onClientConnected = [](pragma::networking::IServerClient &client) {

	};
	eventInterface.handlePacket = [this](pragma::networking::IServerClient &client,NetPacket &packet) {
		HandlePacket(client,packet);
	};
#define USE_LOCAL_HOST 0
#if USE_LOCAL_HOST != 1
	auto netLibName = GetConVarString("net_library");
	auto netModPath = pragma::networking::GetNetworkingModuleLocation(netLibName,true);
	std::string err;
	auto dllHandle = InitializeLibrary(netModPath,&err);
	if(dllHandle)
	{
		auto *fInitNetLib = dllHandle->FindSymbolAddress<void(*)(NetworkState&,std::unique_ptr<pragma::networking::IServer>&)>("initialize_game_server");
		if(fInitNetLib != nullptr)
		{
			fInitNetLib(*this,m_server);
			if(m_server)
			{
				m_server->SetEventInterface(eventInterface);
				pragma::networking::Error err;
				if(m_server->Start(err) == false)
				{
					m_server = nullptr;
					Con::cerr<<"ERROR: Unable to start "<<netLibName<<" server: "<<err.GetMessage()<<Con::endl;
				}
			}
		}
		else
			Con::cerr<<"ERROR: Unable to initialize networking system '"<<netLibName<<"': Function 'initialize_game_server' not found in module!"<<Con::endl;
	}
	else
		Con::cerr<<"ERROR: Unable to initialize networking system '"<<netLibName<<"': "<<err<<Con::endl;
	if(m_server == nullptr)
	{
		ResetGameServer();
		return;
	}
	//m_server->AddClient(m_localClient);
#else
	{
		m_server = std::make_unique<pragma::networking::LocalServer>();
		pragma::networking::Error err;
		m_server->Start(err);
	}
#endif
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

bool ServerState::LoadSoundScripts(const char *file,bool bPrecache)
{
	auto r = NetworkState::LoadSoundScripts(file,bPrecache);
	if(s_game != nullptr && r != false)
		s_game->RegisterGameResource(SoundScriptManager::GetSoundScriptPath() +std::string(file));
	return r;
}

void ServerState::InitializeResourceManager() {m_resourceWatcher = std::make_unique<SResourceWatcherManager>(this);}

void ServerState::Close()
{
	engine->SaveEngineConfig();
	engine->SaveServerConfig();
	NetworkState::Close();
}

void ServerState::Think()
{
	NetworkState::Think();
	if(m_server != nullptr && m_server->IsRunning())
	{
		pragma::networking::Error err;
		if(m_server->PollEvents(err) == false)
			Con::cwar<<"WARNING: Server polling failed: "<<err.GetMessage()<<Con::endl;
	}
}

void ServerState::Tick()
{
	NetworkState::Tick();

}

void ServerState::implFindSimilarConVars(const std::string &input,std::vector<SimilarCmdInfo> &similarCmds) const
{
	NetworkState::implFindSimilarConVars(input,similarCmds);

	auto *svMap = console_system::server::get_convar_map();
	NetworkState::FindSimilarConVars(input,svMap->GetConVars(),similarCmds);
}

SGame *ServerState::GetGameState() {return m_game;}

bool ServerState::IsGameActive() {return m_game != nullptr;}

void ServerState::EndGame()
{
	if(!IsGameActive())
		return;
	CallCallbacks<void,SGame*>("EndGame",m_game);
	m_game->CallCallbacks<void>("EndGame");
	delete m_game;
	m_game = nullptr;
	s_game = nullptr;

	NetworkState::EndGame();
	ClearConCommands();
}

std::shared_ptr<ALSound> ServerState::GetSoundByIndex(unsigned int idx)
{
	auto &snds = GetSounds();
	auto it = std::find_if(snds.begin(),snds.end(),[idx](const ALSoundRef &rsound) {
		return (rsound.get().GetIndex() == idx) ? true : false;
	});
	if(it == snds.end())
		return nullptr;
	return it->get().shared_from_this();
}

void ServerState::StartGame()
{
	NetworkState::StartGame();
	StartServer();
	m_game = new SGame{this};
	s_game = m_game;

	// Register sound-scripts as game resources
	for(auto &f : m_soundScriptManager->GetSoundScriptFiles())
		m_game->RegisterGameResource(f);
	for(auto &pair : m_soundScriptManager->GetScripts())
	{
		auto &script = pair.second;
		std::function<void(SoundScriptEventContainer&)> fIterateScript = nullptr;
		fIterateScript = [this,&fIterateScript](SoundScriptEventContainer &c) {
			for(auto &ev : c.GetEvents())
			{
				auto *evPlaySound = dynamic_cast<SSEPlaySound*>(ev.get());
				if(evPlaySound != nullptr)
				{
					for(auto &src : evPlaySound->sources)
						m_game->RegisterGameResource("sounds\\" +src);
				}
				fIterateScript(*ev);
			}
		};
		fIterateScript(*script);
	}

	CallCallbacks<void,SGame*>("OnGameStart",m_game);
	m_game->Initialize();
	if(IsServerRunning())
		RegisterServerInfo();
}

Lua::ErrorColorMode ServerState::GetLuaErrorColorMode() {return Lua::ErrorColorMode::Cyan;}

void ServerState::LoadMap(const char *map,bool bDontReload)
{
	NetworkState::LoadMap(map,bDontReload);
	auto *game = GetGameState();
	if(game != nullptr)
	{
		auto &mapInfo = game->GetMapInfo();
		ResourceManager::AddResource(mapInfo.fileName);
	}
	RegisterServerInfo();
	game->OnGameReady();
}

bool ServerState::IsMultiPlayer() const {return (m_server == nullptr) ? false : true;}
bool ServerState::IsSinglePlayer() const {return !IsMultiPlayer();}

ConVar *ServerState::SetConVar(std::string scmd,std::string value,bool bApplyIfEqual)
{
	auto *cvar = NetworkState::SetConVar(scmd,value,bApplyIfEqual);
	if(cvar == nullptr)
		return nullptr;
	auto flags = cvar->GetFlags();
	if(((flags &ConVarFlags::Replicated) == ConVarFlags::Replicated || (flags &ConVarFlags::Notify) == ConVarFlags::Notify))
	{
		NetPacket p;
		p->WriteString(scmd);
		p->WriteString(cvar->GetString());
		SendPacket("cvar_set",p,pragma::networking::Protocol::SlowReliable);
	}
	return cvar;
}

pragma::SPlayerComponent *ServerState::GetPlayer(const pragma::networking::IServerClient &session) {return session.GetPlayer();}
bool ServerState::IsServer() const {return true;}
ConVarMap *ServerState::GetConVarMap() {return console_system::server::get_convar_map();}

void ServerState::ClearConCommands()
{
	m_luaConCommands.clear();
	m_conCommandIDs.clear();
	ConVarMap *map = GetConVarMap();
	if(map == NULL)
		m_conCommandID = 0;
	else
		m_conCommandID = map->GetConVarCount() +1;
}

ConCommand *ServerState::CreateConCommand(const std::string &scmd,LuaFunction fc,ConVarFlags flags,const std::string &help)
{
	auto lscmd = scmd;
	ustring::to_lower(lscmd);
	auto *cmd = NetworkState::CreateConCommand(scmd,fc,flags,help);
	if(cmd == nullptr)
		return nullptr;
	cmd->m_ID = m_conCommandID;
	m_conCommandID++;
	m_luaConCommands.insert(decltype(m_luaConCommands)::value_type(scmd,cmd));
	m_conCommandIDs.insert(decltype(m_conCommandIDs)::value_type(scmd,cmd->GetID()));

	NetPacket packet;
	packet->WriteString(scmd);
	packet->Write<unsigned int>(cmd->GetID());
	SendPacket("luacmd_reg",packet,pragma::networking::Protocol::SlowReliable);
	return cmd;
}
WMServerData &ServerState::GetServerData() {return m_serverData;}

void ServerState::GetLuaConCommands(std::unordered_map<std::string,ConCommand*> **cmds) {*cmds = &m_luaConCommands;}

Material *ServerState::LoadMaterial(const std::string &path,bool bReload)
{
	bool bFirstTimeError;
	auto *mat = GetMaterialManager().Load(path,bReload,&bFirstTimeError);
	if(bFirstTimeError == true)
	{
		static auto bSkipPort = false;
		if(bSkipPort == false)
		{
			bSkipPort = true;
			auto b = PortMaterial(path,[this,&mat](const std::string &path,bool bReload) -> Material* {
				return mat = LoadMaterial(path,bReload);
			});
			bSkipPort = false;
			if(b == true)
				return mat;
		}
		Con::cwar<<"WARNING: Unable to load material '"<<path<<"': File not found!"<<Con::endl;
	}
	return mat;
}

MaterialManager &ServerState::GetMaterialManager() {return *engine->GetServerStateInstance().materialManager;}
ModelSubMesh *ServerState::CreateSubMesh() const {return new ModelSubMesh;}
ModelMesh *ServerState::CreateMesh() const {return new ModelMesh;}

REGISTER_CONVAR_CALLBACK_SV(sv_tickrate,[](NetworkState*,ConVar*,int,int val) {
	if(val < 0)
		val = 0;
	engine->SetTickRate(val);
});
