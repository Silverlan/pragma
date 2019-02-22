#include "stdafx_server.h"
#include "pragma/ai/ai_task_target.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>

using namespace pragma;

const BaseEntity *ai::TaskTarget::GetTargetEntity(const Schedule *sched,pragma::SAIComponent &ent) const
{
	auto *target = GetParameter(sched,umath::to_integral(Parameter::Target));
	auto type = (target != nullptr) ? target->GetType() : ai::Schedule::Parameter::Type::None;
	if(type == ai::Schedule::Parameter::Type::Entity)
		return target->GetEntity();
	else if(type != ai::Schedule::Parameter::Type::Vector)
	{
		auto *memFragment = ent.GetPrimaryTarget();
		if(memFragment == nullptr || memFragment->hEntity.IsValid() == false)
			return nullptr;
		return memFragment->hEntity.get();
	}
	return nullptr;
}

void ai::TaskTarget::SetTarget(const Vector3 &target)
{
	SetParameter(umath::to_integral(Parameter::Target),target);
}
void ai::TaskTarget::SetTarget(const EntityHandle &hEnt)
{
	SetParameter(umath::to_integral(Parameter::Target),hEnt.get());
}

bool ai::TaskTarget::GetTargetPosition(const Schedule *sched,pragma::SAIComponent &ent,Vector3 &pos) const
{
	auto *target = GetParameter(sched,umath::to_integral(Parameter::Target));
	auto type = (target != nullptr) ? target->GetType() : ai::Schedule::Parameter::Type::None;
	if(type != ai::Schedule::Parameter::Type::Entity && type != ai::Schedule::Parameter::Type::Vector)
	{
		auto *memFragment = ent.GetPrimaryTarget();
		if(memFragment == nullptr || memFragment->hEntity.IsValid() == false)
			return false;
		auto pTrComponentEnt = memFragment->hEntity.get()->GetTransformComponent();
		if(pTrComponentEnt.expired())
			return false;
		pos = pTrComponentEnt->GetPosition();
	}
	else
	{
		if(type == ai::Schedule::Parameter::Type::Vector)
			pos = *target->GetVector();
		else if(type == ai::Schedule::Parameter::Type::Entity)
		{
			auto *ent = target->GetEntity();
			if(ent == nullptr)
				return false;
			auto pTrComponentEnt = ent->GetTransformComponent();
			if(pTrComponentEnt.expired())
				return false;
			pos = pTrComponentEnt->GetPosition();
		}
		else
			return false;
	}
	return true;
}
