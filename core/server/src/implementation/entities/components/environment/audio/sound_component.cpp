// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.audio.sound;

import :audio;

using namespace pragma;

void SSoundComponent::Initialize() { BaseEnvSoundComponent::Initialize(); }
void SSoundComponent::OnSoundCreated(audio::ALSound &snd)
{
	BaseEnvSoundComponent::OnSoundCreated(snd);
	auto pMapComponent = GetEntity().GetComponent<MapComponent>();
	dynamic_cast<audio::SALSound &>(snd).SetEntityMapIndex(pMapComponent.valid() ? pMapComponent->GetMapIndex() : 0u);
}
void SSoundComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_kvMaxDist);
	auto idx = (m_sound != nullptr) ? m_sound->GetIndex() : std::numeric_limits<uint32_t>::max();
	packet->Write<uint32_t>(idx);
}

void SSoundComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void EnvSound::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundComponent>();
}
