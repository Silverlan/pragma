/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_distortion.h"
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

LINK_ENTITY_TO_CLASS(env_sound_dsp_distortion, CEnvSoundDspDistortion);

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
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	al::EfxDistortionProperties props {};
	props.flEdge = m_kvEdge;
	props.flGain = m_kvGain;
	props.flLowpassCutoff = m_kvLowpassCutoff;
	props.flEQCenter = m_kvEqCenter;
	props.flEQBandwidth = m_kvEqBandwidth;
	m_dsp = soundSys->CreateEffect(props);
}
void CSoundDspDistortionComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDspDistortion::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspDistortionComponent>();
}
