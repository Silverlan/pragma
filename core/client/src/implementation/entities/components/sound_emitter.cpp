// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.sound_emitter;
import :client_state;
import :entities.components.flex;
import :entities.components.particle_system;

using namespace pragma;

void CSoundEmitterComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
bool CSoundEmitterComponent::ShouldRemoveSound(audio::ALSound &snd) const { return (BaseSoundEmitterComponent::ShouldRemoveSound(snd) /* && snd.GetIndex() == 0*/) ? true : false; }

void CSoundEmitterComponent::AddSound(std::shared_ptr<audio::ALSound> snd) { InitializeSound(snd); }

void CSoundEmitterComponent::PrecacheSounds()
{
	BaseSoundEmitterComponent::PrecacheSounds();
	get_client_state()->PrecacheSound("fx.fire_small", audio::ALChannel::Mono);
	ecs::CParticleSystemComponent::Precache("fire");
}

void CSoundEmitterComponent::ReceiveData(NetPacket &packet)
{
	auto numSounds = packet->Read<uint8_t>();
	for(auto i = decltype(numSounds) {0}; i < numSounds; ++i) {
		auto sndIdx = packet->Read<uint32_t>();
		auto snd = get_client_state()->GetSoundByIndex(sndIdx);
		if(snd == nullptr)
			continue;
		AddSound(snd);
	}
}

std::shared_ptr<audio::ALSound> CSoundEmitterComponent::CreateSound(std::string sndname, audio::ALSoundType type, const SoundInfo &sndInfo)
{
	std::shared_ptr<audio::ALSound> snd = get_client_state()->CreateSound(sndname, type, audio::ALCreateFlags::Mono);
	if(snd == nullptr)
		return snd;
	InitializeSound(snd);
	snd->SetGain(sndInfo.gain);
	snd->SetPitch(sndInfo.pitch);
	return snd;
}

std::shared_ptr<audio::ALSound> CSoundEmitterComponent::EmitSound(std::string sndname, audio::ALSoundType type, const SoundInfo &sndInfo)
{
	std::shared_ptr<audio::ALSound> snd = CreateSound(sndname, type, sndInfo);
	if(snd == nullptr)
		return snd;
	auto pTrComponent = GetEntity().GetTransformComponent();
	audio::ALSound *al = snd.get();
	al->SetPosition(pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {});
	//al->SetVelocity(*GetVelocity());
	// TODO: Orientation
	al->Play();
	return snd;
}
void CSoundEmitterComponent::MaintainSounds()
{
	BaseSoundEmitterComponent::MaintainSounds();
	auto pFlexComponent = GetEntity().GetComponent<CFlexComponent>();
	for(auto &snd : m_sounds) {
		if(snd->IsPlaying() == false || (snd->GetType() & audio::ALSoundType::Voice) == audio::ALSoundType::Generic)
			continue;
		if(snd->IsSoundScript() == false) {
			if(pFlexComponent.valid())
				pFlexComponent->UpdateSoundPhonemes(static_cast<audio::CALSound &>(*snd));
			continue;
		}
		auto *sndScript = dynamic_cast<audio::ALSoundScript *>(snd.get());
		if(sndScript == nullptr)
			continue;
		auto numSounds = sndScript->GetSoundCount();
		for(auto i = decltype(numSounds) {0}; i < numSounds; ++i) {
			auto *snd = sndScript->GetSound(i);
			if(snd == nullptr)
				continue;
			if(pFlexComponent.valid())
				pFlexComponent->UpdateSoundPhonemes(static_cast<audio::CALSound &>(*snd));
		}
	}
}

void CSoundEmitterComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseSoundEmitterComponent::RegisterLuaBindings(l, modEnts);
	auto defCSoundEmitter = pragma::LuaCore::create_entity_component_class<CSoundEmitterComponent, BaseSoundEmitterComponent>("SoundEmitterComponent");
	defCSoundEmitter.scope[Lua::SoundEmitter::RegisterSoundInfo()];
	modEnts[defCSoundEmitter];
}
