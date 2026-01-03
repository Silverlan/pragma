// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.sound_emitter;

import :audio;

import :entities.base;
import :server_state;

using namespace pragma;

void SSoundEmitterComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SSoundEmitterComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	auto &sounds = m_sounds;
	auto offset = packet->GetOffset();
	auto numSounds = math::min(sounds.size(), static_cast<size_t>(std::numeric_limits<uint8_t>::max()));
	packet->Write<uint8_t>(numSounds);
	auto numWritten = decltype(numSounds) {0};
	for(auto &snd : sounds) {
		if(dynamic_cast<audio::SALSoundBase *>(snd.get())->IsShared() == false)
			continue;
		packet->Write<uint32_t>(snd->GetIndex());
		if(++numWritten == numSounds)
			break;
	}
	packet->Write<uint8_t>(numWritten, &offset);
}
void SSoundEmitterComponent::UpdateSoundTransform(audio::ALSound &snd) const
{
	auto *baseSnd = audio::SALSound::GetBase(&snd);
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent != nullptr) {
		baseSnd->SetPosition(pTrComponent->GetPosition(), true);
		baseSnd->SetDirection(pTrComponent->GetForward(), true);
	}
	auto pVelComponent = ent.GetComponent<VelocityComponent>();
	if(pVelComponent.valid())
		baseSnd->SetVelocity(pVelComponent->GetVelocity(), true);
}
std::shared_ptr<audio::ALSound> SSoundEmitterComponent::CreateSound(std::string sndname, audio::ALSoundType type, const SoundInfo &sndInfo)
{
	auto flags = audio::ALCreateFlags::Mono;
	if(sndInfo.transmit == false)
		flags |= audio::ALCreateFlags::DontTransmit;
	auto ptrSnd = ServerState::Get()->CreateSound(sndname, type, flags);
	auto *snd = static_cast<audio::ALSound *>(ptrSnd.get());
	if(snd == nullptr)
		return ptrSnd;
	InitializeSound(ptrSnd);
	ptrSnd->SetGain(sndInfo.gain);
	ptrSnd->SetPitch(sndInfo.pitch);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() && sndInfo.transmit == true) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<unsigned int>(snd->GetIndex());
		ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SOUND, p, networking::Protocol::FastUnreliable);
	}
	return ptrSnd;
}
std::shared_ptr<audio::ALSound> SSoundEmitterComponent::EmitSound(std::string sndname, audio::ALSoundType type, const SoundInfo &sndInfo)
{
	std::shared_ptr<audio::ALSound> snd = CreateSound(sndname, type, sndInfo);
	auto *al = snd.get();
	if(al == nullptr)
		return snd;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent != nullptr)
		al->SetPosition(pTrComponent->GetPosition());
	auto pVelComponent = ent.GetComponent<VelocityComponent>();
	if(pVelComponent.valid())
		al->SetVelocity(pVelComponent->GetVelocity());
	// TODO: Orientation
	al->Play();
	return snd;
}
