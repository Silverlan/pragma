#include "stdafx_server.h"
#include "pragma/networking/iserver.hpp"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/networking/error.hpp>

bool pragma::networking::IServer::Shutdown(Error &outErr)
{
	for(auto &cl : m_clients)
		cl->Drop(DropReason::Shutdown,outErr);
	auto result = DoShutdown(outErr);
	m_bRunning = false;
	return result;
}
bool pragma::networking::IServer::SendPacket(Protocol protocol,NetPacket &packet,const ClientRecipientFilter &rf,Error &outErr)
{
	auto success = true;
	for(auto &cl : m_clients)
	{
		if(rf(*cl) == false)
			continue;
		if(cl->SendPacket(protocol,packet,outErr) == false)
			success = false;
	}
	return success;
}
void pragma::networking::IServer::AddClient(const std::shared_ptr<IServerClient> &client)
{
	m_clients.push_back(client);
}
bool pragma::networking::IServer::DropClient(const IServerClient &client,pragma::networking::DropReason reason,Error &outErr)
{
	auto it = std::find_if(m_clients.begin(),m_clients.end(),[&client](const std::shared_ptr<IServerClient> &clientOther) {
		return clientOther.get() == &client;
	});
	if(it == m_clients.end())
		return true;
	if(m_eventInterface.onClientDropped)
		m_eventInterface.onClientDropped(**it,reason);
	auto cl = *it;
	m_clients.erase(it);
	return cl->Drop(reason,outErr);
}

void pragma::networking::IServer::SetEventInterface(const ServerEventInterface &eventHandler) {m_eventInterface = eventHandler;}

bool pragma::networking::IServer::IsRunning() const {return m_bRunning;}
const std::vector<std::shared_ptr<pragma::networking::IServerClient>> &pragma::networking::IServer::GetClients() const {return m_clients;}
const pragma::networking::ServerEventInterface &pragma::networking::IServer::GetEventInterface() const {return m_eventInterface;}
void pragma::networking::IServer::HandlePacket(IServerClient &client,NetPacket &packet)
{
	if(m_eventInterface.handlePacket)
		m_eventInterface.handlePacket(client,packet);
}
