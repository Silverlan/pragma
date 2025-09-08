// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/audio/alsoundscript.h>
#include "pragma/clientstate/clientstate.h"
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.audio.sound;

import pragma.client.entities.components;

using namespace pragma;

extern DLLCLIENT ClientState *client;

static void apply_sound_identifier(ALSound &snd, const std::string &name)
{
	if(snd.IsSoundScript() == false) {
		static_cast<CALSound &>(snd)->SetIdentifier(name);
		return;
	}
	auto *sndScript = dynamic_cast<ALSoundScript *>(&snd);
	if(sndScript == nullptr)
		return;
	sndScript->AddCallback("OnSoundCreated", FunctionCallback<void, ALSound *>::Create([name](ALSound *snd) { apply_sound_identifier(*snd, name); }));
}

void CSoundComponent::OnEntitySpawn()
{
	BaseEnvSoundComponent::OnEntitySpawn();
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	auto &ent = GetEntity();
	auto pNameComponent = ent.GetComponent<pragma::CNameComponent>();
	auto name = m_steamAudioIdentifier = pNameComponent.valid() ? pNameComponent->GetName() : "";
	if(name.empty()) {
		auto pMapComponent = ent.GetComponent<pragma::MapComponent>();
		if(pMapComponent.expired())
			return;
		name = "world_sound" + std::to_string(pMapComponent->GetMapIndex());
	}
	if(m_wpSound.expired() == false)
		apply_sound_identifier(*m_wpSound.lock(), name);
#endif
}

void CSoundComponent::ReceiveData(NetPacket &packet)
{
	m_kvMaxDistance = packet->Read<float>();
	auto soundIdx = packet->Read<uint32_t>();
	auto snd = client->GetSoundByIndex(soundIdx);
	if(snd != nullptr) {
		snd->SetSource(&GetEntity());
		m_wpSound = snd;
	}
}

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
const std::string &CSoundComponent::GetSteamAudioIdentifier() const { return m_steamAudioIdentifier; }
#endif

float CSoundComponent::GetMaxDistance() const { return m_kvMaxDistance; }
void CSoundComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CEnvSound::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundComponent>();
}
