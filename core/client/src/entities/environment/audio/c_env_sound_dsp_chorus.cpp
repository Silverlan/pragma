/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_chorus.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/c_engine.h"
#include <alsoundsystem.hpp>
#include "pragma/audio/c_engine_sound.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <alsoundsystem.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;

LINK_ENTITY_TO_CLASS(env_sound_dsp_chorus, CEnvSoundDspChorus);

void CSoundDspChorusComponent::ReceiveData(NetPacket &packet)
{
	m_kvWaveform = packet->Read<int>();
	m_kvPhase = packet->Read<int>();
	m_kvRate = packet->Read<float>();
	m_kvDepth = packet->Read<float>();
	m_kvFeedback = packet->Read<float>();
	m_kvDelay = packet->Read<float>();
}

void CSoundDspChorusComponent::OnEntitySpawn()
{
	CBaseSoundDspComponent::OnEntitySpawn();
	//CBaseSoundDspComponent::OnSpawn(); // Not calling CBaseSoundDspComponent::OnSpawn() to skip the dsp effect lookup
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	al::EfxChorusProperties props {};
	props.iWaveform = m_kvWaveform;
	props.iPhase = m_kvPhase;
	props.flRate = m_kvRate;
	props.flDepth = m_kvDepth;
	props.flFeedback = m_kvFeedback;
	props.flDelay = m_kvDelay;
	m_dsp = soundSys->CreateEffect(props);
}
void CSoundDspChorusComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDspChorus::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspChorusComponent>();
}
