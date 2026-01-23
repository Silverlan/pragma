// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.audio.dsp.echo;
import :engine;

using namespace pragma;

void CSoundDspEchoComponent::ReceiveData(NetPacket &packet)
{
	m_kvDelay = packet->Read<float>();
	m_kvLRDelay = packet->Read<float>();
	m_kvDamping = packet->Read<float>();
	m_kvFeedback = packet->Read<float>();
	m_kvSpread = packet->Read<float>();
}

void CSoundDspEchoComponent::OnEntitySpawn()
{
	//BaseEnvSoundDspEcho::OnEntitySpawn(); // Not calling BaseEnvSoundDspEcho::OnEntitySpawn() to skip the dsp effect lookup
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	audio::EfxEchoProperties props {};
	props.flDelay = m_kvDelay;
	props.flLRDelay = m_kvLRDelay;
	props.flDamping = m_kvDamping;
	props.flFeedback = m_kvFeedback;
	props.flSpread = m_kvSpread;
	m_dsp = soundSys->CreateEffect(props);
}
void CSoundDspEchoComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDspEcho::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspEchoComponent>();
}
