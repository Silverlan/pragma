// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.tasks.turn_to_target;

import :entities.components;
import :game;

using namespace pragma;

ai::TaskTurnToTarget::TaskTurnToTarget() : TaskTarget() {}
ai::TaskTurnToTarget::TaskTurnToTarget(const TaskTurnToTarget &other) : TaskTarget(other)
{
	if(other.m_targetAng != nullptr)
		m_targetAng = std::make_unique<float>(*other.m_targetAng);
}
ai::BehaviorNode::Result ai::TaskTurnToTarget::Start(const Schedule *sched, BaseAIComponent &ent)
{
	auto r = TaskTarget::Start(sched, ent);
	if(r == Result::Failed)
		return r;
	auto *param = GetParameter(sched, 1);
	if(param != nullptr)
		m_targetAng = std::make_unique<float>(param->GetFloat());
	Vector3 pos;
	if(GetTargetPosition(sched, ent, pos) == true && IsFacingTarget(ent, pos) == true)
		return Result::Succeeded;
	return Result::Pending;
}

bool ai::TaskTurnToTarget::IsFacingTarget(BaseAIComponent &ai, const Vector3 &pos) const
{
	if(m_targetAng == nullptr)
		return false;
	auto &ent = ai.GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	return (pTrComponent != nullptr && pTrComponent->GetDotProduct(pos) >= *m_targetAng) ? true : false;
}

void ai::TaskTurnToTarget::Print(const Schedule *sched, std::ostream &o) const
{
	o << "TurnToTarget[";
	auto *target = GetParameter(sched, math::to_integral(Parameter::Target));
	auto type = (target != nullptr) ? target->GetType() : Schedule::Parameter::Type::None;
	switch(type) {
	case Schedule::Parameter::Type::Entity:
		{
			auto *ent = target->GetEntity();
			if(ent != nullptr) {
				std::string name {};
				auto pNameComponent = ent->GetComponent<SNameComponent>();
				if(pNameComponent.valid())
					name = pNameComponent->GetName();
				if(name.empty())
					name = *ent->GetClass();
				o << name;
			}
			else
				o << "NULL";
			break;
		}
	case Schedule::Parameter::Type::Vector:
		{
			auto &pos = *target->GetVector();
			o << pos.x << "," << pos.y << "," << pos.z;
			break;
		}
	case Schedule::Parameter::Type::Bool:
		{
			auto b = target->GetBool();
			if(b) {
				o << "Enemy";
				break;
			}
		}
	default:
		o << "Nothing";
		break;
	}
	o << "]";
}

ai::BehaviorNode::Result ai::TaskTurnToTarget::Think(const Schedule *sched, BaseAIComponent &ent)
{
	auto r = TaskTarget::Think(sched, ent);
	if(r != Result::Succeeded)
		return r;
	Vector3 pos;
	if(GetTargetPosition(sched, ent, pos) == false)
		return Result::Failed;
	if(ent.TurnStep(pos) == true || IsFacingTarget(ent, pos) == true)
		return Result::Succeeded;
	return Result::Pending;
}
