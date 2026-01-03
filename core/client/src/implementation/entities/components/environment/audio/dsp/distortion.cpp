// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.audio.dsp.distortion;
import :engine;

using namespace pragma;

void CSoundDspDistortionComponent::ReceiveData(NetPacket &packet)
{
	m_kvEdge = packet->Read<float>();
	m_kvGain = packet->Read<float>();
	m_kvLowpassCutoff = packet->Read<float>();
	m_kvEqCenter = packet->Read<float>();
	m_kvEqBandwidth = packet->Read<float>();
}

void CSoundDspDistortionComponent::OnEntitySpawn()
{
	//BaseEnvSoundDspDistortion::OnEntitySpawn(); // Not calling BaseEnvSoundDspDistortion::OnEntitySpawn() to skip the dsp effect lookup
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	audio::EfxDistortionProperties props {};
	props.flEdge = m_kvEdge;
	props.flGain = m_kvGain;
	props.flLowpassCutoff = m_kvLowpassCutoff;
	props.flEQCenter = m_kvEqCenter;
	props.flEQBandwidth = m_kvEqBandwidth;
	m_dsp = soundSys->CreateEffect(props);
}
void CSoundDspDistortionComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDspDistortion::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspDistortionComponent>();
}
