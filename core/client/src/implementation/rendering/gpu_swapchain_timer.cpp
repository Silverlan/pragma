// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.gpu_profiler;
import :engine;

using namespace pragma::debug;

std::shared_ptr<GPUSwapchainTimer> GPUSwapchainTimer::Create(prosper::IQueryPool &timerQueryPool, prosper::IQueryPool &statsQueryPool, prosper::PipelineStageFlags stage) { return std::shared_ptr<GPUSwapchainTimer>(new GPUSwapchainTimer {timerQueryPool, statsQueryPool, stage}); }
GPUSwapchainTimer::GPUSwapchainTimer(prosper::IQueryPool &timerQueryPool, prosper::IQueryPool &statsQueryPool, prosper::PipelineStageFlags stage) : m_stage {stage}, m_wpTimerQueryPool {timerQueryPool.shared_from_this()}, m_wpStatsQueryPool {statsQueryPool.shared_from_this()} {}
void GPUSwapchainTimer::UpdateResult()
{
	auto *pTimerQuery = GetTimerQuery();
	auto *pStatsQuery = GetStatisticsQuery();
	std::chrono::nanoseconds result;
	prosper::PipelineStatistics stats;
	if(pTimerQuery == nullptr || pStatsQuery == nullptr || pTimerQuery->QueryResult(result) == false || pStatsQuery->QueryResult(stats) == false)
		return;
	m_lastTimeResult = result;
	m_lastStatsResult = stats;
}
bool GPUSwapchainTimer::Start()
{
	if(m_bHasStartedAtLeastOnce)
		UpdateResult();
	auto *pTimerQuery = GetTimerQuery();
	auto *pStatsQuery = GetStatisticsQuery();
	if(pTimerQuery == nullptr || pStatsQuery == nullptr)
		return false;
	auto &drawCmd = get_cengine()->GetDrawCommandBuffer();
	auto r = pTimerQuery->Begin(*drawCmd) && pStatsQuery->RecordBegin(*drawCmd);
	if(r == true)
		m_bHasStartedAtLeastOnce = true;
	return r;
}
bool GPUSwapchainTimer::Stop()
{
	auto *pTimerQuery = GetTimerQuery();
	auto *pStatsQuery = GetStatisticsQuery();
	if(pTimerQuery == nullptr || pStatsQuery == nullptr)
		return false;
	auto &drawCmd = get_cengine()->GetDrawCommandBuffer();
	return pTimerQuery->End(*drawCmd) && pStatsQuery->RecordEnd(*drawCmd);
}
bool GPUSwapchainTimer::Reset()
{
	auto *pTimerQuery = GetTimerQuery();
	auto *pStatsQuery = GetStatisticsQuery();
	if(pTimerQuery == nullptr || pStatsQuery == nullptr)
		return false;
	auto &drawCmd = get_cengine()->GetDrawCommandBuffer();
	return pTimerQuery->Reset(*drawCmd) && pStatsQuery->Reset(*drawCmd);
}
std::unique_ptr<ProfilerResult> GPUSwapchainTimer::GetResult() const
{
	auto result = std::make_unique<GPUProfilerResult>();
	result->duration = m_lastTimeResult.has_value() ? *m_lastTimeResult : std::optional<std::chrono::nanoseconds> {};
	result->statistics = m_lastStatsResult.has_value() ? *m_lastStatsResult : std::optional<prosper::PipelineStatistics> {};
	return result;
}

prosper::TimerQuery *GPUSwapchainTimer::GetTimerQuery()
{
	InitializeQueries();
	auto index = get_cengine()->GetRenderContext().GetLastAcquiredPrimaryWindowSwapchainImageIndex();
	if(index >= m_swapchainTimers.size())
		return nullptr;
	return m_swapchainTimers.at(index).timerQuery.get();
}

prosper::PipelineStatisticsQuery *GPUSwapchainTimer::GetStatisticsQuery()
{
	InitializeQueries();
	auto index = get_cengine()->GetRenderContext().GetLastAcquiredPrimaryWindowSwapchainImageIndex();
	if(index >= m_swapchainTimers.size())
		return nullptr;
	return m_swapchainTimers.at(index).statsQuery.get();
}

void GPUSwapchainTimer::InitializeQueries()
{
	auto index = get_cengine()->GetRenderContext().GetLastAcquiredPrimaryWindowSwapchainImageIndex();
	if(index < m_swapchainTimers.size())
		return;
	auto timerPool = m_wpTimerQueryPool.lock();
	auto statsPool = m_wpStatsQueryPool.lock();
	m_swapchainTimers.reserve(index + 1);
	for(auto i = decltype(m_swapchainTimers.size()) {0u}; i <= index; ++i) {
		// TODO: Using the same stage for both time queries makes no sense
		auto timerQuery = timerPool->CreateTimerQuery(static_cast<prosper::PipelineStageFlags>(m_stage), static_cast<prosper::PipelineStageFlags>(m_stage));
		auto statsQuery = statsPool->CreatePipelineStatisticsQuery();
		m_swapchainTimers.push_back({timerQuery, statsQuery});
	}
}
