// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"

import pragma.client;


#define DEBUG_CLIENT_VERBOSE 1

void pragma::networking::NWMClientConnection::OnPacketSent(const NWMEndpoint &ep, const NetPacket &packet)
{
	nwm::Client::OnPacketSent(ep, packet);
#if DEBUG_CLIENT_VERBOSE == 1
	auto id = packet.GetMessageID();
	auto *svMap = GetServerMessageMap();
	util::StringMap<uint32_t> *svMsgs;
	svMap->GetNetMessages(&svMsgs);
	auto it = std::find_if(svMsgs->begin(), svMsgs->end(), [id](const std::pair<std::string, uint32_t> &pair) { return (pair.second == id) ? true : false; });
	std::string msgName = (it != svMsgs->end()) ? it->first : "Unknown";
	Con::ccl << "OnPacketSent: " << msgName << " (" << id << ")" << Con::endl;
#endif
	MemorizeNetMessage(MessageTracker::MessageType::Outgoing, packet.GetMessageID(), ep, packet);
}
void pragma::networking::NWMClientConnection::OnPacketReceived(const NWMEndpoint &ep, unsigned int id, NetPacket &packet)
{
	nwm::Client::OnPacketReceived(ep, id, packet);
#if DEBUG_CLIENT_VERBOSE == 1
	auto *clMap = GetClientMessageMap();
	util::StringMap<uint32_t> *clMsgs;
	clMap->GetNetMessages(&clMsgs);
	auto it = std::find_if(clMsgs->begin(), clMsgs->end(), [id](const std::pair<std::string, uint32_t> &pair) { return (pair.second == id) ? true : false; });
	std::string msgName = (it != clMsgs->end()) ? it->first : "Unknown";
	Con::ccl << "OnPacketReceived: " << msgName << " (" << id << ")" << Con::endl;
#endif
	MemorizeNetMessage(MessageTracker::MessageType::Incoming, id, ep, packet);
}

bool pragma::networking::NWMClientConnection::HandlePacket(const NWMEndpoint &ep, unsigned int id, NetPacket &packet)
{
#if DEBUG_CLIENT_VERBOSE == 1
	Con::ccl << "HandlePacket: " << id << Con::endl;
#endif
	if(nwm::Client::HandlePacket(ep, id, packet) == true)
		return true;
	m_client->HandlePacket(packet);
	return true;
}
void pragma::networking::NWMClientConnection::OnConnected()
{
#if DEBUG_CLIENT_VERBOSE == 1
	Con::ccl << "Connected to server..." << Con::endl;
#endif
	m_client->OnConnected();
}
void pragma::networking::NWMClientConnection::OnClosed()
{
	auto err = GetLastError();
#if DEBUG_CLIENT_VERBOSE == 1
	Con::ccl << "Connection to server has closed: " << err.Message() << Con::endl;
#endif
	m_client->OnConnectionClosed();
}
void pragma::networking::NWMClientConnection::OnDisconnected(nwm::ClientDropped reason)
{
#if DEBUG_CLIENT_VERBOSE == 1
	Con::ccl << "Disconnected from server (" << nwm::client_dropped_enum_to_string(reason) << ")..." << Con::endl;
#endif
	m_bDisconnected = true;
	m_client->OnDisconnected();
}

pragma::networking::NWMClientConnection::NWMClientConnection(const std::shared_ptr<CLNWMUDPConnection> &udp, std::shared_ptr<CLNWMTCPConnection> &tcp) : nwm::Client(udp, tcp) {}

std::unique_ptr<pragma::networking::NWMClientConnection> pragma::networking::NWMClientConnection::Create(const std::string &serverIp, unsigned short serverPort)
{
	std::unique_ptr<NWMClientConnection> cl = nullptr;
	NWMException lastException("No error.");
	for(unsigned short localPort = 27018; localPort < 27034; localPort++) {
		try {
			cl = nwm::Client::Create<NWMClientConnection>(serverIp, serverPort, localPort, nwm::ConnectionType::TCPUDP);
			break;
		}
		catch(NWMException &e) {
			lastException = e;
		}
	}
	if(cl == nullptr) {
		Con::cwar << "Unable to connect to server '" << serverIp << ":" << serverPort << ": " << lastException.what() << Con::endl;
		return nullptr;
	}
#ifdef _DEBUG
	cl->SetTimeoutDuration(0.f);
#else
	cl->SetTimeoutDuration(pragma::get_client_state()->GetConVarFloat("sv_timeout_duration"));
#endif
	//cl->SetPingEnabled(false);
	cl->Start();
	return cl;
}

void pragma::networking::NWMClientConnection::SetClient(StandardClient &client) { m_client = &client; }

bool pragma::networking::NWMClientConnection::IsDisconnected() const { return m_bDisconnected; }

namespace {
	auto _ = pragma::console::client::register_variable_listener<float>("sv_timeout_duration", +[](NetworkState *, const ConVar &, float, float val) {
		auto *client = pragma::get_client_state();
		if(client == nullptr)
			return;
		auto *cl = client->GetClient();
		if(cl == nullptr)
			return;
		cl->SetTimeoutDuration(val);
	});
};
