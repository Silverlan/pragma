// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.audio.dsp.equalizer;
import :engine;

using namespace pragma;

void CSoundDspEqualizerComponent::ReceiveData(NetPacket &packet)
{
	m_lowGain = packet->Read<float>();
	m_lowCutoff = packet->Read<float>();
	m_mid1Gain = packet->Read<float>();
	m_mid1Center = packet->Read<float>();
	m_mid1Width = packet->Read<float>();
	m_mid2Gain = packet->Read<float>();
	m_mid2Center = packet->Read<float>();
	m_mid2Width = packet->Read<float>();
	m_highGain = packet->Read<float>();
	m_highCutoff = packet->Read<float>();
}

void CSoundDspEqualizerComponent::OnEntitySpawn()
{
	//BaseEnvSoundDspEqualizer::OnEntitySpawn(); // Not calling BaseEnvSoundDspEqualizer::OnEntitySpawn() to skip the dsp effect lookup
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	audio::EfxEqualizer props {};
	props.flLowGain = m_lowGain;
	props.flLowCutoff = m_lowCutoff;
	props.flMid1Gain = m_mid1Gain;
	props.flMid1Center = m_mid1Center;
	props.flMid1Width = m_mid1Width;
	props.flMid2Gain = m_mid2Gain;
	props.flMid2Center = m_mid2Center;
	props.flMid2Width = m_mid2Width;
	props.flHighGain = m_highGain;
	props.flHighCutoff = m_highCutoff;
	m_dsp = soundSys->CreateEffect(props);
}
void CSoundDspEqualizerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDspEqualizer::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspEqualizerComponent>();
}
