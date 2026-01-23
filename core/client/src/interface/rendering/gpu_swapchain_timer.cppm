// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.gpu_swapchain_timer;

export import pragma.prosper;
export import pragma.shared;

export namespace pragma {
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
