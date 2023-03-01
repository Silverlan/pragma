/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_GPU_SWAPCHAIN_TIMER_HPP__
#define __C_GPU_SWAPCHAIN_TIMER_HPP__

#include "pragma/c_enginedefinitions.h"
#include <pragma/debug/debug_performance_profiler.hpp>
#include <queries/prosper_timer_query.hpp>
#include <queries/prosper_pipeline_statistics_query.hpp>
#include <chrono>

namespace prosper {
	class IQueryPool;
};
namespace pragma {
	namespace debug {
		class DLLCLIENT GPUSwapchainTimer : public Timer {
		  public:
			static std::shared_ptr<GPUSwapchainTimer> Create(prosper::IQueryPool &timerQueryPool, prosper::IQueryPool &statsQueryPool, prosper::PipelineStageFlags stage);

			virtual bool Start() override;
			virtual bool Stop() override;
			bool Reset();
			virtual std::unique_ptr<ProfilerResult> GetResult() const override;
		  private:
			GPUSwapchainTimer(prosper::IQueryPool &timerQueryPool, prosper::IQueryPool &statsQueryPool, prosper::PipelineStageFlags stage);
			void UpdateResult();
			prosper::TimerQuery *GetTimerQuery();
			prosper::PipelineStatisticsQuery *GetStatisticsQuery();
			void InitializeQueries();
			struct SwapchainQuery {
				std::shared_ptr<prosper::TimerQuery> timerQuery;
				std::shared_ptr<prosper::PipelineStatisticsQuery> statsQuery;
			};

			std::vector<SwapchainQuery> m_swapchainTimers = {};
			prosper::PipelineStageFlags m_stage;
			bool m_bHasStartedAtLeastOnce = false;
			std::weak_ptr<prosper::IQueryPool> m_wpTimerQueryPool = {};
			std::weak_ptr<prosper::IQueryPool> m_wpStatsQueryPool = {};
			std::optional<std::chrono::nanoseconds> m_lastTimeResult = {};
			std::optional<prosper::PipelineStatistics> m_lastStatsResult = {};
		};
	};
};

#endif
