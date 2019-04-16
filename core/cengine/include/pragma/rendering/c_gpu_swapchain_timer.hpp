#ifndef __C_GPU_SWAPCHAIN_TIMER_HPP__
#define __C_GPU_SWAPCHAIN_TIMER_HPP__

#include "pragma/c_enginedefinitions.h"
#include <queries/prosper_timer_query.hpp>
#include <queries/prosper_pipeline_statistics_query.hpp>
#include <chrono>

namespace prosper {class QueryPool;};
namespace pragma
{
	class DLLCENGINE GPUSwapchainTimer
	{
	public:
		static std::shared_ptr<GPUSwapchainTimer> Create(prosper::QueryPool &timerQueryPool,prosper::QueryPool &statsQueryPool,Anvil::PipelineStageFlagBits stage);
		GPUSwapchainTimer(const GPUSwapchainTimer&)=delete;
		GPUSwapchainTimer &operator=(const GPUSwapchainTimer&)=delete;

		bool GetResult(std::chrono::nanoseconds &outDuration) const;
		bool GetStatisticsResult(prosper::PipelineStatisticsQuery::Statistics &outStatistics) const;
		bool Reset();
		bool Begin();
		bool End();
	private:
		GPUSwapchainTimer(prosper::QueryPool &timerQueryPool,prosper::QueryPool &statsQueryPool,Anvil::PipelineStageFlagBits stage);
		void UpdateResult();
		prosper::TimerQuery *GetTimerQuery();
		prosper::PipelineStatisticsQuery *GetStatisticsQuery();
		void InitializeQueries();
		struct SwapchainQuery
		{
			std::shared_ptr<prosper::TimerQuery> timerQuery;
			std::shared_ptr<prosper::PipelineStatisticsQuery> statsQuery;
		};

		std::vector<SwapchainQuery> m_swapchainTimers = {};
		Anvil::PipelineStageFlagBits m_stage;
		bool m_bHasStartedAtLeastOnce = false;
		std::weak_ptr<prosper::QueryPool> m_wpTimerQueryPool = {};
		std::weak_ptr<prosper::QueryPool> m_wpStatsQueryPool = {};
		std::optional<std::chrono::nanoseconds> m_lastTimeResult = {};
		std::optional<prosper::PipelineStatisticsQuery::Statistics> m_lastStatsResult = {};
	};
};

#endif
