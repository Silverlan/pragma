// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server;
import :ai.tasks.target;

import :entities.components;

using namespace pragma;

const pragma::ecs::BaseEntity *ai::TaskTarget::GetTargetEntity(const Schedule *sched, pragma::BaseAIComponent &ent) const
{
	auto *target = GetParameter(sched, umath::to_integral(Parameter::Target));
	auto type = (target != nullptr) ? target->GetType() : ai::Schedule::Parameter::Type::None;
	if(type == ai::Schedule::Parameter::Type::Entity)
		return target->GetEntity();
	else if(type != ai::Schedule::Parameter::Type::Vector) {
		auto *memFragment = static_cast<SAIComponent&>(ent).GetPrimaryTarget();
		if(memFragment == nullptr || memFragment->hEntity.valid() == false)
			return nullptr;
		return memFragment->hEntity.get();
	}
	return nullptr;
}

void ai::TaskTarget::SetTarget(const Vector3 &target) { SetParameter(umath::to_integral(Parameter::Target), target); }
void ai::TaskTarget::SetTarget(const EntityHandle &hEnt) { SetParameter(umath::to_integral(Parameter::Target), hEnt.get()); }

bool ai::TaskTarget::GetTargetPosition(const Schedule *sched, pragma::BaseAIComponent &ent, Vector3 &pos) const
{
	auto *target = GetParameter(sched, umath::to_integral(Parameter::Target));
	auto type = (target != nullptr) ? target->GetType() : ai::Schedule::Parameter::Type::None;
	if(type != ai::Schedule::Parameter::Type::Entity && type != ai::Schedule::Parameter::Type::Vector) {
		auto *memFragment = static_cast<SAIComponent&>(ent).GetPrimaryTarget();
		if(memFragment == nullptr || memFragment->hEntity.valid() == false)
			return false;
		auto pTrComponentEnt = memFragment->hEntity.get()->GetTransformComponent();
		if(!pTrComponentEnt)
			return false;
		pos = pTrComponentEnt->GetPosition();
	}
	else {
		if(type == ai::Schedule::Parameter::Type::Vector)
			pos = *target->GetVector();
		else if(type == ai::Schedule::Parameter::Type::Entity) {
			auto *ent = target->GetEntity();
			if(ent == nullptr)
				return false;
			auto pTrComponentEnt = ent->GetTransformComponent();
			if(!pTrComponentEnt)
				return false;
			pos = pTrComponentEnt->GetPosition();
		}
		else
			return false;
	}
	return true;
}
