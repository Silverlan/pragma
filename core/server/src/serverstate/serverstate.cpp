#include "stdafx_server.h"
#include "pragma/networking/wvserver.h"
#include <pragma/serverstate/serverstate.h>
#include "pragma/networking/netmessages.h"
#include "pragma/networking/resourcemanager.h"
#include "pragma/game/s_game.h"
#include "pragma/entities/player.h"
#include "pragma/networking/clientsessioninfo.h"
#include "pragma/console/convarhandle.h"
#include <pragma/audio/soundscript.h>
#include "luasystem.h"
#include "pragma/networking/wvserverclient.h"
#include "pragma/networking/wvlocalclient.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/console/convars.h>
#include "pragma/model/s_modelmanager.h"
#include "pragma/entities/components/s_resource_watcher.hpp"
#include <sharedutils/util_file.h>

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

	if(!engine->IsServerOnly())
		m_local = std::unique_ptr<WVLocalClient>(new WVLocalClient);
	else m_local = nullptr;
	FileManager::AddCustomMountDirectory("cache",static_cast<fsys::SearchFlags>(FSYS_SEARCH_CACHE));

	RegisterCallback<void,SGame*>("EndGame");
	RegisterCallback<void,SGame*>("OnGameStart");
	RegisterCallback<void,std::reference_wrapper<NetPacket>,std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketTCP");
	RegisterCallback<void,std::reference_wrapper<NetPacket>,std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketUDP");
}

ServerState::~ServerState()
{
	/*if(m_wmsClient != nullptr)
	{
		m_wmsClient->Close();
		while(m_wmsClient->IsActive())
			m_wmsClient->Run();
		delete m_wmsClient;
	}*/
	FileManager::RemoveCustomMountDirectory("cache");
	std::unordered_map<std::string,std::shared_ptr<PtrConVar>> &conVarPtrs = GetConVarPtrs();
	std::unordered_map<std::string,std::shared_ptr<PtrConVar>>::iterator itHandles;
	for(itHandles=conVarPtrs.begin();itHandles!=conVarPtrs.end();itHandles++)
		itHandles->second->set(NULL);
	ResourceManager::ClearResources();
	ModelManager::Clear();
	GetMaterialManager().ClearUnused();
	if(m_server != nullptr)
		m_server->Close();
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
	if(m_server != nullptr && m_server->IsActive())
		m_server->PollEvents();
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
	/*if(m_server != NULL)
	{
		std::vector<boost::shared_ptr<ClientSession>> *sessions;
		m_server->get()->GetSessions(&sessions);
		for(unsigned int i=0;i<sessions->size();i++)
		{
			ClientSession *session = (*sessions)[i].get();
			if(session->userData != NULL)
			{
				ClientSessionInfo *info = static_cast<ClientSessionInfo*>(session->userData);
				info->SetPlayer(NULL);
			}
		}
	}*/
	NetworkState::EndGame();
	ClearConCommands();
}
/*
void ServerState::HandleUDPPacket(UDPSession *session,NetPacket &packet,udp::endpoint ep)
{
	boost::asio::ip::address address = ep.address();
	unsigned short port = ep.port();
	std::vector<Player*> *players;
	Player::GetAll(&players);
	for(int i=0;i<players->size();i++)
	{
		Player *pl = (*players)[i];
		ClientSession *session = pl->GetClientSession();
		if(session != NULL)
		{
			tcp::socket *socketPl = session->GetSocket();
			if(socketPl != NULL)
			{
				tcp::endpoint epPl = socketPl->remote_endpoint();
				if(address == epPl.address() && pl->GetUDPPort() == port) // TODO: Find a better way to get our target
				{
					HandlePacket(session,packet);
					break;
				}
			}
		}
	}
}
*/ // WVTODO
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
	if(m_local != nullptr)
		m_local->Reset();
	NetworkState::StartGame();
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
		BroadcastTCP("cvar_set",p);
	}
	return cvar;
}

pragma::SPlayerComponent *ServerState::GetPlayer(WVServerClient *session)
{
	ClientSessionInfo *info = session->GetSessionInfo();
	if(info == nullptr)
		return nullptr;
	return info->GetPlayer();
}
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
	BroadcastTCP("luacmd_reg",packet);
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
