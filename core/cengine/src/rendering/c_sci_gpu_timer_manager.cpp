#include "stdafx_cengine.h"
#include "pragma/rendering/c_sci_gpu_timer_manager.hpp"
#include "pragma/rendering/c_gpu_swapchain_timer.hpp"
#include <pragma/console/c_cvar.h>
#include <pragma/console/convars.h>
#include <pragma/console/c_cvar_global_functions.h>
#include <queries/prosper_query_pool.hpp>
#include <queries/prosper_timer_query.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_util.hpp>

extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
static CVar cvTimerQueries = GetClientConVar("cl_gpu_timer_queries_enabled");
CSciGPUTimerManager::CSciGPUTimerManager()
	: m_timerQueryPool{nullptr},m_statsQueryPool{nullptr}
{
	InitializeQueries();

	// Initialize events
	SetupEvent(GPUTimerEvent::GUI,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::Frame,{
		GPUTimerEvent::GUI,
		GPUTimerEvent::Scene,
		GPUTimerEvent::PostProcessing,
		GPUTimerEvent::Present
	},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT,EventFlags::Root);
	SetupEvent(GPUTimerEvent::Scene,{
		GPUTimerEvent::Prepass,
		GPUTimerEvent::CullLightSources,
		GPUTimerEvent::Shadows,
		GPUTimerEvent::SSAO,
		
		GPUTimerEvent::Skybox,
		GPUTimerEvent::World,
		GPUTimerEvent::Particles,
		GPUTimerEvent::Debug,
		GPUTimerEvent::Water,
		GPUTimerEvent::View
	},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::Prepass,{
		GPUTimerEvent::PrepassSkybox,
		GPUTimerEvent::PrepassWorld,
		GPUTimerEvent::PrepassView
	},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::PrepassSkybox,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::PrepassWorld,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::PrepassView,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::CullLightSources,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::Shadows,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::SSAO,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::PostProcessing,{
		GPUTimerEvent::PostProcessingFog,
		GPUTimerEvent::PostProcessingFXAA,
		GPUTimerEvent::PostProcessingGlow,
		GPUTimerEvent::PostProcessingBloom,
		GPUTimerEvent::PostProcessingHDR
	},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::PostProcessingFog,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::PostProcessingFXAA,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::PostProcessingGlow,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::PostProcessingBloom,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::PostProcessingHDR,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::Present,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::Skybox,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::World,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::Particles,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::Debug,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::Water,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	SetupEvent(GPUTimerEvent::View,{},Anvil::PipelineStageFlagBits::BOTTOM_OF_PIPE_BIT);
	static_assert(umath::to_integral(GPUTimerEvent::Count) == 23u,"Timer event has been added, but not set up!");
}
void CSciGPUTimerManager::SetupEvent(GPUTimerEvent eventId,const std::vector<GPUTimerEvent> &dependencies,Anvil::PipelineStageFlagBits stage,EventFlags eventFlags)
{
	if(dependencies.empty() == false)
		m_eventDependencies[eventId] = dependencies;
	m_eventData.at(umath::to_integral(eventId)) = {stage};
	if(umath::is_flag_set(eventFlags,EventFlags::Root))
		m_rootEvents.push_back(eventId);
}
void CSciGPUTimerManager::InitializeQueries()
{
	if(m_timerQueryPool != nullptr || m_statsQueryPool != nullptr)
		return;
	auto swapchainImageCount = c_engine->GetSwapchainImageCount();
	m_timerQueryPool = prosper::util::create_query_pool(*c_engine,vk::QueryType::eTimestamp,m_timerQueries.size() *swapchainImageCount *2); // 2 timestamps per timer
	m_statsQueryPool = prosper::util::create_query_pool(*c_engine,
		Anvil::QueryPipelineStatisticFlagBits::INPUT_ASSEMBLY_VERTICES_BIT |
		Anvil::QueryPipelineStatisticFlagBits::INPUT_ASSEMBLY_PRIMITIVES_BIT |
		Anvil::QueryPipelineStatisticFlagBits::VERTEX_SHADER_INVOCATIONS_BIT |
		Anvil::QueryPipelineStatisticFlagBits::GEOMETRY_SHADER_INVOCATIONS_BIT |
		Anvil::QueryPipelineStatisticFlagBits::GEOMETRY_SHADER_PRIMITIVES_BIT |
		Anvil::QueryPipelineStatisticFlagBits::CLIPPING_INVOCATIONS_BIT |
		Anvil::QueryPipelineStatisticFlagBits::CLIPPING_PRIMITIVES_BIT |
		Anvil::QueryPipelineStatisticFlagBits::FRAGMENT_SHADER_INVOCATIONS_BIT |
		Anvil::QueryPipelineStatisticFlagBits::TESSELLATION_CONTROL_SHADER_PATCHES_BIT |
		Anvil::QueryPipelineStatisticFlagBits::TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT |
		Anvil::QueryPipelineStatisticFlagBits::COMPUTE_SHADER_INVOCATIONS_BIT,
		m_timerQueries.size() *swapchainImageCount
	);
	if(m_timerQueryPool == nullptr || m_statsQueryPool == nullptr)
		return;
	for(auto i=decltype(m_timerQueries.size()){0u};i<m_timerQueries.size();++i)
		m_timerQueries.at(i) = pragma::GPUSwapchainTimer::Create(*m_timerQueryPool,*m_statsQueryPool,m_eventData.at(i).stage);
}
const std::unordered_map<GPUTimerEvent,std::vector<GPUTimerEvent>> &CSciGPUTimerManager::GetEventDependencies() const {return m_eventDependencies;}
const std::vector<GPUTimerEvent> &CSciGPUTimerManager::GetRootEvents() const {return m_rootEvents;}
void CSciGPUTimerManager::StartTimer(GPUTimerEvent e)
{
	if(cvTimerQueries->GetBool() == false || m_timerQueryPool == nullptr)
		return;
	auto idx = umath::to_integral(e);
	if(idx >= m_timerQueries.size())
		return;
	auto &query = m_timerQueries.at(idx);
	query->Begin();
}
void CSciGPUTimerManager::StopTimer(GPUTimerEvent e)
{
	if(cvTimerQueries->GetBool() == false || m_timerQueryPool == nullptr)
		return;
	auto idx = umath::to_integral(e);
	if(idx >= m_timerQueries.size())
		return;
	auto &query = m_timerQueries.at(idx);
	query->End();
}
void CSciGPUTimerManager::Reset()
{
	if(cvTimerQueries->GetBool() == false || m_timerQueryPool == nullptr)
		return;
	for(auto &query : m_timerQueries)
	{
		if(query == nullptr)
			continue;
		query->Reset();
	}
}
const CSciGPUTimerManager::EventData &CSciGPUTimerManager::GetEventData(GPUTimerEvent eventId) const {return m_eventData.at(umath::to_integral(eventId));}
bool CSciGPUTimerManager::GetResult(GPUTimerEvent e,std::chrono::nanoseconds &outDuration) const
{
	if(cvTimerQueries->GetBool() == false || m_timerQueryPool == nullptr)
		return false;
	auto idx = umath::to_integral(e);
	if(idx >= m_timerQueries.size())
		return false;
	auto &query = m_timerQueries.at(idx);
	return query->GetResult(outDuration);
}
std::vector<std::optional<CSciGPUTimerManager::ResultData>> CSciGPUTimerManager::GetResults() const
{
	std::vector<std::optional<ResultData>> results {};
	results.resize(m_timerQueries.size());
	for(auto i=decltype(m_timerQueries.size()){0u};i<m_timerQueries.size();++i)
	{
		auto &query = m_timerQueries.at(i);
		if(query == nullptr)
			continue;
		auto &resultData = results.at(i);
		auto isValid = true;
		resultData = ResultData{};
		std::chrono::nanoseconds result;
		if(query->GetResult(result))
			resultData->duration = result;
		else
			isValid = false;
		prosper::PipelineStatisticsQuery::Statistics statistics;
		if(query->GetStatisticsResult(statistics))
			resultData->statistics = statistics;
		else
			isValid = false;
		if(isValid == false)
			resultData = {};
	}
	return results;
}

std::string CSciGPUTimerManager::EventToString(GPUTimerEvent e)
{
	switch(e)
	{
		case GPUTimerEvent::GUI:
			return "GUI";
		case GPUTimerEvent::Frame:
			return "Frame";
		case GPUTimerEvent::Scene:
			return "Scene";
		case GPUTimerEvent::Prepass:
			return "Prepass";
		case GPUTimerEvent::PrepassSkybox:
			return "Skybox";
		case GPUTimerEvent::PrepassWorld:
			return "World";
		case GPUTimerEvent::CullLightSources:
			return "CullLightSources";
		case GPUTimerEvent::Shadows:
			return "Shadows";
		case GPUTimerEvent::SSAO:
			return "SSAO";
		case GPUTimerEvent::PostProcessing:
			return "PostProcessing";
		case GPUTimerEvent::PostProcessingFog:
			return "Fog";
		case GPUTimerEvent::PostProcessingFXAA:
			return "FXAA";
		case GPUTimerEvent::PostProcessingGlow:
			return "Glow";
		case GPUTimerEvent::PostProcessingBloom:
			return "Bloom";
		case GPUTimerEvent::PostProcessingHDR:
			return "HDR";
		case GPUTimerEvent::Present:
			return "Present";
		case GPUTimerEvent::Skybox:
			return "Skybox";
		case GPUTimerEvent::World:
			return "World";
		case GPUTimerEvent::Particles:
			return "Particles";
		case GPUTimerEvent::Debug:
			return "Debug";
		case GPUTimerEvent::Water:
			return "Water";
		case GPUTimerEvent::View:
			return "View";
		default:
			return "Invalid";
	}
	static_assert(umath::to_integral(GPUTimerEvent::Count) == 23u,"Timer event has been added, but not added to EventToString!");
}

/////////////////////////

void Console::commands::cl_gpu_timer_queries_dump(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	auto extended = false;
	if(argv.empty() == false)
		extended = util::to_boolean(argv.at(0));
	Con::cout<<"-------- GPU-Timer Query Results --------"<<Con::endl;
	auto &manager = c_engine->GetGPUTimerManager();
	auto results = manager.GetResults();
	struct ResultData
	{
		GPUTimerEvent eventId;
		std::optional<CSciGPUTimerManager::ResultData> data;
		std::vector<ResultData> children;
	};
	std::function<void(ResultData&,const std::vector<GPUTimerEvent>&)> fPopulateResults = nullptr;
	fPopulateResults = [&fPopulateResults,&results,&manager](ResultData &resultData,const std::vector<GPUTimerEvent> &eventIds) {
		resultData.children.resize(eventIds.size());
		for(auto i=decltype(resultData.children.size()){0u};i<resultData.children.size();++i)
		{
			auto &childResultData = resultData.children.at(i);
			auto childEventId = eventIds.at(i);
			childResultData.data = results.at(umath::to_integral(childEventId));
			childResultData.eventId = childEventId;

			auto &eventDependencies = manager.GetEventDependencies();
			auto itDependencies = eventDependencies.find(childEventId);
			if(itDependencies == eventDependencies.end())
				continue;
			fPopulateResults(childResultData,itDependencies->second);
		}
		std::sort(resultData.children.begin(),resultData.children.end(),[](const ResultData &a,const ResultData &b) {
			auto aValue = a.data.has_value() ? a.data->duration : std::chrono::nanoseconds::min();
			auto bValue = b.data.has_value() ? b.data->duration : std::chrono::nanoseconds::min();
			return aValue > bValue;
		});
	};
	ResultData rootResult {};
	fPopulateResults(rootResult,manager.GetRootEvents());

	std::function<void(const ResultData&,const std::string&,bool)> fPrintResults = nullptr;
	fPrintResults = [&fPrintResults,&manager,extended](const ResultData &resultData,const std::string &t,bool bRoot) {
		if(bRoot == false)
		{
			auto &result = resultData.data;
			std::string sTime = "Pending";
			if(result.has_value())
			{
				auto t = result->duration.count() /static_cast<long double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1)).count());
				sTime = util::round_string(t,2) +" ms";
				sTime += " (" +std::to_string(result->duration.count()) +" ns)";
			}
			Con::cout<<t<<CSciGPUTimerManager::EventToString(resultData.eventId)<<": "<<sTime;
			
			auto &eventData = manager.GetEventData(resultData.eventId);
			Con::cout<<" ("<<prosper::util::to_string(eventData.stage)<<")"<<Con::endl;

			if(result.has_value() && extended == true)
			{
				auto &stats = result->statistics;
				Con::cout<<t<<"Input Assembly Vertices: "<<stats.inputAssemblyVertices<<Con::endl;
				Con::cout<<t<<"Input Assembly Primitives: "<<stats.inputAssemblyPrimitives<<Con::endl;
				Con::cout<<t<<"Vertex Shader Invocations: "<<stats.vertexShaderInvocations<<Con::endl;
				Con::cout<<t<<"Geometry Shader Invocations: "<<stats.geometryShaderInvocations<<Con::endl;
				Con::cout<<t<<"Geometry Shader Primitives: "<<stats.geometryShaderPrimitives<<Con::endl;
				Con::cout<<t<<"Clipping Invocations: "<<stats.clippingInvocations<<Con::endl;
				Con::cout<<t<<"Clipping Primitives: "<<stats.clippingPrimitives<<Con::endl;
				Con::cout<<t<<"Fragment Shader Invocations: "<<stats.fragmentShaderInvocations<<Con::endl;
				Con::cout<<t<<"Tessellation Control Shader Patches: "<<stats.tessellationControlShaderPatches<<Con::endl;
				Con::cout<<t<<"Tessellation Evaluation Shader Invocations: "<<stats.tessellationEvaluationShaderInvocations<<Con::endl;
				Con::cout<<t<<"Compute Shader Invocations: "<<stats.computeShaderInvocations<<Con::endl;
				Con::cout<<Con::endl;
			}
		}
		for(auto &childResultData : resultData.children)
			fPrintResults(childResultData,t +(bRoot ? "" : "\t"),false);
	};
	fPrintResults(rootResult,"",true);
	Con::cout<<"-----------------------------------------"<<Con::endl;
}
#pragma optimize("",on)
