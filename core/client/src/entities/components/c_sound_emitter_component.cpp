/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/audio/alsound_type.h>
#include <pragma/audio/alsoundscript.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT ClientState *client;

void CSoundEmitterComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
bool CSoundEmitterComponent::ShouldRemoveSound(ALSound &snd) const { return (BaseSoundEmitterComponent::ShouldRemoveSound(snd) /* && snd.GetIndex() == 0*/) ? true : false; }

void CSoundEmitterComponent::AddSound(std::shared_ptr<ALSound> snd) { InitializeSound(snd); }

void CSoundEmitterComponent::PrecacheSounds()
{
	BaseSoundEmitterComponent::PrecacheSounds();
	client->PrecacheSound("fx.fire_small", ALChannel::Mono);
	CParticleSystemComponent::Precache("fire");
}

void CSoundEmitterComponent::ReceiveData(NetPacket &packet)
{
	auto numSounds = packet->Read<uint8_t>();
	for(auto i = decltype(numSounds) {0}; i < numSounds; ++i) {
		auto sndIdx = packet->Read<uint32_t>();
		auto snd = client->GetSoundByIndex(sndIdx);
		if(snd == nullptr)
			continue;
		AddSound(snd);
	}
}

std::shared_ptr<ALSound> CSoundEmitterComponent::CreateSound(std::string sndname, ALSoundType type, const SoundInfo &sndInfo)
{
	std::shared_ptr<ALSound> snd = client->CreateSound(sndname, type, ALCreateFlags::Mono);
	if(snd == NULL)
		return snd;
	InitializeSound(snd);
	snd->SetGain(sndInfo.gain);
	snd->SetPitch(sndInfo.pitch);
	return snd;
}

std::shared_ptr<ALSound> CSoundEmitterComponent::EmitSound(std::string sndname, ALSoundType type, const SoundInfo &sndInfo)
{
	std::shared_ptr<ALSound> snd = CreateSound(sndname, type, sndInfo);
	if(snd == NULL)
		return snd;
	auto pTrComponent = GetEntity().GetTransformComponent();
	ALSound *al = snd.get();
	al->SetPosition(pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {});
	//al->SetVelocity(*GetVelocity());
	// TODO: Orientation
	al->Play();
	return snd;
}
void CSoundEmitterComponent::MaintainSounds()
{
	BaseSoundEmitterComponent::MaintainSounds();
	auto pFlexComponent = GetEntity().GetComponent<pragma::CFlexComponent>();
	for(auto &snd : m_sounds) {
		if(snd->IsPlaying() == false || (snd->GetType() & ALSoundType::Voice) == ALSoundType::Generic)
			continue;
		if(snd->IsSoundScript() == false) {
			if(pFlexComponent.valid())
				pFlexComponent->UpdateSoundPhonemes(static_cast<CALSound &>(*snd));
			continue;
		}
		auto *sndScript = dynamic_cast<ALSoundScript *>(snd.get());
		if(sndScript == nullptr)
			continue;
		auto numSounds = sndScript->GetSoundCount();
		for(auto i = decltype(numSounds) {0}; i < numSounds; ++i) {
			auto *snd = sndScript->GetSound(i);
			if(snd == nullptr)
				continue;
			if(pFlexComponent.valid())
				pFlexComponent->UpdateSoundPhonemes(static_cast<CALSound &>(*snd));
		}
	}
}
