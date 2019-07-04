#include "stdafx_server.h"
#include "pragma/networking/standard_server.hpp"
#include "pragma/networking/netmessages.h"
#include "pragma/entities/player.h"
#include "wms_shared.h"
#include "wms_message.h"
#include "pragma/networking/wv_message.h"
#include "wmserverdata.h"
#include "pragma/entities/components/s_player_component.hpp"
#include <servermanager/connection/sv_nwm_tcpconnection.h>
#include <servermanager/connection/sv_nwm_udpconnection.h>
#include <servermanager/interface/sv_nwm_serverclient.hpp>
#include <pragma/networking/netmessages.h>
#include <pragma/networking/error.hpp>
#include <servermanager/interface/sv_nwm_manager_create.hpp>

#define DEBUG_SERVER_VERBOSE 1

#ifdef _DEBUG
#define GET_TIMEOUT_DURATION(f) 0.f
#else
#define GET_TIMEOUT_DURATION(f) f
#endif

extern DLLSERVER ServerState *server;

#pragma optimize("",off)
pragma::networking::NWMActiveServer::NWMActiveServer(const std::shared_ptr<SVNWMUDPConnection> &udp,const std::shared_ptr<SVNWMTCPConnection> &tcp)
	: nwm::Server(udp,tcp),m_lastHeartBeat()
{
	m_dispatcher = UDPMessageDispatcher::Create();
}
nwm::ServerClient &pragma::networking::NWMActiveServer::GetNWMClient(StandardServerClient &cl) const
{
	return cl.GetNWMClient();
}
pragma::networking::StandardServerClient &pragma::networking::NWMActiveServer::GetPragmaClient(nwm::ServerClient &cl) const
{
	return *m_nwmToPragmaClient.find(&cl)->second;
}
void pragma::networking::NWMActiveServer::SetPragmaServer(IServer *sv) {m_server = sv;}

void pragma::networking::NWMActiveServer::OnPacketSent(const NWMEndpoint &ep,const NetPacket &packet)
{
	nwm::Server::OnPacketSent(ep,packet);
#if DEBUG_SERVER_VERBOSE == 1
	auto id = packet.GetMessageID();
	auto *clMap = GetClientMessageMap();
	std::unordered_map<std::string,uint32_t> *clMsgs;
	clMap->GetNetMessages(&clMsgs);
	auto it = std::find_if(clMsgs->begin(),clMsgs->end(),[id](const std::pair<std::string,uint32_t> &pair) {
		return (pair.second == id) ? true : false;
		});
	std::string msgName = (it != clMsgs->end()) ? it->first : "Unknown";
	Con::csv<<"OnPacketSent: "<<msgName<<" ("<<id<<")"<<Con::endl;
#endif
	m_server->MemorizeNetMessage(MessageTracker::MessageType::Outgoing,packet.GetMessageID(),ep,packet);
}

void pragma::networking::NWMActiveServer::OnPacketReceived(const NWMEndpoint &ep,nwm::ServerClient *cl,unsigned int id,NetPacket &packet)
{
	nwm::Server::OnPacketReceived(ep,cl,id,packet);
#if DEBUG_SERVER_VERBOSE == 1
	auto *svMap = GetServerMessageMap();
	std::unordered_map<std::string,uint32_t> *svMsgs;
	svMap->GetNetMessages(&svMsgs);
	auto it = std::find_if(svMsgs->begin(),svMsgs->end(),[id](const std::pair<std::string,uint32_t> &pair) {
		return (pair.second == id) ? true : false;
		});
	std::string msgName = (it != svMsgs->end()) ? it->first : "Unknown";
	Con::csv<<"OnPacketReceived: "<<msgName<<" ("<<id<<")"<<Con::endl;
#endif
	m_server->MemorizeNetMessage(MessageTracker::MessageType::Incoming,id,ep,packet);
}

