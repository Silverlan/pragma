#include "stdafx_engine.h"
#include "pragma/debug/debug_performance_profiler.hpp"

using namespace pragma::debug;

void Profiler::Initialize()
{
	m_rootStage = ProfilingStage::Create(*this,"root");
}
bool Profiler::StartStage(ProfilingStage::StageId stage)
{
	auto *pStage = GetStage(stage);
	if(pStage == nullptr)
		return false;
	return pStage->GetTimer().Start();
}
bool Profiler::StopStage(ProfilingStage::StageId stage)
{
	auto *pStage = GetStage(stage);
	if(pStage == nullptr)
		return false;
	return pStage->GetTimer().Stop();
}
const ProfilingStage &Profiler::GetRootStage() const {return const_cast<Profiler*>(this)->GetRootStage();}
ProfilingStage &Profiler::GetRootStage() {return *m_rootStage;}
const std::vector<std::weak_ptr<ProfilingStage>> &Profiler::GetStages() const {return m_stages;}
void Profiler::AddStage(ProfilingStage &stage)
{
	m_stages.push_back(stage.shared_from_this());
}
ProfilingStage *Profiler::GetStage(ProfilingStage::StageId stage)
{
	if(stage >= m_stages.size())
		return nullptr;
	auto &wpStage = m_stages.at(stage);
	if(wpStage.expired())
	{
		m_stages.erase(m_stages.begin() +stage);
		return nullptr;
	}
	return wpStage.lock().get();
}

/////////////////

std::shared_ptr<CPUTimer> CPUTimer::Create()
{
	return std::shared_ptr<CPUTimer>{new CPUTimer{}};
}
bool CPUTimer::Start()
{
	m_startTime = std::chrono::high_resolution_clock::now();
	return true;
}
bool CPUTimer::Stop()
{
	m_stopTime = std::chrono::high_resolution_clock::now();
	return true;
}
std::unique_ptr<ProfilerResult> CPUTimer::GetResult() const
{
	auto result = std::make_unique<ProfilerResult>();
	result->duration = m_stopTime -m_startTime;
	return result;
}

/////////////////

ProfilingStage::ProfilingStage(Profiler &profiler,const std::string &name)
	: m_profiler{profiler},m_parent{},
	m_name{name}
{}
void ProfilingStage::Initialize(ProfilingStage *parent)
{
	if(parent)
		parent->m_children.push_back(shared_from_this());
	InitializeTimer();
}
void ProfilingStage::InitializeTimer()
{
	m_timer = GetProfiler().CreateTimer();
}
std::shared_ptr<ProfilingStage> ProfilingStage::Create(Profiler &profiler,const std::string &name,ProfilingStage *parent)
{
	return Create<ProfilingStage>(profiler,name,parent);
}
std::unique_ptr<ProfilerResult> ProfilingStage::GetResult() const
{
	return m_timer->GetResult();
}
ProfilingStage *ProfilingStage::GetParent()
{
	if(m_parent.expired())
		return nullptr;
	return m_parent.lock().get();
}
const std::vector<std::weak_ptr<ProfilingStage>> &ProfilingStage::GetChildren() const {return m_children;}
ProfilingStage::StageId ProfilingStage::GetStageId() const {return m_stage;}
const std::string &ProfilingStage::GetName() const {return m_name;}
Profiler &ProfilingStage::GetProfiler() {return m_profiler;}
const pragma::debug::Timer &ProfilingStage::GetTimer() const {return const_cast<ProfilingStage*>(this)->GetTimer();}
pragma::debug::Timer &ProfilingStage::GetTimer() {return *m_timer;}
bool ProfilingStage::Start() {return GetTimer().Start();}
bool ProfilingStage::Stop() {return GetTimer().Stop();}

/////////////////

std::shared_ptr<pragma::debug::Timer> CPUProfiler::CreateTimer()
{
	return CPUTimer::Create();
}
