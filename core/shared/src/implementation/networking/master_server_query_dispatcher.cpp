// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :networking.master_server_query_dispatcher;

void pragma::networking::IMasterServerQueryDispatcher::Release() { CancelQuery(); }
void pragma::networking::IMasterServerQueryDispatcher::SetEventCallbacks(const EventCallbacks &eventCallbacks) { m_eventCallbacks = eventCallbacks; }
void pragma::networking::IMasterServerQueryDispatcher::CancelQuery()
{
	m_numServersPinged = 0;
	m_queryResults.clear();
	m_serverPingQueue = {};
	DoCancelQuery();
}
void pragma::networking::IMasterServerQueryDispatcher::PingServer(uint32_t serverIdx)
{
	++m_numServersPinged;
	DoPingServer(serverIdx);
}
void pragma::networking::IMasterServerQueryDispatcher::QueryServers(const Filter &filter)
{
	CancelQuery();
	DoQueryServers(filter);
}
void pragma::networking::IMasterServerQueryDispatcher::Poll()
{
	DispatchPingBatch();
	DoPoll();
}
void pragma::networking::IMasterServerQueryDispatcher::AddQueryResult(MasterServerQueryResult &&queryResult)
{
	if(m_queryResults.size() == m_queryResults.capacity())
		m_queryResults.reserve(m_queryResults.size() + 50);
	m_queryResults.emplace_back(std::move(queryResult));
}
void pragma::networking::IMasterServerQueryDispatcher::OnServerPingResponse(uint32_t serverIdx, bool pingSuccessful)
{
	--m_numServersPinged;
	if(m_eventCallbacks.onServerPingResponse)
		m_eventCallbacks.onServerPingResponse(*GetQueryResult(serverIdx), pingSuccessful);
	if(pingSuccessful == false)
		m_queryResults.at(serverIdx).reset();

	if(m_numServersPinged == 0u)
		OnRefreshComplete();
}
void pragma::networking::IMasterServerQueryDispatcher::OnRefreshComplete()
{
	if(m_eventCallbacks.onRefreshComplete == nullptr)
		return;
	auto numServers = std::count_if(m_queryResults.begin(), m_queryResults.end(), [](const std::optional<MasterServerQueryResult> &result) { return result.has_value(); });
	m_eventCallbacks.onRefreshComplete(numServers);
}
void pragma::networking::IMasterServerQueryDispatcher::OnQueryResponse(bool successful)
{
	if(m_eventCallbacks.onQueryResponse)
		m_eventCallbacks.onQueryResponse(successful, m_queryResults.size());
}
pragma::networking::MasterServerQueryResult *pragma::networking::IMasterServerQueryDispatcher::GetQueryResult(uint32_t idx)
{
	auto &queryResult = m_queryResults.at(idx);
	return queryResult.has_value() ? &*queryResult : nullptr;
}
void pragma::networking::IMasterServerQueryDispatcher::DispatchPingBatch()
{
	if(m_numServersPinged >= m_batchCount)
		return;
	auto numServersToPing = m_batchCount - m_numServersPinged;
	while(m_serverPingQueue.empty() == false && numServersToPing-- > 0) {
		auto svIdx = m_serverPingQueue.front();
		m_serverPingQueue.pop();
		auto &sv = m_queryResults.at(svIdx);
		PingServer(svIdx);
	}
}
void pragma::networking::IMasterServerQueryDispatcher::PingServers()
{
	for(auto i = decltype(m_queryResults.size()) {0u}; i < m_queryResults.size(); ++i)
		m_serverPingQueue.push(i);
}
