// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.audio.dsp.base;
import :client_state;
import :entities.components.toggle;
import :game;

using namespace pragma;

CBaseSoundDspComponent::~CBaseSoundDspComponent() { DetachAllSoundSources(); }
void CBaseSoundDspComponent::Initialize()
{
	BaseEnvSoundDspComponent::Initialize();
	SetTickPolicy(TickPolicy::Always); // TODO
}
void CBaseSoundDspComponent::OnTick(double dt)
{
	if(m_dsp == nullptr)
		return;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pToggleComponent = ent.GetComponent<CToggleComponent>();
	if(pTrComponent == nullptr || (pToggleComponent.valid() && pToggleComponent->IsTurnedOn() == false))
		return;
	auto radiusInnerSqr = math::pow2(m_kvInnerRadius);
	auto radiusOuterSqr = math::pow2(m_kvOuterRadius);
	auto &pos = pTrComponent->GetPosition();
	auto &sounds = get_client_state()->GetSounds();
	for(auto &rsnd : sounds) {
		auto &snd = rsnd.get();
		if(snd.IsPlaying() == false)
			continue;
		auto &alSnd = *static_cast<audio::SoundSource *>(static_cast<audio::CALSound *>(&snd));
		if(m_bAllSounds == false && (m_bAllWorldSounds == false || snd.IsRelative() == true) && (snd.GetType() & m_types) == audio::ALSoundType::Generic) {
			DetachSoundSource(alSnd);
			continue;
		}
		if(m_bApplyGlobal == false) {
			if(m_bAffectRelative == false && snd.IsRelative() == true)
				continue;
			auto posSnd = alSnd->GetWorldPosition();

			auto d = uvec::length_sqr(posSnd - pos);
			if(d > radiusOuterSqr)
				DetachSoundSource(alSnd);
			else {
				d = math::sqrt(d);
				auto intensity = math::clamp(d / m_kvInnerRadius, 0.f, 1.f);
				UpdateSoundSource(alSnd, intensity);
			}
		}
		else {
			if(m_bAffectRelative == false && snd.IsRelative() == true)
				continue;
			UpdateSoundSource(alSnd, 1.f);
		}
	}
}
void CBaseSoundDspComponent::ReceiveData(NetPacket &packet)
{
	m_kvDsp = packet->ReadString();
	m_kvInnerRadius = packet->Read<float>();
	m_kvOuterRadius = packet->Read<float>();
	auto bTurnedOn = packet->Read<bool>();
	auto gain = packet->Read<float>();
	SetGain(gain);

	auto spawnFlags = GetEntity().GetSpawnFlags();
	m_bAffectRelative = (spawnFlags & math::to_integral(SpawnFlags::AffectRelative));
	m_bApplyGlobal = (spawnFlags & math::to_integral(SpawnFlags::ApplyGlobally));
	m_types = GetTargetSoundTypes();
	m_bAllWorldSounds = (static_cast<SpawnFlags>(spawnFlags) & (SpawnFlags::World | SpawnFlags::All)) != SpawnFlags::None;
	m_bAllSounds = (static_cast<SpawnFlags>(spawnFlags) & SpawnFlags::All) != SpawnFlags::None;

	auto pToggleComponent = GetEntity().GetComponent<CToggleComponent>();
	if(pToggleComponent.valid())
		pToggleComponent->SetTurnedOn(bTurnedOn);
}
void CBaseSoundDspComponent::OnEntitySpawn()
{
	BaseEnvSoundDspComponent::OnEntitySpawn();
	if(m_kvDsp.empty() == false)
		m_dsp = get_cgame()->GetAuxEffect(m_kvDsp);
}
util::EventReply CBaseSoundDspComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvSoundDspComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseToggleComponent::EVENT_ON_TURN_OFF)
		DetachAllSoundSources();
	return util::EventReply::Unhandled;
}
audio::ALSoundType CBaseSoundDspComponent::GetTargetSoundTypes() const
{
	auto types = audio::ALSoundType::Generic;
	auto spawnFlags = static_cast<SpawnFlags>(GetEntity().GetSpawnFlags());
	if((spawnFlags & SpawnFlags::Effects) != SpawnFlags::None)
		types |= audio::ALSoundType::Effect;
	if((spawnFlags & SpawnFlags::Music) != SpawnFlags::None)
		types |= audio::ALSoundType::Music;
	if((spawnFlags & SpawnFlags::Voices) != SpawnFlags::None)
		types |= audio::ALSoundType::Voice;
	if((spawnFlags & SpawnFlags::Weapons) != SpawnFlags::None)
		types |= audio::ALSoundType::Weapon;
	if((spawnFlags & SpawnFlags::NPCs) != SpawnFlags::None)
		types |= audio::ALSoundType::NPC;
	if((spawnFlags & SpawnFlags::Players) != SpawnFlags::None)
		types |= audio::ALSoundType::Player;
	if((spawnFlags & SpawnFlags::Vehicles) != SpawnFlags::None)
		types |= audio::ALSoundType::Vehicle;
	if((spawnFlags & SpawnFlags::Physics) != SpawnFlags::None)
		types |= audio::ALSoundType::Physics;
	if((spawnFlags & SpawnFlags::Environment) != SpawnFlags::None)
		types |= audio::ALSoundType::Environment;
	if((spawnFlags & SpawnFlags::GUI) != SpawnFlags::None)
		types |= audio::ALSoundType::GUI;
	if((spawnFlags & SpawnFlags::All) != SpawnFlags::None)
		types |= audio::ALSoundType::All;
	return types;
}
Bool CBaseSoundDspComponent::ReceiveNetEvent(UInt32 eventId, NetPacket &p)
{
	if(eventId == m_netEvSetGain) {
		auto gain = p->Read<float>();
		SetGain(gain);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, p);
	return true;
}
std::vector<std::pair<audio::SoundSourceHandle, uint32_t>>::iterator CBaseSoundDspComponent::FindSoundSource(audio::SoundSource &src)
{
	return std::find_if(m_affectedSounds.begin(), m_affectedSounds.end(), [&src](const std::pair<audio::SoundSourceHandle, uint32_t> &pair) { return (pair.first.get() == &src) ? true : false; });
}
void CBaseSoundDspComponent::UpdateSoundSource(audio::SoundSource &src, float gain)
{
	if(m_dsp == nullptr)
		return;
	gain *= GetGain();
	auto it = FindSoundSource(src);
	if(it != m_affectedSounds.end()) {
		src->SetEffectGain(it->second, gain);
		return;
	}
	uint32_t slotId;
	if(src->AddEffect(*m_dsp, slotId, gain) == false)
		return;
	m_affectedSounds.push_back({src.GetHandle(), slotId});
}
void CBaseSoundDspComponent::DetachSoundSource(audio::SoundSource &src)
{
	auto it = FindSoundSource(src);
	if(it == m_affectedSounds.end())
		return;
	src->RemoveEffect(it->second);
	m_affectedSounds.erase(it);
}
void CBaseSoundDspComponent::DetachAllSoundSources()
{
	for(auto &pair : m_affectedSounds) {
		if(pair.first.IsValid() == false)
			continue;
		auto &src = *static_cast<audio::SoundSource *>(static_cast<audio::CALSound *>(pair.first.get()));
		src->RemoveEffect(pair.second);
	}
	m_affectedSounds.clear();
}
void CSoundDspComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDsp::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspComponent>();
}
