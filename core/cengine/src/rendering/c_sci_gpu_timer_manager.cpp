#include "stdafx_cengine.h"
#include "pragma/rendering/c_sci_gpu_timer_manager.hpp"
#include <pragma/console/c_cvar.h>
#include <pragma/console/convars.h>
#include <pragma/console/c_cvar_global_functions.h>
#include <queries/prosper_query_pool.hpp>
#include <queries/prosper_timer_query.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

static CVar cvTimerQueries = GetClientConVar("cl_gpu_timer_queries_enabled");
CSciGPUTimerManager::CSciGPUTimerManager()
	: m_pool(nullptr)
{
	InitializeQueries();
}

void CSciGPUTimerManager::InitializeQueries()
{
	for(auto &query : m_timerQueries)
	{
		if(query == nullptr)
			query = std::shared_ptr<Query>(new Query());
	}
	auto &dev = c_engine->GetDevice();
	auto setBufferCount = c_engine->GetSwapchainImageCount();
	auto numBuffersTotal = setBufferCount *2; // Draw command buffers +compute command buffers
	// prosper TODO: The pool causes a crash on exit?
	m_pool = nullptr;//prosper::util::create_query_pool(*c_engine,vk::QueryType::eTimestamp,numBuffersTotal *umath::to_integral(GPUTimerEvent::Count) *2); // Each timer query requires two timestamp queries
	m_cmdBufferIndices.clear();
	if(m_pool == nullptr)
		return;
	for(auto i=decltype(setBufferCount){0};i<setBufferCount;++i)
	{
		auto &drawCmd = c_engine->GetDrawCommandBuffer(i);
		m_cmdBufferIndices.insert(decltype(m_cmdBufferIndices)::value_type(drawCmd.get(),m_cmdBufferIndices.size()));
	}
	for(auto i=decltype(setBufferCount){0};i<setBufferCount;++i)
	{
		//auto &computeCmd = context.GetComputeCmd(i); // prosper TODO
		//m_cmdBufferIndices.insert(decltype(m_cmdBufferIndices)::value_type(computeCmd.get(),m_cmdBufferIndices.size()));
	}
	for(auto &query : m_timerQueries)
	{
		query->cmdQueries.resize(numBuffersTotal);
		for(auto &pair : m_cmdBufferIndices)
		{
			auto &cmdInfo = query->cmdQueries.at(pair.second) = std::shared_ptr<Query::CmdInfo>(new Query::CmdInfo());
			cmdInfo->query = prosper::util::create_timer_query(*m_pool,pair.first,Anvil::PipelineStageFlagBits::TOP_OF_PIPE_BIT);
		}
	}
}

std::size_t CSciGPUTimerManager::GetCommandBufferIndex(GPUTimerEvent e) const
{
	auto &drawCmd = GetCommandBuffer(e);
	auto it = std::find_if(m_cmdBufferIndices.begin(),m_cmdBufferIndices.end(),[&drawCmd](const std::pair<std::shared_ptr<prosper::CommandBuffer>,std::size_t> &pair) {
		return pair.first.get() == &drawCmd;
	});
	if(it == m_cmdBufferIndices.end())
		return std::numeric_limits<std::size_t>::max();
	return it->second;
}

const prosper::CommandBuffer &CSciGPUTimerManager::GetCommandBuffer(GPUTimerEvent e) const
{
	//auto &drawCmd = (umath::to_integral(e) < umath::to_integral(GPUTimerEvent::ComputeStart)) ? context.GetDrawCmd() : context.GetComputeCmd(); // prosper TODO
	auto &drawCmd = c_engine->GetDrawCommandBuffer();
	auto it = std::find_if(m_cmdBufferIndices.begin(),m_cmdBufferIndices.end(),[&drawCmd](const std::pair<std::shared_ptr<prosper::CommandBuffer>,std::size_t> &pair) {
		return pair.first.get() == drawCmd.get();
	});
	if(it == m_cmdBufferIndices.end())
		const_cast<CSciGPUTimerManager*>(this)->InitializeQueries(); // Command buffers have been changed, we need to reload all queries
	return *drawCmd;
}

bool CSciGPUTimerManager::IsTimerActive(GPUTimerEvent e) const {return (GetTimerState(e) == TimerState::Started) ? true : false;}
CSciGPUTimerManager::TimerState CSciGPUTimerManager::GetTimerState(GPUTimerEvent e) const {return m_timerQueries.at(umath::to_integral(e))->cmdQueries.at(GetCommandBufferIndex(e))->state;}

