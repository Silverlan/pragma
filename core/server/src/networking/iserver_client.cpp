#include "stdafx_server.h"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include <pragma/networking/resource.h>

#pragma optimize("",off)
pragma::networking::IServerClient::~IServerClient() {ClearResourceTransfer();}
bool pragma::networking::IServerClient::Drop(DropReason reason,pragma::networking::Error &outErr)
{
	return false;
}
std::optional<pragma::networking::IPAddress> pragma::networking::IServerClient::GetIPAddress() const
{
	auto ip = GetIP();
	auto port = GetPort();
	if(ip.has_value() == false || port.has_value() == false)
		return {};
	return pragma::networking::IPAddress{*ip,*port};
}
pragma::SPlayerComponent *pragma::networking::IServerClient::GetPlayer() const {return m_player.get();}
void pragma::networking::IServerClient::SetTransferComplete(bool b) {m_bTransferring = !b;}
bool pragma::networking::IServerClient::IsTransferring() const {return m_bTransferring;}
bool pragma::networking::IServerClient::IsInitialResourceTransferComplete() const {return (m_initialResourceTransferState == TransferState::Complete) ? true : false;}
void pragma::networking::IServerClient::SetInitialResourceTransferState(TransferState state) {m_initialResourceTransferState = state;}
pragma::networking::IServerClient::TransferState pragma::networking::IServerClient::GetInitialResourceTransferState() {return m_initialResourceTransferState;}
void pragma::networking::IServerClient::SetPlayer(pragma::SPlayerComponent &pl)
{
	m_player = pl.GetHandle<pragma::SPlayerComponent>();
}
const std::vector<std::shared_ptr<Resource>> &pragma::networking::IServerClient::GetResourceTransfer() const {return m_resourceTransfer;}
bool pragma::networking::IServerClient::AddResource(const std::string &fileName,bool stream)
{
	auto canonName = FileManager::GetCanonicalizedPath(fileName);
	auto it = std::find_if(m_resourceTransfer.begin(),m_resourceTransfer.end(),[canonName](const std::shared_ptr<Resource> &res) {
		return (res->name == canonName) ? true : false;
		});
	if(it != m_resourceTransfer.end())
	{
		if(stream == false)
			return true;
		(*it)->stream = stream;
		return true;
	}
	auto res = std::make_shared<Resource>(canonName,stream);
	if(res->Construct() == false)
		return false;
	it = m_resourceTransfer.end();
	if(stream == false)
	{
		// Insert new resource before streamed resources
		it = m_resourceTransfer.begin();
		for(auto itRes=m_resourceTransfer.begin();itRes!=m_resourceTransfer.end();++itRes)
		{
			if((*itRes)->stream == stream)
				it = itRes +1;
			else
				break;
		}
	}
	m_resourceTransfer.insert(it,res);
	return true;
}
void pragma::networking::IServerClient::RemoveResource(uint32_t i)
{
	m_resourceTransfer.erase(m_resourceTransfer.begin() +i);
}
void pragma::networking::IServerClient::ClearResourceTransfer()
{
	m_resourceTransfer.clear();
}

uint8_t pragma::networking::IServerClient::SwapSnapshotId()
{
	return m_snapshotId++; // Overflow doesn't matter
}

void pragma::networking::IServerClient::ScheduleResource(const std::string &fileName)
{
	auto it = std::find(m_scheduledResources.begin(),m_scheduledResources.end(),fileName);
	if(it != m_scheduledResources.end())
		return;
	if(m_scheduledResources.size() == m_scheduledResources.capacity())
		m_scheduledResources.reserve(m_scheduledResources.size() +20);
	m_scheduledResources.push_back(fileName);
}

std::vector<std::string> &pragma::networking::IServerClient::GetScheduledResources() {return m_scheduledResources;}

void pragma::networking::IServerClient::SetSteamId(uint64_t steamId) {m_steamId = steamId;}
uint64_t pragma::networking::IServerClient::GetSteamId() const {return m_steamId;}
#pragma optimize("",on)
