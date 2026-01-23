// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :networking.iserver;

import :networking.iserver_client;
import :networking.recipient_filter;

bool pragma::networking::IServer::Shutdown(Error &outErr)
{
	for(auto &cl : m_clients)
		cl->Drop(DropReason::Shutdown, outErr);
	auto result = DoShutdown(outErr);
	m_bRunning = false;
	return result;
}
bool pragma::networking::IServer::SendPacket(Protocol protocol, NetPacket &packet, const ClientRecipientFilter &rf, Error &outErr)
{
	auto success = true;
	for(auto &cl : m_clients) {
		if(rf(*cl) == false)
			continue;
		if(cl->SendPacket(protocol, packet, outErr) == false)
			success = false;
	}
	return success;
}
void pragma::networking::IServer::AddClient(const std::shared_ptr<IServerClient> &client)
{
	m_clients.push_back(client);
	if(m_eventInterface.onClientConnected)
		m_eventInterface.onClientConnected(*client);
}
bool pragma::networking::IServer::Start(Error &outErr, uint16_t port, bool useP2PIfAvailable)
{
	auto result = DoStart(outErr, port, useP2PIfAvailable);
	if(result)
		m_bRunning = true;
	return result;
}
bool pragma::networking::IServer::DropClient(const IServerClient &client, DropReason reason, Error &outErr)
{
	auto it = std::find_if(m_clients.begin(), m_clients.end(), [&client](const std::shared_ptr<IServerClient> &clientOther) { return clientOther.get() == &client; });
	if(it == m_clients.end())
		return true;
	if(m_eventInterface.onClientDropped)
		m_eventInterface.onClientDropped(**it, reason);
	auto cl = *it;
	m_clients.erase(it);
	return cl->Drop(reason, outErr);
}

void pragma::networking::IServer::SetEventInterface(const ServerEventInterface &eventHandler) { m_eventInterface = eventHandler; }

bool pragma::networking::IServer::IsRunning() const { return m_bRunning; }
const std::vector<std::shared_ptr<pragma::networking::IServerClient>> &pragma::networking::IServer::GetClients() const { return m_clients; }
const pragma::networking::ServerEventInterface &pragma::networking::IServer::GetEventInterface() const { return m_eventInterface; }
void pragma::networking::IServer::HandlePacket(IServerClient &client, NetPacket &packet)
{
	if(m_eventInterface.handlePacket)
		m_eventInterface.handlePacket(client, packet);
}
void pragma::networking::IServer::OnClientConnected(IServerClient &client)
{
	if(m_eventInterface.onClientConnected)
		m_eventInterface.onClientConnected(client);
}
void pragma::networking::IServer::OnClientDropped(IServerClient &client, DropReason reason)
{
	if(m_eventInterface.onClientDropped)
		m_eventInterface.onClientDropped(client, reason);
}
