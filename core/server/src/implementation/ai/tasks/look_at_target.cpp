// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.tasks.look_at_target;

import :entities.components;
import :game;

using namespace pragma;

void ai::TaskLookAtTarget::SetLookDuration(float dur) { SetParameter(math::to_integral(Parameter::LookDuration), dur); }
void ai::TaskLookAtTarget::Print(const Schedule *sched, std::ostream &o) const
{
	o << "LookAt[";
	auto *target = GetParameter(sched, math::to_integral(TaskTarget::Parameter::Target));
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
ai::BehaviorNode::Result ai::TaskLookAtTarget::Start(const Schedule *sched, BaseAIComponent &npc)
{
	auto r = TaskTarget::Start(sched, npc);
	if(r == Result::Failed)
		return r;
	auto lookTime = std::numeric_limits<float>::max();
	auto *param = GetParameter(sched, math::to_integral(Parameter::LookDuration));
	if(param != nullptr)
		lookTime = SGame::Get()->CurTime() + param->GetFloat();

	auto *ent = GetTargetEntity(sched, npc);
	if(ent != nullptr)
		npc.SetLookTarget(*ent, lookTime);
	else {
		Vector3 pos;
		if(GetTargetPosition(sched, npc, pos) == true)
			npc.SetLookTarget(pos, lookTime);
	}
	return Result::Succeeded;
}
