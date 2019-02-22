#include "stdafx_server.h"
#include "pragma/ai/ai_task_turn_to_target.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/debug/debugoverlay.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/model/animation/activities.h>

using namespace pragma;

extern DLLSERVER SGame *s_game;

ai::TaskTurnToTarget::TaskTurnToTarget()
	: TaskTarget()
{}
ai::TaskTurnToTarget::TaskTurnToTarget(const TaskTurnToTarget &other)
	: TaskTarget(other)
{
	if(other.m_targetAng != nullptr)
		m_targetAng = std::make_unique<float>(*other.m_targetAng);
}
ai::BehaviorNode::Result ai::TaskTurnToTarget::Start(const Schedule *sched,pragma::SAIComponent &ent)
{
	auto r = TaskTarget::Start(sched,ent);
	if(r == Result::Failed)
		return r;
	auto *param = GetParameter(sched,1);
	if(param != nullptr)
		m_targetAng = std::make_unique<float>(param->GetFloat());
	Vector3 pos;
	if(GetTargetPosition(sched,ent,pos) == true && IsFacingTarget(ent,pos) == true)
		return Result::Succeeded;
	return ai::BehaviorNode::Result::Pending;
}

bool ai::TaskTurnToTarget::IsFacingTarget(pragma::SAIComponent &ai,const Vector3 &pos) const
{
	if(m_targetAng == nullptr)
		return false;
	auto &ent = ai.GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	return (pTrComponent.valid() && pTrComponent->GetDotProduct(pos) >= *m_targetAng) ? true : false;
}

void ai::TaskTurnToTarget::Print(const Schedule *sched,std::ostream &o) const
{
	o<<"TurnToTarget[";
	auto *target = GetParameter(sched,umath::to_integral(TaskTarget::Parameter::Target));
	auto type = (target != nullptr) ? target->GetType() : ai::Schedule::Parameter::Type::None;
	switch(type)
	{
		case ai::Schedule::Parameter::Type::Entity:
		{
			auto *ent = target->GetEntity();
			if(ent != nullptr)
			{
				std::string name {};
				auto pNameComponent = ent->GetComponent<pragma::SNameComponent>();
				if(pNameComponent.valid())
					name = pNameComponent->GetName();
				if(name.empty())
					name = ent->GetClass();
				o<<name;
			}
			else
				o<<"NULL";
			break;
		}
		case ai::Schedule::Parameter::Type::Vector:
		{
			auto &pos = *target->GetVector();
			o<<pos.x<<","<<pos.y<<","<<pos.z;
			break;
		}
		case ai::Schedule::Parameter::Type::Bool:
		{
			auto b = target->GetBool();
			if(b)
			{
				o<<"Enemy";
				break;
			}
		}
		default:
			o<<"Nothing";
			break;
	}
	o<<"]";
}

ai::BehaviorNode::Result ai::TaskTurnToTarget::Think(const Schedule *sched,pragma::SAIComponent &ent)
{
	auto r = TaskTarget::Think(sched,ent);
	if(r != Result::Succeeded)
		return r;
	Vector3 pos;
	if(GetTargetPosition(sched,ent,pos) == false)
		return Result::Failed;
	if(ent.TurnStep(pos) == true || IsFacingTarget(ent,pos) == true)
		return Result::Succeeded;
	return Result::Pending;
}
