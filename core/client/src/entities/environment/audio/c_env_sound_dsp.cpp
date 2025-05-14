/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/basetoggle.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

LINK_ENTITY_TO_CLASS(env_sound_dsp, CEnvSoundDsp);

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
	auto radiusInnerSqr = umath::pow2(m_kvInnerRadius);
	auto radiusOuterSqr = umath::pow2(m_kvOuterRadius);
	auto &pos = pTrComponent->GetPosition();
	auto &sounds = client->GetSounds();
	for(auto &rsnd : sounds) {
		auto &snd = rsnd.get();
		if(snd.IsPlaying() == false)
			continue;
		auto &alSnd = *static_cast<al::SoundSource *>(static_cast<CALSound *>(&snd));
		if(m_bAllSounds == false && (m_bAllWorldSounds == false || snd.IsRelative() == true) && (snd.GetType() & m_types) == ALSoundType::Generic) {
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
				d = umath::sqrt(d);
				auto intensity = umath::clamp(d / m_kvInnerRadius, 0.f, 1.f);
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
	m_bAffectRelative = (spawnFlags & umath::to_integral(SpawnFlags::AffectRelative));
	m_bApplyGlobal = (spawnFlags & umath::to_integral(SpawnFlags::ApplyGlobally));
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
		m_dsp = c_game->GetAuxEffect(m_kvDsp);
}
util::EventReply CBaseSoundDspComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvSoundDspComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_OFF)
		DetachAllSoundSources();
	return util::EventReply::Unhandled;
}
ALSoundType CBaseSoundDspComponent::GetTargetSoundTypes() const
{
	auto types = ALSoundType::Generic;
	auto spawnFlags = static_cast<SpawnFlags>(GetEntity().GetSpawnFlags());
	if((spawnFlags & SpawnFlags::Effects) != SpawnFlags::None)
		types |= ALSoundType::Effect;
	if((spawnFlags & SpawnFlags::Music) != SpawnFlags::None)
		types |= ALSoundType::Music;
	if((spawnFlags & SpawnFlags::Voices) != SpawnFlags::None)
		types |= ALSoundType::Voice;
	if((spawnFlags & SpawnFlags::Weapons) != SpawnFlags::None)
		types |= ALSoundType::Weapon;
	if((spawnFlags & SpawnFlags::NPCs) != SpawnFlags::None)
		types |= ALSoundType::NPC;
	if((spawnFlags & SpawnFlags::Players) != SpawnFlags::None)
		types |= ALSoundType::Player;
	if((spawnFlags & SpawnFlags::Vehicles) != SpawnFlags::None)
		types |= ALSoundType::Vehicle;
	if((spawnFlags & SpawnFlags::Physics) != SpawnFlags::None)
		types |= ALSoundType::Physics;
	if((spawnFlags & SpawnFlags::Environment) != SpawnFlags::None)
		types |= ALSoundType::Environment;
	if((spawnFlags & SpawnFlags::GUI) != SpawnFlags::None)
		types |= ALSoundType::GUI;
	if((spawnFlags & SpawnFlags::All) != SpawnFlags::None)
		types |= ALSoundType::All;
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
std::vector<std::pair<al::SoundSourceHandle, uint32_t>>::iterator CBaseSoundDspComponent::FindSoundSource(al::SoundSource &src)
{
	return std::find_if(m_affectedSounds.begin(), m_affectedSounds.end(), [&src](const std::pair<al::SoundSourceHandle, uint32_t> &pair) { return (pair.first.get() == &src) ? true : false; });
}
void CBaseSoundDspComponent::UpdateSoundSource(al::SoundSource &src, float gain)
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
void CBaseSoundDspComponent::DetachSoundSource(al::SoundSource &src)
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
		auto &src = *static_cast<al::SoundSource *>(static_cast<CALSound *>(pair.first.get()));
		src->RemoveEffect(pair.second);
	}
	m_affectedSounds.clear();
}
void CSoundDspComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDsp::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspComponent>();
}
