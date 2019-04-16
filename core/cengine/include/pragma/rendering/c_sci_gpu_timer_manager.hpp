#ifndef __C_SCI_GPU_TIMER_MANAGER_HPP__
#define __C_SCI_GPU_TIMER_MANAGER_HPP__

#include "pragma/c_enginedefinitions.h"
#include <queries/prosper_pipeline_statistics_query.hpp>
#include <array>
#include <set>

enum class GPUTimerEvent : uint32_t
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

namespace prosper
{
	class QueryPool;
	class TimerQuery;
};
namespace pragma
{
	class GPUSwapchainTimer;
};
class DLLCENGINE CSciGPUTimerManager
{
public:
	enum class EventFlags : uint32_t
	{
		None = 0u,
		Root
	};
	struct DLLCENGINE EventData
	{
		Anvil::PipelineStageFlagBits stage;
	};
	struct DLLCENGINE ResultData
	{
		std::chrono::nanoseconds duration;
		prosper::PipelineStatisticsQuery::Statistics statistics;
	};

	CSciGPUTimerManager();
	void StartTimer(GPUTimerEvent e);
	void StopTimer(GPUTimerEvent e);
	bool GetResult(GPUTimerEvent e,std::chrono::nanoseconds &outDuration) const;
	void Reset();

	std::vector<std::optional<ResultData>> GetResults() const;
	const std::unordered_map<GPUTimerEvent,std::vector<GPUTimerEvent>> &GetEventDependencies() const;
	const std::vector<GPUTimerEvent> &GetRootEvents() const;
	const EventData &GetEventData(GPUTimerEvent eventId) const;
	static std::string EventToString(GPUTimerEvent e);
private:
	std::shared_ptr<prosper::QueryPool> m_timerQueryPool = nullptr;
	std::shared_ptr<prosper::QueryPool> m_statsQueryPool = nullptr;
	std::unordered_map<GPUTimerEvent,std::vector<GPUTimerEvent>> m_eventDependencies = {};
	std::vector<GPUTimerEvent> m_rootEvents = {};
	std::array<EventData,umath::to_integral(GPUTimerEvent::Count)> m_eventData = {};
	std::array<std::shared_ptr<pragma::GPUSwapchainTimer>,umath::to_integral(GPUTimerEvent::Count)> m_timerQueries;

	void SetupEvent(GPUTimerEvent eventId,const std::vector<GPUTimerEvent> &dependencies,Anvil::PipelineStageFlagBits stage,EventFlags eventFlags=EventFlags::None);
	void InitializeQueries();
};
REGISTER_BASIC_BITWISE_OPERATORS(CSciGPUTimerManager::EventFlags)

#endif
