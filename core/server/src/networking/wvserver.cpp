#include "stdafx_server.h"
#include "pragma/networking/wvserver.h"
#include "pragma/networking/wvserverclient.h"
#include "pragma/networking/netmessages.h"
#include "pragma/entities/player.h"
#include "wms_shared.h"
#include "wms_message.h"
#include "pragma/networking/wv_message.h"
#include "wmserverdata.h"
#include "pragma/entities/components/s_player_component.hpp"
#include <servermanager/connection/sv_nwm_tcpconnection.h>
#include <servermanager/connection/sv_nwm_udpconnection.h>
#include <pragma/networking/netmessages.h>
#include <servermanager/interface/sv_nwm_manager_create.hpp>


#define DEBUG_SERVER_VERBOSE 0

#ifdef _DEBUG
	#define GET_TIMEOUT_DURATION(f) 0.f
#else
	#define GET_TIMEOUT_DURATION(f) f
#endif

extern DLLSERVER ServerState *server;

WVServer::WVServer(const std::shared_ptr<SVNWMUDPConnection> &udp,const std::shared_ptr<SVNWMTCPConnection> &tcp)
	: nwm::Server(udp,tcp),m_lastHeartBeat()
{
	m_dispatcher = UDPMessageDispatcher::Create();
}

void WVServer::OnPacketSent(const NWMEndpoint &ep,const NetPacket &packet)
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
	MemorizeNetMessage(MessageTracker::MessageType::Outgoing,packet.GetMessageID(),ep,packet);
}

void WVServer::OnPacketReceived(const NWMEndpoint &ep,nwm::ServerClient *cl,unsigned int id,NetPacket &packet)
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
	MemorizeNetMessage(MessageTracker::MessageType::Incoming,id,ep,packet);
}

bool WVServer::HandleAsyncPacket(const NWMEndpoint &ep,NWMSession *session,uint32_t id,NetPacket &packet)
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

bool WVServer::HandlePacket(const NWMEndpoint &ep,nwm::ServerClient *cl,unsigned int id,NetPacket &packet)
{
#if DEBUG_SERVER_VERBOSE == 1
	std::cout<<"HandlePacket: "<<id<<std::endl;
#endif
	if(nwm::Server::HandlePacket(ep,cl,id,packet) == true || cl == nullptr || !cl->IsFullyConnected())
		return true;
	if(server->HandlePacket(static_cast<WVServerClient*>(cl),packet) == false)
		;
	return true;
	//return server->HandlePacket(static_cast<WVServerClient*>(cl),packet);
}
void WVServer::OnClientConnected(nwm::ServerClient *cl)
{
#if DEBUG_SERVER_VERBOSE == 1
	std::cout<<"Client connected: "<<cl->GetIP()<<std::endl;
#endif
}

void WVServer::OnClientDropped(nwm::ServerClient *cl,nwm::ClientDropped reason)
{
	auto *pl = server->GetPlayer(static_cast<WVServerClient*>(cl));
	if(pl != nullptr)
		pl->GetEntity().RemoveSafely();
#if DEBUG_SERVER_VERBOSE == 1
	std::cout<<"Client dropped: "<<cl->GetIP()<<" ("<<nwm::client_dropped_enum_to_string(reason)<<")"<<std::endl;
#endif
}

void WVServer::OnClosed()
{
#if DEBUG_SERVER_VERBOSE == 1
	std::cout<<"Connection closed..."<<std::endl;
#endif
}

void WVServer::PollEvents()
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
void WVServer::Heartbeat()
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
std::shared_ptr<nwm::ServerClient> WVServer::CreateClient() {return Server::CreateClient<WVServerClient>();}

std::unique_ptr<WVServer> WVServer::Create(uint16_t tcpPort,uint16_t udpPort,nwm::ConnectionType conType)
{
	auto r = nwm::Server::Create<WVServer>(tcpPort,udpPort,conType);
	r->SetTimeoutDuration(GET_TIMEOUT_DURATION(server->GetConVarFloat("sv_timeout_duration")));
	r->Start();
	return r;
}

std::unique_ptr<WVServer> WVServer::Create(uint16_t port,nwm::ConnectionType conType)
{
	auto r = nwm::Server::Create<WVServer>(port,conType);
	r->SetTimeoutDuration(GET_TIMEOUT_DURATION(server->GetConVarFloat("sv_timeout_duration")));
	r->Start();
	return r;
}

REGISTER_CONVAR_CALLBACK_SV(sv_timeout_duration,[](NetworkState*,ConVar*,float,float val) {
	if(server == nullptr)
		return;
	auto *sv = server->GetServer();
	if(sv == nullptr)
		return;
	sv->SetTimeoutDuration(GET_TIMEOUT_DURATION(val));
});