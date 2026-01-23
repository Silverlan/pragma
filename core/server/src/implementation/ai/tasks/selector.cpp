// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.tasks.selector;

import pragma.util;

using namespace pragma;

void ai::TaskSelector::Reset(uint32_t) {}

///////////////////////////////

ai::TaskSelectorSequential::TaskSelectorSequential() : m_currentTask(0) {}
std::shared_ptr<ai::TaskSelector> ai::TaskSelectorSequential::Copy() const { return std::shared_ptr<TaskSelector>(new TaskSelectorSequential(*this)); }
uint32_t ai::TaskSelectorSequential::GetCurrentTask() const { return m_currentTask; }
uint32_t ai::TaskSelectorSequential::SelectNextTask() { return ++m_currentTask; }
ai::SelectorType ai::TaskSelectorSequential::GetType() const { return SelectorType::Sequential; }
void ai::TaskSelectorSequential::Reset(uint32_t) { m_currentTask = 0; }

///////////////////////////////

ai::TaskSelectorRandomShuffle::TaskSelectorRandomShuffle() : m_taskOffset(0) {}
std::shared_ptr<ai::TaskSelector> ai::TaskSelectorRandomShuffle::Copy() const { return std::shared_ptr<TaskSelector>(new TaskSelectorRandomShuffle(*this)); }
uint32_t ai::TaskSelectorRandomShuffle::GetCurrentTask() const { return (m_taskOffset < m_tasks.size()) ? m_tasks[m_taskOffset] : std::numeric_limits<uint32_t>::max(); }
uint32_t ai::TaskSelectorRandomShuffle::SelectNextTask() { return ++m_taskOffset; }
ai::SelectorType ai::TaskSelectorRandomShuffle::GetType() const { return SelectorType::RandomShuffle; }
void ai::TaskSelectorRandomShuffle::Reset(uint32_t taskCount)
{
	m_tasks.resize(taskCount);
	for(auto i = decltype(taskCount) {0}; i < taskCount; ++i)
		m_tasks[i] = i;
	std::shuffle(m_tasks.begin(), m_tasks.end(), std::default_random_engine(util::clock::to_int(util::clock::get_duration_since_start())));
}
