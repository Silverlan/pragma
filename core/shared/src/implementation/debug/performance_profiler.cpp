// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :debug.performance_profiler;

using namespace pragma::debug;

void Profiler::Initialize() { m_rootStage = ProfilingStage::Create(*this, {}, "root"); }
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
const ProfilingStage &Profiler::GetRootStage() const { return const_cast<Profiler *>(this)->GetRootStage(); }
ProfilingStage &Profiler::GetRootStage() { return *m_rootStage; }
const std::vector<std::weak_ptr<ProfilingStage>> &Profiler::GetStages() const { return m_stages; }
void Profiler::AddStage(ProfilingStage &stage) { m_stages.push_back(stage.shared_from_this()); }
ProfilingStage *Profiler::GetStage(ProfilingStage::StageId stage)
{
	if(stage >= m_stages.size())
		return nullptr;
	auto &wpStage = m_stages.at(stage);
	if(wpStage.expired()) {
		m_stages.erase(m_stages.begin() + stage);
		return nullptr;
	}
	return wpStage.lock().get();
}

/////////////////

std::shared_ptr<CPUTimer> CPUTimer::Create() { return std::shared_ptr<CPUTimer> {new CPUTimer {}}; }
bool CPUTimer::Start()
{
	Timer::Start();
	m_startTime = util::Clock::now();
	return true;
}
bool CPUTimer::Stop()
{
	auto t = util::Clock::now();
	m_duration += (t - m_startTime);
	return true;
}
void CPUTimer::ResetCounters()
{
	Timer::ResetCounters();
	m_duration = {};
}
std::unique_ptr<ProfilerResult> CPUTimer::GetResult() const
{
	auto result = std::make_unique<ProfilerResult>();
	result->duration = m_duration;
	return result;
}

/////////////////

ProfilingStage::ProfilingStage(Profiler &profiler, std::thread::id tid, const std::string &name) : m_profiler {profiler}, m_parent {}, m_threadId {tid}, m_name {name} {}
void ProfilingStage::Initialize() { InitializeTimer(); }
void ProfilingStage::InitializeTimer() { m_timer = GetProfiler().CreateTimer(); }
std::shared_ptr<ProfilingStage> ProfilingStage::Create(Profiler &profiler, std::thread::id tid, const std::string &name) { return Create<ProfilingStage>(profiler, tid, name); }
std::unique_ptr<ProfilerResult> ProfilingStage::GetResult() const { return m_timer->GetResult(); }
size_t ProfilingStage::GetCount() const { return m_timer->GetCount(); }
ProfilingStage *ProfilingStage::GetParent()
{
	if(m_parent.expired())
		return nullptr;
	return m_parent.lock().get();
}
void ProfilingStage::SetParent(ProfilingStage *parent)
{
	if(m_parent.expired() == false) {
		auto curParent = m_parent.lock();
		auto it = std::find_if(curParent->m_children.begin(), curParent->m_children.end(), [this](const std::weak_ptr<ProfilingStage> &stage) { return stage.lock().get() == this; });
		assert(it != curParent->m_children.end());
		if(it != curParent->m_children.end())
			curParent->m_children.erase(it);
		m_parent = {};
	}
	if(!parent)
		return;
	parent->m_children.push_back(shared_from_this());
	m_parent = parent->shared_from_this();
}
const std::vector<std::weak_ptr<ProfilingStage>> &ProfilingStage::GetChildren() const { return m_children; }
ProfilingStage::StageId ProfilingStage::GetStageId() const { return m_stage; }
const std::string &ProfilingStage::GetName() const { return m_name; }
Profiler &ProfilingStage::GetProfiler() { return m_profiler; }
const pragma::debug::Timer &ProfilingStage::GetTimer() const { return const_cast<ProfilingStage *>(this)->GetTimer(); }
pragma::debug::Timer &ProfilingStage::GetTimer() { return *m_timer; }
bool ProfilingStage::Start()
{
	if(m_parent.expired() || m_parent.lock()->GetName() == "root")
		ResetCounters();
	return GetTimer().Start();
}
bool ProfilingStage::Stop() { return GetTimer().Stop(); }
void ProfilingStage::ResetCounters()
{
	m_timer->ResetCounters();
	for(auto &hChild : m_children) {
		if(hChild.expired())
			continue;
		hChild.lock()->ResetCounters();
	}
}

/////////////////

std::shared_ptr<pragma::debug::Timer> CPUProfiler::CreateTimer() { return CPUTimer::Create(); }
