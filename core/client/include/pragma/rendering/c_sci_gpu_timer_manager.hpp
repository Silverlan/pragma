/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SCI_GPU_TIMER_MANAGER_HPP__
#define __C_SCI_GPU_TIMER_MANAGER_HPP__

#include "pragma/c_enginedefinitions.h"
#include <queries/prosper_pipeline_statistics_query.hpp>
#include <pragma/debug/debug_performance_profiler.hpp>
#include <array>
#include <set>

namespace prosper {
	class IQueryPool;
	class TimerQuery;
};
namespace pragma {
	namespace debug {
		class GPUSwapchainTimer;
		struct DLLCLIENT GPUProfilerResult : public pragma::debug::ProfilerResult {
			std::optional<prosper::PipelineStatistics> statistics;
		};

		class GPUProfiler;
		class DLLCLIENT GPUProfilingStage : public ProfilingStage {
		  public:
			static std::shared_ptr<GPUProfilingStage> Create(Profiler &profiler, std::thread::id tid, const std::string &name, prosper::PipelineStageFlags stage);
			GPUProfilingStage(Profiler &profiler, std::thread::id tid, const std::string &name);
			const GPUSwapchainTimer &GetTimer() const;
			GPUSwapchainTimer &GetTimer();
			GPUProfilingStage *GetParent();

			prosper::PipelineStageFlags GetPipelineStage() const;
		  private:
			virtual void InitializeTimer() override;
			GPUProfiler &GetProfiler();
			prosper::PipelineStageFlags m_stage;
		};

		class DLLCLIENT GPUProfiler : public Profiler {
		  public:
			virtual void Initialize() override;
			void Reset();
			std::shared_ptr<Timer> CreateTimer(prosper::PipelineStageFlags stage);
		  private:
			GPUProfiler();
			std::shared_ptr<prosper::IQueryPool> m_timerQueryPool = nullptr;
			std::shared_ptr<prosper::IQueryPool> m_statsQueryPool = nullptr;

			void InitializeQueries();
			template<class TProfiler>
			friend std::shared_ptr<TProfiler> Profiler::Create();
		};
	};
};

#endif
