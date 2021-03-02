/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/ai/ai_behavior.h"
#include <random>
#include <sharedutils/util_clock.hpp>

using namespace pragma;

void ai::TaskSelector::Reset(uint32_t) {}

///////////////////////////////

ai::TaskSelectorSequential::TaskSelectorSequential()
	: m_currentTask(0)
{}
std::shared_ptr<ai::TaskSelector> ai::TaskSelectorSequential::Copy() const {return std::shared_ptr<ai::TaskSelector>(new TaskSelectorSequential(*this));}
uint32_t ai::TaskSelectorSequential::GetCurrentTask() const {return m_currentTask;}
uint32_t ai::TaskSelectorSequential::SelectNextTask() {return ++m_currentTask;}
ai::SelectorType ai::TaskSelectorSequential::GetType() const {return ai::SelectorType::Sequential;}
void ai::TaskSelectorSequential::Reset(uint32_t)
{
	m_currentTask = 0;
}

///////////////////////////////

ai::TaskSelectorRandomShuffle::TaskSelectorRandomShuffle()
	: m_taskOffset(0)
{}
std::shared_ptr<ai::TaskSelector> ai::TaskSelectorRandomShuffle::Copy() const {return std::shared_ptr<ai::TaskSelector>(new TaskSelectorRandomShuffle(*this));}
uint32_t ai::TaskSelectorRandomShuffle::GetCurrentTask() const {return (m_taskOffset < m_tasks.size()) ? m_tasks[m_taskOffset] : std::numeric_limits<uint32_t>::max();}
uint32_t ai::TaskSelectorRandomShuffle::SelectNextTask() {return ++m_taskOffset;}
ai::SelectorType ai::TaskSelectorRandomShuffle::GetType() const {return ai::SelectorType::RandomShuffle;}
void ai::TaskSelectorRandomShuffle::Reset(uint32_t taskCount)
{
	m_tasks.resize(taskCount);
	for(auto i=decltype(taskCount){0};i<taskCount;++i)
		m_tasks[i] = i;
	std::shuffle(m_tasks.begin(),m_tasks.end(),std::default_random_engine(util::clock::to_int(util::clock::get_duration_since_start())));
}
