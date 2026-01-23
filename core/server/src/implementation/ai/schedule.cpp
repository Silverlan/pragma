// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.schedule;

import :entities.components;

using namespace pragma;

std::shared_ptr<ai::Schedule> ai::Schedule::Create() { return std::shared_ptr<Schedule>(new Schedule()); }
ai::Schedule::Schedule() : std::enable_shared_from_this<Schedule>(), m_rootTask(pragma::util::make_shared<BehaviorNode>(BehaviorNode::Type::Sequence)) { m_rootTask->SetDebugName("Root"); }
void ai::Schedule::Cancel() const
{
	if(m_rootTask->IsActive() == false)
		return;
	m_rootTask->Stop();
}
ai::BehaviorNode::Result ai::Schedule::Start(BaseAIComponent &ent)
{
	std::function<void(const BehaviorNode &)> fResetNode = nullptr;
	fResetNode = [&fResetNode](const BehaviorNode &node) {
		auto &debugInfo = node.GetDebugInfo();
		debugInfo.lastResult = BehaviorNode::Result::Initial;
		for(auto &child : node.GetNodes())
			fResetNode(*child);
	};
	fResetNode(*m_rootTask);
	return m_rootTask->Start(this, ent);
}
ai::BehaviorNode::Result ai::Schedule::Think(BaseAIComponent &ent) const { return m_rootTask->Think(this, static_cast<SAIComponent &>(ent)); }
ai::BehaviorNode &ai::Schedule::GetRootNode() const { return *m_rootTask; }
std::shared_ptr<ai::Schedule> ai::Schedule::Copy() const
{
	auto *cpy = new Schedule();
	cpy->m_rootTask = m_rootTask->Copy();
	cpy->m_params.reserve(m_params.size());
	for(auto &p : m_params)
		cpy->m_params.push_back(std::unique_ptr<ParameterInfo>(new ParameterInfo(*p)));
	return std::shared_ptr<Schedule>(cpy);
}
void ai::Schedule::SetInterruptFlags(uint32_t f) { m_interruptFlags = f; }
void ai::Schedule::AddInterruptFlags(uint32_t f) { m_interruptFlags |= f; }
uint32_t ai::Schedule::GetInterruptFlags() const { return m_interruptFlags; }

void ai::Schedule::DebugPrint(std::stringstream &ss) const { m_rootTask->DebugPrint(this, ss); }
