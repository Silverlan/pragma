// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <alsoundsystem.hpp>
#include "alsound_effect.hpp"
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <alsoundsystem.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.audio.dsp.flanger;

import pragma.client.engine;

using namespace pragma;

extern CEngine *c_engine;

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
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	al::EfxFlangerProperties props {};
	props.iWaveform = m_kvWaveform;
	props.iPhase = m_kvPhase;
	props.flRate = m_kvRate;
	props.flDepth = m_kvDepth;
	props.flFeedback = m_kvFeedback;
	props.flDelay = m_kvDelay;
	m_dsp = soundSys->CreateEffect(props);
}
void CSoundDspFlangerComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDspFlanger::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspFlangerComponent>();
}
