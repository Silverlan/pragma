// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <vector>
#include <memory>
#include "pragma/lua/core.hpp"

#include <string>

module pragma.shared;

import :entities.components.base_sound_emitter;

using namespace pragma;

ComponentEventId BaseSoundEmitterComponent::EVENT_ON_SOUND_CREATED = pragma::INVALID_COMPONENT_ID;
void BaseSoundEmitterComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_SOUND_CREATED = registerEvent("ON_SOUND_CREATED", ComponentEventInfo::Type::Broadcast); }
BaseSoundEmitterComponent::BaseSoundEmitterComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
BaseSoundEmitterComponent::~BaseSoundEmitterComponent()
{
	for(int i = 0; i < m_sounds.size(); i++) {
		ALSound *al = m_sounds[i].get();
		if(al->IsLooping()) // Stop all looping sounds immediately
			al->Stop();
	}
}
void BaseSoundEmitterComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent("transform");
}
std::shared_ptr<ALSound> BaseSoundEmitterComponent::CreateSound(std::string snd, pragma::audio::ALSoundType, const SoundInfo &) { return std::shared_ptr<ALSound>(); }
std::shared_ptr<ALSound> BaseSoundEmitterComponent::EmitSound(std::string snd, pragma::audio::ALSoundType, const SoundInfo &) { return std::shared_ptr<ALSound>(); }

void BaseSoundEmitterComponent::StopSounds()
{
	for(int i = 0; i < m_sounds.size(); i++) {
		ALSound *al = m_sounds[i].get();
		al->Stop();
	}
	m_sounds.clear();

	SetTickPolicy(TickPolicy::Never);
}

void BaseSoundEmitterComponent::GetSounds(std::vector<std::shared_ptr<ALSound>> **sounds) { *sounds = &m_sounds; }

bool BaseSoundEmitterComponent::ShouldRemoveSound(ALSound &snd) const
{
	// Index 0 = shared sound
	return (/*snd.GetIndex() == 0 && */ snd.IsPlaying() == false) ? true : false;
}

void BaseSoundEmitterComponent::OnTick(double dt) { MaintainSounds(); }

void BaseSoundEmitterComponent::MaintainSounds()
{
	for(auto it = m_sounds.begin(); it != m_sounds.end();) {
		auto &snd = *it;
		if(ShouldRemoveSound(*snd) == true && snd.use_count() == 2)
			it = m_sounds.erase(it);
		else {
			UpdateSoundTransform(*snd);
			++it;
		}
	}
}

void BaseSoundEmitterComponent::InitializeSound(const std::shared_ptr<ALSound> &ptrSnd)
{
	auto &ent = GetEntity();
	auto *snd = static_cast<ALSound *>(ptrSnd.get());
	snd->SetSource(&ent);
	snd->SetRolloffFactor(2.f);
	snd->SetReferenceDistance(150.f);
	snd->SetMaxDistance(5'000.f);

	UpdateSoundTransform(*snd);

	m_sounds.push_back(ptrSnd);
	SetTickPolicy(TickPolicy::Always);

	CEOnSoundCreated evData {ptrSnd};
	BroadcastEvent(EVENT_ON_SOUND_CREATED, evData);
}

void BaseSoundEmitterComponent::PrecacheSounds() {}

void BaseSoundEmitterComponent::UpdateSoundTransform(ALSound &snd) const
{
	if(snd.IsRelative())
		return; // TODO: Allow transforms for relative sound sources (but make sure 'global' flag isn't set if it's a sound-script!)
	auto &ent = GetEntity();
	auto tComponent = ent.GetTransformComponent();
	snd.SetPosition(tComponent->GetPosition());
	snd.SetDirection(tComponent->GetForward());
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(pVelComponent.valid())
		snd.SetVelocity(pVelComponent->GetVelocity());
}

/////////////////

CEOnSoundCreated::CEOnSoundCreated(const std::shared_ptr<ALSound> &sound) : sound {sound} {}
void CEOnSoundCreated::PushArguments(lua_State *l) { Lua::Push<std::shared_ptr<ALSound>>(l, sound); }