bool pragma::networking::NWMActiveServer::HandleAsyncPacket(const NWMEndpoint &ep,NWMSession *session,uint32_t id,NetPacket &packet)
{
	switch(static_cast<WVQuery>(id))
	{
	case WVQuery::PING:
	{
		NetPacket pong{id};
		auto *epPtr = ep.get();
		// Send response
		if(ep->IsUDP() == true)
		{
			NWMEndpoint ep = static_cast<NWMUDPEndpoint&>(*epPtr);
			nwm::impl::ManagerBase::SendPacket(nwm::Protocol::UDP,pong,ep);
		}
		else
		{
			NWMEndpoint ep = static_cast<NWMTCPEndpoint&>(*epPtr);
			nwm::impl::ManagerBase::SendPacket(nwm::Protocol::TCP,pong,ep);
		}
		return true;
	}
	}
	return false;
}

bool pragma::networking::NWMActiveServer::HandlePacket(const NWMEndpoint &ep,nwm::ServerClient *cl,unsigned int id,NetPacket &packet)
{
#if DEBUG_SERVER_VERBOSE == 1
	std::cout<<"HandlePacket: "<<id<<std::endl;
#endif
	if(nwm::Server::HandlePacket(ep,cl,id,packet) == true || cl == nullptr || !cl->IsFullyConnected())
		return true;
	auto it = m_nwmToPragmaClient.find(cl);
	if(it == m_nwmToPragmaClient.end())
		return false;
	auto *prCl = it->second;
	if(server->HandlePacket(*prCl,packet) == false)
		;
	return true;
	//return server->HandlePacket(static_cast<pragma::networking::NWMActiveServerClient*>(cl),packet);
}
void pragma::networking::NWMActiveServer::OnClientConnected(nwm::ServerClient *nwmCl)
{
#if DEBUG_SERVER_VERBOSE == 1
	std::cout<<"Client connected: "<<nwmCl->GetIP()<<std::endl;
#endif
	auto prCl = m_server->AddClient<StandardServerClient>();
	if(prCl == nullptr)
	{
		nwmCl->Drop(nwm::ClientDropped::Error);
		return;
	}
	prCl->SetNWMClient(nwmCl);
	m_nwmToPragmaClient[nwmCl] = prCl.get();
}

void pragma::networking::NWMActiveServer::OnClientDropped(nwm::ServerClient *cl,nwm::ClientDropped reason)
{
	auto it = m_nwmToPragmaClient.find(cl);
	if(it == m_nwmToPragmaClient.end())
		return;
	auto *prCl = it->second;
	auto *pl = server->GetPlayer(*prCl);
	if(pl)
		pl->GetEntity().RemoveSafely();
#if DEBUG_SERVER_VERBOSE == 1
	std::cout<<"Client dropped: "<<cl->GetIP()<<" ("<<nwm::client_dropped_enum_to_string(reason)<<")"<<std::endl;
#endif
	m_nwmToPragmaClient.erase(it);

	pragma::networking::Error err;
	m_server->DropClient(*prCl,get_pragma_drop_reason(reason),err);
}

void pragma::networking::NWMActiveServer::OnClosed()
{
#if DEBUG_SERVER_VERBOSE == 1
	std::cout<<"Connection closed..."<<std::endl;
#endif
}

void pragma::networking::NWMActiveServer::PollEvents()
{
	nwm::Server::PollEvents();
	if(m_dispatcher != nullptr)
		m_dispatcher->Poll();
	auto t = std::chrono::high_resolution_clock::now();
	auto tDelta = t -m_lastHeartBeat;
	if(std::chrono::duration_cast<std::chrono::minutes>(tDelta).count() >= 5)
	{
		m_lastHeartBeat = t;
		Heartbeat();
	}
}
void pragma::networking::NWMActiveServer::Heartbeat()
{
	if(m_dispatcher == nullptr || !server->IsGameActive())
		return;
	auto &data = server->GetServerData();
	DataStream body;
	data.Write(body);

	auto msgHeader = WMSMessageHeader(CUInt32(WMSMessage::HEARTBEAT));
	msgHeader.size = CUInt16(body->GetSize());
	DataStream header;
	header->Write<WMSMessageHeader>(msgHeader);

	m_dispatcher->Dispatch(header,GetMasterServerIP(),GetMasterServerPort(),[this,body](const nwm::ErrorCode err,UDPMessageDispatcher::Message*) mutable -> void {
		if(!err)
		{
			m_dispatcher->Dispatch(body,GetMasterServerIP(),GetMasterServerPort(),[](const nwm::ErrorCode err,UDPMessageDispatcher::Message*) -> void {
				if(err)
					Con::cwar<<"WARNING: Unable to reach master server: "<<err.Message()<<". The server will not show up in the server browser."<<Con::endl;
				});
		}
		else
			Con::cwar<<"WARNING: Unable to reach master server: "<<err.Message()<<". The server will not show up in the server browser."<<Con::endl;
		});
}
std::shared_ptr<nwm::ServerClient> pragma::networking::NWMActiveServer::CreateClient() {return Server::CreateClient<pragma::networking::NWMActiveServerClient>();}

