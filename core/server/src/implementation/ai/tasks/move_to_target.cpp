// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.tasks.move_to_target;

import :entities.components;

using namespace pragma;

void ai::TaskMoveToTarget::Print(const Schedule *sched, std::ostream &o) const
{
	o << "MoveToTarget[";
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
ai::BehaviorNode::Result ai::TaskMoveToTarget::Think(const Schedule *sched, BaseAIComponent &ent)
{
	auto r = TaskTarget::Think(sched, ent);
	if(r != Result::Succeeded)
		return r;
	Vector3 pos;
	auto moveDistance = MAX_NODE_DISTANCE;
	if(GetTargetPosition(sched, ent, pos) == false)
		return r;
	auto *param = GetParameter(sched, math::to_integral(Parameter::Target));
	if(param != nullptr) {
		if(param->GetType() == Schedule::Parameter::Type::Float)
			moveDistance = param->GetFloat();
		else {
			param = GetParameter(sched, math::to_integral(Parameter::Distance));
			if(param->GetType() == Schedule::Parameter::Type::Float)
				moveDistance = param->GetFloat();
		}
	}

	/*auto *target = GetParameter(sched,0);
	if(target == nullptr || target->GetType() == ai::Schedule::Parameter::Type::Float) // If first parameter isn't specified (or specified as distance), we'll move to our current primary target
	{
		auto *memFragment = ent->GetPrimaryTarget();
		if(memFragment == nullptr || memFragment->hEntity.IsValid() == false)
			return r;
		pos = memFragment->hEntity.get()->GetPosition();
		if(target != nullptr)
			moveDistance = target->GetFloat();
	}
	else
	{
		if(target->GetType() == ai::Schedule::Parameter::Type::Vector)
			pos = *target->GetVector();
		else if(target->GetType() == ai::Schedule::Parameter::Type::Entity)
		{
			auto *ent = target->GetEntity();
			if(ent == nullptr)
				return r;
			pos = ent->GetPosition();
		}
		else
			return r;
		auto *paramDist = GetParameter(sched,1);
		if(paramDist->GetType() == ai::Schedule::Parameter::Type::Float)
			moveDistance = paramDist->GetFloat();
	}*/

	auto moveAct = Activity::Run;
	auto *paramAct = GetParameter(sched, math::to_integral(Parameter::MoveActivity));
	if(paramAct != nullptr && paramAct->GetType() == Schedule::Parameter::Type::Int)
		moveAct = static_cast<Activity>(paramAct->GetInt());
	ent.MoveTo(pos, moveAct);
	if(ent.GetDistanceToMoveTarget() <= moveDistance) {
		ent.OnPathDestinationReached();
		return r;
	}
	return Result::Pending;
}

void ai::TaskMoveToTarget::SetMoveDistance(float dist) { SetParameter(math::to_integral(Parameter::Distance), dist); }
void ai::TaskMoveToTarget::SetMoveActivity(Activity act) { SetParameter(math::to_integral(Parameter::MoveActivity), math::to_integral(act)); }
