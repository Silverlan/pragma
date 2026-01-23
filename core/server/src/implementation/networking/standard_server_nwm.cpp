// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.server;
import :networking.standard_server;

import :console.register_commands;
import :entities;
import :entities.components;
import :server_state;
import pragma.wms;

#define DEBUG_SERVER_VERBOSE 1

#ifdef _DEBUG
#define GET_TIMEOUT_DURATION(f) 0.f
#else
#define GET_TIMEOUT_DURATION(f) f
#endif

pragma::networking::NWMActiveServer::NWMActiveServer(const std::shared_ptr<SVNWMUDPConnection> &udp, const std::shared_ptr<SVNWMTCPConnection> &tcp) : Server(udp, tcp), m_lastHeartBeat() { m_dispatcher = UDPMessageDispatcher::Create(); }
nwm::ServerClient &pragma::networking::NWMActiveServer::GetNWMClient(StandardServerClient &cl) const { return cl.GetNWMClient(); }
pragma::networking::StandardServerClient &pragma::networking::NWMActiveServer::GetPragmaClient(nwm::ServerClient &cl) const { return *m_nwmToPragmaClient.find(&cl)->second; }
void pragma::networking::NWMActiveServer::SetServer(StandardServer &server) { m_server = &server; }

void pragma::networking::NWMActiveServer::OnPacketSent(const NWMEndpoint &ep, const NetPacket &packet)
{
	Server::OnPacketSent(ep, packet);
#if DEBUG_SERVER_VERBOSE == 1
	auto id = packet.GetMessageID();
	auto *clMap = get_client_message_map();
	util::StringMap<uint32_t> *clMsgs;
	clMap->GetNetMessages(&clMsgs);
	auto it = std::find_if(clMsgs->begin(), clMsgs->end(), [id](const std::pair<std::string, uint32_t> &pair) { return (pair.second == id) ? true : false; });
	std::string msgName = (it != clMsgs->end()) ? it->first : "Unknown";
	Con::CSV << "OnPacketSent: " << msgName << " (" << id << ")" << Con::endl;
#endif
	m_server->MemorizeNetMessage(MessageTracker::MessageType::Outgoing, packet.GetMessageID(), ep, packet);
}

void pragma::networking::NWMActiveServer::OnPacketReceived(const NWMEndpoint &ep, nwm::ServerClient *cl, unsigned int id, NetPacket &packet)
{
	Server::OnPacketReceived(ep, cl, id, packet);
#if DEBUG_SERVER_VERBOSE == 1
	auto *svMap = get_server_message_map();
	util::StringMap<uint32_t> *svMsgs;
	svMap->GetNetMessages(&svMsgs);
	auto it = std::find_if(svMsgs->begin(), svMsgs->end(), [id](const std::pair<std::string, uint32_t> &pair) { return (pair.second == id) ? true : false; });
	std::string msgName = (it != svMsgs->end()) ? it->first : "Unknown";
	Con::CSV << "OnPacketReceived: " << msgName << " (" << id << ")" << Con::endl;
#endif
	m_server->MemorizeNetMessage(MessageTracker::MessageType::Incoming, id, ep, packet);
}

bool pragma::networking::NWMActiveServer::HandleAsyncPacket(const NWMEndpoint &ep, NWMSession *session, uint32_t id, NetPacket &packet)
{
	switch(static_cast<ServerQuery>(id)) {
	case ServerQuery::Ping:
		{
			NetPacket pong {id};
			auto *epPtr = ep.get();
			// Send response
			if(ep->IsUDP() == true) {
				NWMEndpoint ep = static_cast<NWMUDPEndpoint &>(*epPtr);
				ManagerBase::SendPacket(nwm::Protocol::UDP, pong, ep);
			}
			else {
				NWMEndpoint ep = static_cast<NWMTCPEndpoint &>(*epPtr);
				ManagerBase::SendPacket(nwm::Protocol::TCP, pong, ep);
			}
			return true;
		}
	}
	return false;
}

bool pragma::networking::NWMActiveServer::HandlePacket(const NWMEndpoint &ep, nwm::ServerClient *cl, unsigned int id, NetPacket &packet)
{
#if DEBUG_SERVER_VERBOSE == 1
	std::cout << "HandlePacket: " << id << std::endl;
#endif
	if(Server::HandlePacket(ep, cl, id, packet) == true || cl == nullptr || !cl->IsFullyConnected())
		return true;
	auto it = m_nwmToPragmaClient.find(cl);
	if(it == m_nwmToPragmaClient.end())
		return false;
	auto *prCl = it->second;
	m_server->HandlePacket(*prCl, packet);
	return true;
}
void pragma::networking::NWMActiveServer::OnClientConnected(nwm::ServerClient *nwmCl)
{
#if DEBUG_SERVER_VERBOSE == 1
	std::cout << "Client connected: " << nwmCl->GetIP() << std::endl;
#endif
	auto prCl = m_server->AddClient<StandardServerClient>();
	if(prCl == nullptr) {
		nwmCl->Drop(nwm::ClientDropped::Error);
		return;
	}
	prCl->SetNWMClient(nwmCl);
	m_nwmToPragmaClient[nwmCl] = prCl.get();
	m_server->OnClientConnected(*prCl);
}

