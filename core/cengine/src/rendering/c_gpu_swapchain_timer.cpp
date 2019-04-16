#include "stdafx_cengine.h"
#include "pragma/rendering/c_gpu_swapchain_timer.hpp"
#include <queries/prosper_query_pool.hpp>
#include <queries/prosper_pipeline_statistics_query.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

std::shared_ptr<GPUSwapchainTimer> GPUSwapchainTimer::Create(prosper::QueryPool &timerQueryPool,prosper::QueryPool &statsQueryPool,Anvil::PipelineStageFlagBits stage)
{
	return std::shared_ptr<GPUSwapchainTimer>(new GPUSwapchainTimer{timerQueryPool,statsQueryPool,stage});
}
GPUSwapchainTimer::GPUSwapchainTimer(prosper::QueryPool &timerQueryPool,prosper::QueryPool &statsQueryPool,Anvil::PipelineStageFlagBits stage)
	: m_stage{stage},m_wpTimerQueryPool{timerQueryPool.shared_from_this()},m_wpStatsQueryPool{statsQueryPool.shared_from_this()}
{}
bool GPUSwapchainTimer::GetResult(std::chrono::nanoseconds &outDuration) const
{
	if(m_lastTimeResult.has_value() == false)
		return false;
	outDuration = *m_lastTimeResult;
	return true;
}
bool GPUSwapchainTimer::GetStatisticsResult(prosper::PipelineStatisticsQuery::Statistics &outStatistics) const
{
	if(m_lastStatsResult.has_value() == false)
		return false;
	outStatistics = *m_lastStatsResult;
	return true;
}
void GPUSwapchainTimer::UpdateResult()
{
	auto *pTimerQuery = GetTimerQuery();
	auto *pStatsQuery = GetStatisticsQuery();
	std::chrono::nanoseconds result;
	prosper::PipelineStatisticsQuery::Statistics stats;
	if(pTimerQuery == nullptr || pStatsQuery == nullptr || pTimerQuery->QueryResult(result) == false || pStatsQuery->QueryResult(stats) == false)
		return;
	m_lastTimeResult = result;
	m_lastStatsResult = stats;
}
bool GPUSwapchainTimer::Reset()
{
	auto *pTimerQuery = GetTimerQuery();
	auto *pStatsQuery = GetStatisticsQuery();
	if(pTimerQuery == nullptr || pStatsQuery == nullptr)
		return false;
	auto &drawCmd = c_engine->GetDrawCommandBuffer();
	return pTimerQuery->Reset(**drawCmd) && pStatsQuery->Reset(**drawCmd);
}
bool GPUSwapchainTimer::Begin()
{
	if(m_bHasStartedAtLeastOnce)
		UpdateResult();
	auto *pTimerQuery = GetTimerQuery();
	auto *pStatsQuery = GetStatisticsQuery();
	if(pTimerQuery == nullptr || pStatsQuery == nullptr)
		return false;
	auto &drawCmd = c_engine->GetDrawCommandBuffer();
	auto r = pTimerQuery->Begin(**drawCmd) && pStatsQuery->RecordBegin(**drawCmd);
	if(r == true)
		m_bHasStartedAtLeastOnce = true;
	return r;
}
bool GPUSwapchainTimer::End()
{
	auto *pTimerQuery = GetTimerQuery();
	auto *pStatsQuery = GetStatisticsQuery();
	if(pTimerQuery == nullptr || pStatsQuery == nullptr)
		return false;
	auto &drawCmd = c_engine->GetDrawCommandBuffer();
	return pTimerQuery->End(**drawCmd) && pStatsQuery->RecordEnd(**drawCmd);;
}

prosper::TimerQuery *GPUSwapchainTimer::GetTimerQuery()
{
	InitializeQueries();
	auto index = c_engine->GetSwapchain()->get_last_acquired_image_index();
	if(index >= m_swapchainTimers.size())
		return nullptr;
	return m_swapchainTimers.at(index).timerQuery.get();
}

prosper::PipelineStatisticsQuery *GPUSwapchainTimer::GetStatisticsQuery()
{
	InitializeQueries();
	auto index = c_engine->GetSwapchain()->get_last_acquired_image_index();
	if(index >= m_swapchainTimers.size())
		return nullptr;
	return m_swapchainTimers.at(index).statsQuery.get();
}

void GPUSwapchainTimer::InitializeQueries()
{
	auto index = c_engine->GetSwapchain()->get_last_acquired_image_index();
	if(index < m_swapchainTimers.size())
		return;
	auto timerPool = m_wpTimerQueryPool.lock();
	auto statsPool = m_wpStatsQueryPool.lock();
	m_swapchainTimers.reserve(index +1);
	for(auto i=decltype(m_swapchainTimers.size()){0u};i<=index;++i)
	{
		auto timerQuery = prosper::util::create_timer_query(*timerPool,m_stage);
		auto statsQuery = prosper::util::create_pipeline_statistics_query(*statsPool);
		m_swapchainTimers.push_back({timerQuery,statsQuery});
	}
}
