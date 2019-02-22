#include "stdafx_server.h"
#include <pragma/serverstate/serverstate.h>
#include <pragma/engine.h>
#include "pragma/entities/player.h"
#include "pragma/networking/clientsessioninfo.h"
#include "pragma/networking/wvserver.h"
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/networking/wvlocalclient.h"
#include <wms_shared.h>
#include <pragma/engine_version.h>
#include <networkmanager/nwm_error_handle.h>

extern DLLENGINE Engine *engine;
extern DLLSERVER SGame *s_game;
void ServerState::OnMasterServerRegistered(bool b,std::string reason)
{
	if(b == false)
	{
		m_tNextWMSConnect = std::chrono::high_resolution_clock::now();
		Con::cwar<<"WARNING: Unable to connect to master server ("<<reason<<")"<<Con::endl;
	}
}

void ServerState::RegisterServerInfo()
{
	if(m_server == nullptr || s_game == nullptr)
		return;
	if(s_game == nullptr)
		return;
	auto *gameMode = s_game->GetGameMode();
	m_serverData = WMServerData();
	m_serverData.name = GetConVarString("sv_servername");
	m_serverData.map = s_game->GetMapName();
	if(gameMode != nullptr)
		m_serverData.gameMode = gameMode->name;
	m_serverData.players = s_game->GetPlayerCount();
	m_serverData.tcpPort = m_server->GetLocalTCPPort();
	m_serverData.udpPort = m_server->GetLocalUDPPort();
	m_serverData.engineVersion = get_engine_version();
	//m_serverData.maxPlayers
	//m_serverData.password
	//m_serverData.bots
}

void ServerState::StartServer()
{
	CloseServer();
	auto tcpPort = GetConVarInt("sv_port_tcp");
	auto udpPort = GetConVarInt("sv_port_udp");
	try
	{
		m_server = WVServer::Create(static_cast<uint16_t>(tcpPort),static_cast<uint16_t>(udpPort),nwm::ConnectionType::TCPUDP);
		m_server->SetClientDroppedHandle([this](nwm::ServerClient *client,nwm::ClientDropped reason) {
			auto *game = GetGameState();
			if(game == nullptr)
				return;
			game->OnClientDropped(client,reason);
		});
	}
	catch(const NWMException &e)
	{
		Con::cerr<<"ERROR: Unable to start server with ports TCP = "<<tcpPort<<" and UDP = "<<udpPort<<": "<<e.what()<<Con::endl;
	}
	if(IsGameActive())
		RegisterServerInfo();
	// TODO
	/*if(m_ioservice != NULL) return;
	//CloseServer(); // TODO: Wait for it; Pass function?
#ifdef DEBUG_SOCKET
	Con::cout<<"[SERVER] Binding to port "<<port<<Con::endl;
#endif
	m_endpoint = new tcp::endpoint(tcp::v4(),port);
	m_ioservice = new boost::asio::io_service;
	Server *sv;
	try
	{
		sv = new Server(*m_ioservice,*m_endpoint);
	}
	catch(std::exception e)
	{
		Con::cwar<<"WARNING: Unable to start server at port "<<port<<": "<<e.what()<<Con::endl;
		delete m_ioservice;
		delete m_endpoint;
		m_ioservice = NULL;
		return;
	}
	m_server = new std::shared_ptr<Server>(sv);
	sv->SetPacketHandle(boost::bind(&ServerState::HandlePacket,this,_1,_2));
	sv->SetSessionHandle(boost::bind(&ServerState::HandleSession,this,_1));
	sv->SetDisconnectHandle(boost::bind(&ServerState::HandleDisconnect,this,_1));
	sv->SetTerminateHandle(boost::bind(&ServerState::HandleTerminate,this));
	sv->SetErrorHandle(boost::bind(&ServerState::HandleError,this,_1,_2,_3,_4));
	StartUDPSession(GetUDPPort());*/
}

void ServerState::CloseServer()
{
	if(m_server == nullptr)
		return;
	m_server->Shutdown();
	m_server = nullptr;
	/*if(m_ioservice == NULL) return;
	Server *server = m_server->get();
	if(!force)
	{
		SendTCPMessage("shutdown");
		server->Close();
		m_wmsClient->Close();
	}
	else
	{
		MarkSocketForTermination();
		InvalidateSocketHandles();
	}
	EndUDPSession();
	if(engine->IsServerOnly())
		engine->ShutDown();*/
}

/////////////////////////////////

DLLSERVER void CMD_startserver(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	if(argv.empty())
	{
		engine->StartServer();
		return;
	}
	engine->StartServer(); // atoi(argv[0].c_str())); // WVTODO
}

DLLSERVER void CMD_closeserver(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&) {engine->CloseServer();}
