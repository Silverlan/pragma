#include "stdafx_server.h"
#include "pragma/networking/wvserverclient.h"

WVServerClient::WVServerClient(nwm::Server *manager)
	: nwm::ServerClient(manager)
{
	Reset();
}

WVServerClient::~WVServerClient()
{}

void WVServerClient::Reset()
{
	m_sessionInfo = std::make_unique<ClientSessionInfo>();
}

void WVServerClient::OnClosed() {}

ClientSessionInfo *WVServerClient::GetSessionInfo() {return m_sessionInfo.get();}
void WVServerClient::ClearSessionInfo()
{
	if(m_sessionInfo == nullptr)
		return;
	m_sessionInfo = nullptr;
}

uint8_t WVServerClient::SwapSnapshotId()
{
	return m_snapshotId++; // Overflow doesn't matter
}

void WVServerClient::ScheduleResource(const std::string &fileName)
{
	auto it = std::find(m_scheduledResources.begin(),m_scheduledResources.end(),fileName);
	if(it != m_scheduledResources.end())
		return;
	if(m_scheduledResources.size() == m_scheduledResources.capacity())
		m_scheduledResources.reserve(m_scheduledResources.size() +20);
	m_scheduledResources.push_back(fileName);
}

std::vector<std::string> &WVServerClient::GetScheduledResources() {return m_scheduledResources;}
