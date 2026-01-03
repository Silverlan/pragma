// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.audio.dsp.flanger;
import :engine;

using namespace pragma;

void CSoundDspFlangerComponent::ReceiveData(NetPacket &packet)
{
	m_kvWaveform = packet->Read<int>();
	m_kvPhase = packet->Read<int>();
	m_kvRate = packet->Read<float>();
	m_kvDepth = packet->Read<float>();
	m_kvFeedback = packet->Read<float>();
	m_kvDelay = packet->Read<float>();
}

void CSoundDspFlangerComponent::OnEntitySpawn()
{
	//BaseEnvSoundDspFlanger::OnEntitySpawn(); // Not calling BaseEnvSoundDspFlanger::OnEntitySpawn() to skip the dsp effect lookup
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	audio::EfxFlangerProperties props {};
	props.iWaveform = m_kvWaveform;
	props.iPhase = m_kvPhase;
	props.flRate = m_kvRate;
	props.flDepth = m_kvDepth;
	props.flFeedback = m_kvFeedback;
	props.flDelay = m_kvDelay;
	m_dsp = soundSys->CreateEffect(props);
}
void CSoundDspFlangerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDspFlanger::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspFlangerComponent>();
}