std::unique_ptr<pragma::networking::NWMActiveServer> pragma::networking::NWMActiveServer::Create(uint16_t tcpPort,uint16_t udpPort,nwm::ConnectionType conType)
{
	auto r = nwm::Server::Create<pragma::networking::NWMActiveServer>(tcpPort,udpPort,conType);
	r->SetTimeoutDuration(GET_TIMEOUT_DURATION(server->GetConVarFloat("sv_timeout_duration")));
	r->Start();
	return r;
}

std::unique_ptr<pragma::networking::NWMActiveServer> pragma::networking::NWMActiveServer::Create(uint16_t port,nwm::ConnectionType conType)
{
	auto r = nwm::Server::Create<pragma::networking::NWMActiveServer>(port,conType);
	r->SetTimeoutDuration(GET_TIMEOUT_DURATION(server->GetConVarFloat("sv_timeout_duration")));
	r->Start();
	return r;
}

/////////////////

pragma::networking::NWMActiveServerClient::NWMActiveServerClient(nwm::Server *manager)
	: nwm::ServerClient(manager)
{}

pragma::networking::NWMActiveServerClient::~NWMActiveServerClient()
{}

void pragma::networking::NWMActiveServerClient::OnClosed() {}

/////////////////

pragma::networking::StandardServerClient::StandardServerClient()
{}

uint16_t pragma::networking::StandardServerClient::GetLatency() const
{
	return m_nwmClient->GetLatency();
}
std::string pragma::networking::StandardServerClient::GetIdentifier() const
{
	return m_nwmClient->GetAddress().ToString();
}
std::optional<nwm::IPAddress> pragma::networking::StandardServerClient::GetIPAddress() const
{
	return m_nwmClient->GetAddress();
}
std::optional<std::string> pragma::networking::StandardServerClient::GetIP() const
{
	return m_nwmClient->GetIP();
}
std::optional<pragma::networking::Port> pragma::networking::StandardServerClient::GetPort() const
{
	return m_nwmClient->GetPort();
}
bool pragma::networking::StandardServerClient::IsListenServerHost() const
{
	return true; // TODO
}
bool pragma::networking::StandardServerClient::Drop(DropReason reason,pragma::networking::Error &outErr)
{
	m_nwmClient->Drop(get_nwm_drop_reason(reason));
	return true;
}
bool pragma::networking::StandardServerClient::SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr)
{
	// TODO
	return true;
}
void pragma::networking::StandardServerClient::SetNWMClient(nwm::ServerClient *cl) {m_nwmClient = cl;}
nwm::ServerClient &pragma::networking::StandardServerClient::GetNWMClient() const {return *m_nwmClient;}

static void sv_timeout_duration_callback(NetworkState*,ConVar*,float,float val)
{
	if(server == nullptr)
		return;
	auto *sv = server->GetServer();
	if(sv == nullptr)
		return;
	sv->SetTimeoutDuration(GET_TIMEOUT_DURATION(val));
}
REGISTER_CONVAR_CALLBACK_SV(sv_timeout_duration,sv_timeout_duration_callback);
#pragma optimize("",on)
