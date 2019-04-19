#ifndef __C_SCI_GPU_TIMER_MANAGER_HPP__
#define __C_SCI_GPU_TIMER_MANAGER_HPP__

#include "pragma/c_enginedefinitions.h"
#include <queries/prosper_pipeline_statistics_query.hpp>
#include <pragma/debug/debug_performance_profiler.hpp>
#include <array>
#include <set>
/*
enum class GPUProfilingStage : uint32_t
{
	// Draw
	GUI = 0,
	Frame,
	Scene,

	Prepass,
	PrepassSkybox,
	PrepassWorld,
	PrepassView,

	CullLightSources,
	Shadows,
	SSAO,

	PostProcessing,
	PostProcessingFog,
	PostProcessingFXAA,
	PostProcessingGlow,
	PostProcessingBloom,
	PostProcessingHDR,

	Present,
	Skybox,
	World,
	Particles,
	Debug,
	Water,
	View,

	Count
};
*/
namespace prosper
{
	class QueryPool;
	class TimerQuery;
};
namespace pragma
{
	namespace debug
	{
		class GPUSwapchainTimer;
		struct DLLCENGINE GPUProfilerResult
			: public pragma::debug::ProfilerResult
		{
			std::optional<prosper::PipelineStatisticsQuery::Statistics> statistics;
		};

		class GPUProfiler;
		class DLLCENGINE GPUProfilingStage
			: public ProfilingStage
		{
		public:
			static std::shared_ptr<GPUProfilingStage> Create(Profiler &profiler,const std::string &name,Anvil::PipelineStageFlagBits stage,GPUProfilingStage *parent=nullptr);
			const GPUSwapchainTimer &GetTimer() const;
			GPUSwapchainTimer &GetTimer();

			Anvil::PipelineStageFlagBits GetPipelineStage() const;
		private:
			using ProfilingStage::ProfilingStage;
			virtual void InitializeTimer() override;
			GPUProfiler &GetProfiler();
			GPUProfilingStage *GetParent();
			Anvil::PipelineStageFlagBits m_stage;
		};

		class DLLCENGINE GPUProfiler
			: public Profiler
		{
		public:
			virtual void Initialize() override;
			void Reset();
			std::shared_ptr<Timer> CreateTimer(Anvil::PipelineStageFlagBits stage);
		private:
			GPUProfiler();
			std::shared_ptr<prosper::QueryPool> m_timerQueryPool = nullptr;
			std::shared_ptr<prosper::QueryPool> m_statsQueryPool = nullptr;

			void InitializeQueries();
			template<class TProfiler>
				friend std::shared_ptr<TProfiler> Profiler::Create();
		};
	};
};

#endif
