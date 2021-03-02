/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/ai/ai_task.h"
#include <mathutil/umath.h>
#include "pragma/ai/ai_behavior.h"

using namespace pragma;

std::shared_ptr<ai::Schedule> ai::Schedule::Create()
{
	return std::shared_ptr<Schedule>(new Schedule());
}
ai::Schedule::Schedule()
	: std::enable_shared_from_this<Schedule>(),m_rootTask(std::make_shared<ai::BehaviorNode>(ai::BehaviorNode::Type::Sequence))
{
	m_rootTask->SetDebugName("Root");
}
void ai::Schedule::Cancel() const
{
	if(m_rootTask->IsActive() == false)
		return;
	m_rootTask->Stop();
}
ai::BehaviorNode::Result ai::Schedule::Start(pragma::SAIComponent &ent)
{
	std::function<void(const ai::BehaviorNode&)> fResetNode = nullptr;
	fResetNode = [&fResetNode](const ai::BehaviorNode &node) {
		auto &debugInfo = node.GetDebugInfo();
		debugInfo.lastResult = BehaviorNode::Result::Initial;
		for(auto &child : node.GetNodes())
			fResetNode(*child);
	};
	fResetNode(*m_rootTask);
	return m_rootTask->Start(this,ent);
}
ai::BehaviorNode::Result ai::Schedule::Think(pragma::SAIComponent &ent) const {return m_rootTask->Think(this,ent);}
ai::BehaviorNode &ai::Schedule::GetRootNode() const {return *m_rootTask;}
std::shared_ptr<ai::Schedule> ai::Schedule::Copy() const
{
	auto *cpy = new ai::Schedule();
	cpy->m_rootTask = m_rootTask->Copy();
	cpy->m_params.reserve(m_params.size());
	for(auto &p : m_params)
		cpy->m_params.push_back(std::unique_ptr<ParameterInfo>(new ParameterInfo(*p)));
	return std::shared_ptr<ai::Schedule>(cpy);
}
void ai::Schedule::SetInterruptFlags(uint32_t f) {m_interruptFlags = f;}
void ai::Schedule::AddInterruptFlags(uint32_t f) {m_interruptFlags |= f;}
uint32_t ai::Schedule::GetInterruptFlags() const {return m_interruptFlags;}

void ai::Schedule::DebugPrint(std::stringstream &ss) const
{
	m_rootTask->DebugPrint(this,ss);
}