void pragma::networking::NWMActiveServer::OnClientDropped(nwm::ServerClient *cl, nwm::ClientDropped reason)
{
	auto it = m_nwmToPragmaClient.find(cl);
	if(it == m_nwmToPragmaClient.end())
		return;
	auto *prCl = it->second;
	auto *pl = ServerState::Get()->GetPlayer(*prCl);
	if(pl)
		pl->GetEntity().RemoveSafely();
#if DEBUG_SERVER_VERBOSE == 1
	std::cout << "Client dropped: " << cl->GetIP() << " (" << nwm::client_dropped_enum_to_string(reason) << ")" << std::endl;
#endif
	m_nwmToPragmaClient.erase(it);

	Error err;
	m_server->DropClient(*prCl, get_pragma_drop_reason(reason), err);
	m_server->OnClientDropped(*prCl, get_pragma_drop_reason(reason));
}

void pragma::networking::NWMActiveServer::OnClosed()
{
#if DEBUG_SERVER_VERBOSE == 1
	std::cout << "Connection closed..." << std::endl;
#endif
}

void pragma::networking::NWMActiveServer::PollEvents()
{
	Server::PollEvents();
	if(m_dispatcher != nullptr)
		m_dispatcher->Poll();
	auto t = util::Clock::now();
	auto tDelta = t - m_lastHeartBeat;
	if(std::chrono::duration_cast<std::chrono::minutes>(tDelta).count() >= 5) {
		m_lastHeartBeat = t;
		Heartbeat();
	}
}
void pragma::networking::NWMActiveServer::Heartbeat()
{
	if(m_dispatcher == nullptr || !ServerState::Get()->IsGameActive())
		return;
	auto &data = ServerState::Get()->GetServerData();
	util::DataStream body;
	data.Write(body);

	auto msgHeader = WMSMessageHeader(CUInt32(WMSMessage::HEARTBEAT));
	msgHeader.size = CUInt16(body->GetSize());
	util::DataStream header;
	header->Write<WMSMessageHeader>(msgHeader);

	m_dispatcher->Dispatch(header, GetMasterServerIP(), GetMasterServerPort(), [this, body](const nwm::ErrorCode err, UDPMessageDispatcher::Message *) mutable -> void {
		if(!err) {
			m_dispatcher->Dispatch(body, GetMasterServerIP(), GetMasterServerPort(), [](const nwm::ErrorCode err, UDPMessageDispatcher::Message *) -> void {
				if(err)
					Con::CWAR << "Unable to reach master server: " << err.Message() << ". The server will not show up in the server browser." << Con::endl;
			});
		}
		else
			Con::CWAR << "Unable to reach master server: " << err.Message() << ". The server will not show up in the server browser." << Con::endl;
	});
}
std::shared_ptr<nwm::ServerClient> pragma::networking::NWMActiveServer::CreateClient() { return nwm::Server::CreateClient<NWMActiveServerClient>(); }

std::unique_ptr<pragma::networking::NWMActiveServer> pragma::networking::NWMActiveServer::Create(uint16_t tcpPort, uint16_t udpPort, nwm::ConnectionType conType)
{
	auto r = nwm::Server::Create<NWMActiveServer>(tcpPort, udpPort, conType);
	r->SetTimeoutDuration(GET_TIMEOUT_DURATION(ServerState::Get()->GetConVarFloat("sv_timeout_duration")));
	r->Start();
	return r;
}

std::unique_ptr<pragma::networking::NWMActiveServer> pragma::networking::NWMActiveServer::Create(uint16_t port, nwm::ConnectionType conType)
{
	auto r = nwm::Server::Create<NWMActiveServer>(port, conType);
	r->SetTimeoutDuration(GET_TIMEOUT_DURATION(ServerState::Get()->GetConVarFloat("sv_timeout_duration")));
	r->Start();
	return r;
}

/////////////////

pragma::networking::NWMActiveServerClient::NWMActiveServerClient(nwm::Server *manager) : ServerClient(manager) {}

pragma::networking::NWMActiveServerClient::~NWMActiveServerClient() {}

void pragma::networking::NWMActiveServerClient::OnClosed() {}

/////////////////

pragma::networking::StandardServerClient::StandardServerClient() {}

uint16_t pragma::networking::StandardServerClient::GetLatency() const { return m_nwmClient->GetLatency(); }
std::string pragma::networking::StandardServerClient::GetIdentifier() const { return m_nwmClient->GetAddress().ToString(); }
std::optional<std::string> pragma::networking::StandardServerClient::GetIP() const { return m_nwmClient->GetIP(); }
std::optional<pragma::networking::Port> pragma::networking::StandardServerClient::GetPort() const { return m_nwmClient->GetPort(); }
bool pragma::networking::StandardServerClient::IsListenServerHost() const
{
	return true; // TODO
}
bool pragma::networking::StandardServerClient::Drop(DropReason reason, Error &outErr)
{
	m_nwmClient->Drop(get_nwm_drop_reason(reason));
	return true;
}
bool pragma::networking::StandardServerClient::SendPacket(Protocol protocol, NetPacket &packet, Error &outErr)
{
	// TODO
	return true;
}
void pragma::networking::StandardServerClient::SetNWMClient(nwm::ServerClient *cl) { m_nwmClient = cl; }
nwm::ServerClient &pragma::networking::StandardServerClient::GetNWMClient() const { return *m_nwmClient; }

static void sv_timeout_duration_callback(pragma::NetworkState *, const pragma::console::ConVar &, float, float val)
{
	if(pragma::ServerState::Get() == nullptr)
		return;
	auto *sv = pragma::ServerState::Get()->GetServer();
	if(sv == nullptr)
		return;
	sv->SetTimeoutDuration(GET_TIMEOUT_DURATION(val));
}
namespace {
	auto UVN = pragma::console::server::register_variable_listener<float>("sv_timeout_duration", &sv_timeout_duration_callback);
}
