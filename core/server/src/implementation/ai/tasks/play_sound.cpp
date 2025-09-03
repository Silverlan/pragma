// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/ai/ai_task_play_sound.h"
#include "pragma/ai/ai_schedule.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/audio/alsound_type.h>

module pragma.server.ai.tasks.play_activity;

import pragma.server.entities.components;

using namespace pragma;

ai::BehaviorNode::Result ai::TaskPlaySound::Start(const Schedule *sched, pragma::SAIComponent &ent)
{
	auto *sndName = GetSoundName(sched);
	if(sndName == nullptr)
		return Result::Failed;
	auto pSoundComponent = ent.GetEntity().GetComponent<pragma::SSoundEmitterComponent>();
	if(pSoundComponent.expired())
		return Result::Failed;
	pSoundComponent->EmitSound(*sndName, ALSoundType::NPC, {GetGain(sched), GetPitch(sched)});
	return Result::Succeeded;
}

const std::string *ai::TaskPlaySound::GetSoundName(const Schedule *sched) const
{
	auto *target = GetParameter(sched, umath::to_integral(Parameter::SoundName));
	if(target == nullptr || target->GetType() != ai::Schedule::Parameter::Type::String)
		return nullptr;
	return target->GetString();
}
float ai::TaskPlaySound::GetGain(const Schedule *sched) const
{
	auto gain = 1.f;
	auto *paramGain = GetParameter(sched, umath::to_integral(Parameter::Gain));
	if(paramGain != nullptr && paramGain->GetType() == ai::Schedule::Parameter::Type::Float)
		gain = paramGain->GetFloat();
	return gain;
}
float ai::TaskPlaySound::GetPitch(const Schedule *sched) const
{
	auto pitch = 1.f;
	auto *paramPitch = GetParameter(sched, umath::to_integral(Parameter::Pitch));
	if(paramPitch != nullptr && paramPitch->GetType() == ai::Schedule::Parameter::Type::Float)
		pitch = paramPitch->GetFloat();
	return pitch;
}

void ai::TaskPlaySound::SetSoundName(const std::string &sndName) { SetParameter(umath::to_integral(Parameter::SoundName), sndName); }
void ai::TaskPlaySound::SetGain(float gain) { SetParameter(umath::to_integral(Parameter::Gain), gain); }
void ai::TaskPlaySound::SetPitch(float pitch) { SetParameter(umath::to_integral(Parameter::Pitch), pitch); }

void ai::TaskPlaySound::Print(const Schedule *sched, std::ostream &o) const
{
	auto *sndName = GetSoundName(sched);
	o << "PlaySound[" << ((sndName != nullptr) ? *sndName : "Nothing") << "]";
	auto gain = GetGain(sched);
	auto pitch = GetPitch(sched);
	if(gain != 1.f || pitch != 1.f)
		o << "[" << gain << "][" << pitch << "]";
}
