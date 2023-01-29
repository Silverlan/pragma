/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/ai/ai_task_look_at_target.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/debug/debugoverlay.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_name_component.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/model/animation/activities.h>

using namespace pragma;

extern DLLSERVER SGame *s_game;

void ai::TaskLookAtTarget::SetLookDuration(float dur) { SetParameter(umath::to_integral(Parameter::LookDuration), dur); }
void ai::TaskLookAtTarget::Print(const Schedule *sched, std::ostream &o) const
{
	o << "LookAt[";
	auto *target = GetParameter(sched, umath::to_integral(TaskTarget::Parameter::Target));
	auto type = (target != nullptr) ? target->GetType() : ai::Schedule::Parameter::Type::None;
	switch(type) {
	case ai::Schedule::Parameter::Type::Entity:
		{
			auto *ent = target->GetEntity();
			if(ent != nullptr) {
				std::string name {};
				auto pNameComponent = ent->GetComponent<pragma::SNameComponent>();
				if(pNameComponent.valid())
					name = pNameComponent->GetName();
				if(name.empty())
					name = ent->GetClass();
				o << name;
			}
			else
				o << "NULL";
			break;
		}
	case ai::Schedule::Parameter::Type::Vector:
		{
			auto &pos = *target->GetVector();
			o << pos.x << "," << pos.y << "," << pos.z;
			break;
		}
	case ai::Schedule::Parameter::Type::Bool:
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
ai::BehaviorNode::Result ai::TaskLookAtTarget::Start(const Schedule *sched, pragma::SAIComponent &npc)
{
	auto r = TaskTarget::Start(sched, npc);
	if(r == Result::Failed)
		return r;
	auto lookTime = std::numeric_limits<float>::max();
	auto *param = GetParameter(sched, umath::to_integral(Parameter::LookDuration));
	if(param != nullptr)
		lookTime = s_game->CurTime() + param->GetFloat();

	auto *ent = GetTargetEntity(sched, npc);
	if(ent != nullptr)
		npc.SetLookTarget(*ent, lookTime);
	else {
		Vector3 pos;
		if(GetTargetPosition(sched, npc, pos) == true)
			npc.SetLookTarget(pos, lookTime);
	}
	return ai::BehaviorNode::Result::Succeeded;
}