void CSciGPUTimerManager::StartTimer(GPUTimerEvent e)
{
	if(cvTimerQueries->GetBool() == false || m_pool == nullptr)
		return;
	auto idx = umath::to_integral(e);
	if(idx >= m_timerQueries.size())
		return;
	auto &query = m_timerQueries.at(idx);
	auto &cmdInfo = query->cmdQueries.at(GetCommandBufferIndex(e));
	if(GetTimerState(e) == TimerState::Stopped)
	{
		if(cmdInfo->query->QueryResult(query->result) == false)
			return;
		cmdInfo->resultFrame = c_engine->GetLastFrameId();
	}
	cmdInfo->state = TimerState::Started;
	cmdInfo->query->Begin();
}
void CSciGPUTimerManager::StopTimer(GPUTimerEvent e)
{
	if(cvTimerQueries->GetBool() == false || m_pool == nullptr)
		return;
	if(GetTimerState(e) != TimerState::Started)
		return;
	auto idx = umath::to_integral(e);
	if(idx >= m_timerQueries.size())
		return;
	auto &query = m_timerQueries.at(idx);
	auto &cmdInfo = query->cmdQueries.at(GetCommandBufferIndex(e));
	cmdInfo->state = TimerState::Stopped;
	cmdInfo->query->End();
}

bool CSciGPUTimerManager::IsResultAvailable(GPUTimerEvent e) const
{
	auto r = 0.f;
	return GetResult(e,r);
}
void CSciGPUTimerManager::GetResults(std::vector<float> &results) const
{
	auto count = umath::to_integral(GPUTimerEvent::Count);
	results.reserve(results.size() +count);
	for(auto i=decltype(count){0};i<count;++i)
	{
		results.push_back(std::numeric_limits<float>::max());
		GetResult(static_cast<GPUTimerEvent>(i),results.back());
	}
}
bool CSciGPUTimerManager::GetResult(GPUTimerEvent e,float &v) const
{
	if(cvTimerQueries->GetBool() == false || m_pool == nullptr)
		return false;
	auto idx = umath::to_integral(e);
	if(idx >= m_timerQueries.size())
		return false;
	auto &query = m_timerQueries.at(idx);
	//auto &cmdInfo = query->cmdQueries.at(GetCommandBufferIndex());
	//if(cmdInfo->resultFrame == std::numeric_limits<uint64_t>::max())
	//	return false;
	v = query->result;
	return true;
}
float CSciGPUTimerManager::GetResult(GPUTimerEvent e) const
{
	auto r = 0.f;
	GetResult(e,r);
	return r;
}
std::string CSciGPUTimerManager::EventToString(GPUTimerEvent e)
{
	switch(e)
	{
		case GPUTimerEvent::GUI:
			return "GUI";
		case GPUTimerEvent::WorldScene:
			return "WorldScene";
		case GPUTimerEvent::Particles:
			return "Particles";
		case GPUTimerEvent::UpdateExposure:
			return "UpdateExposure";
		case GPUTimerEvent::Shadow:
			return "Shadow";
		case GPUTimerEvent::Skybox:
			return "Skybox";
		case GPUTimerEvent::World:
			return "World";
		case GPUTimerEvent::WorldTranslucent:
			return "WorldTranslucent";
		case GPUTimerEvent::Water:
			return "Water";
		case GPUTimerEvent::Debug:
			return "Debug";
		case GPUTimerEvent::View:
			return "View";
		case GPUTimerEvent::ViewParticles:
			return "ViewParticles";
		case GPUTimerEvent::Glow:
			return "Glow";
		case GPUTimerEvent::BlurHDRBloom:
			return "BlurHDRBloom";
		case GPUTimerEvent::GlowAdditive:
			return "GlowAdditive";
		case GPUTimerEvent::FinalAdditive:
			return "FinalAdditive";
		case GPUTimerEvent::HDR:
			return "HDR";
		case GPUTimerEvent::SceneToScreen:
			return "SceneToScreen";
		case GPUTimerEvent::GameRender:
			return "GameRender";
		case GPUTimerEvent::Mesh0:
			return "Mesh0";
		case GPUTimerEvent::Mesh1:
			return "Mesh1";
		case GPUTimerEvent::Mesh2:
			return "Mesh2";
		case GPUTimerEvent::Mesh3:
			return "Mesh3";
		case GPUTimerEvent::Mesh4:
			return "Mesh4";
		case GPUTimerEvent::Mesh5:
			return "Mesh5";
		case GPUTimerEvent::Mesh6:
			return "Mesh6";
		case GPUTimerEvent::Mesh7:
			return "Mesh7";
		case GPUTimerEvent::Mesh8:
			return "Mesh8";
		case GPUTimerEvent::DebugMesh:
			return "DebugMesh";
		case GPUTimerEvent::ShadowLayerBlit:
			return "ShadowLayerBlit";
		case GPUTimerEvent::WaterSurface:
			return "WaterSurface";
		default:
			return "Invalid";
	}
}

/////////////////////////

void Console::commands::cl_gpu_timer_queries_dump(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	Con::cout<<"-------- GPU-Timer Query Results --------"<<Con::endl;
	auto &manager = c_engine->GetGPUTimerManager();
	std::vector<float> results;
	manager.GetResults(results);
	for(auto i=decltype(results.size()){0};i<results.size();++i)
	{
		auto ev = static_cast<GPUTimerEvent>(i);
		Con::cout<<CSciGPUTimerManager::EventToString(ev)<<": "<<results.at(i)<<Con::endl;
	}
	Con::cout<<"-----------------------------------------"<<Con::endl;
}
