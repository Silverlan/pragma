/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_eaxreverb.h"
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

LINK_ENTITY_TO_CLASS(env_sound_dsp_eaxreverb, CEnvSoundDspEAXReverb);

void CSoundDspEAXReverbComponent::ReceiveData(NetPacket &packet)
{
	m_kvDensity = packet->Read<float>();
	m_kvDiffusion = packet->Read<float>();
	m_kvGain = packet->Read<float>();
	m_kvGainHF = packet->Read<float>();
	m_kvGainLF = packet->Read<float>();
	m_kvDecay = packet->Read<float>();
	m_kvDecayHF = packet->Read<float>();
	m_kvDecayLF = packet->Read<float>();
	m_kvDecayHFLimit = packet->Read<int>();
	m_kvReflectionsGain = packet->Read<float>();
	m_kvReflectionsDelay = packet->Read<float>();
	m_kvReflectionsPan = nwm::read_vector(packet);
	m_kvLateGain = packet->Read<float>();
	m_kvLateDelay = packet->Read<float>();
	m_kvLatePan = nwm::read_vector(packet);
	m_kvEchoTime = packet->Read<float>();
	m_kvEchoDepth = packet->Read<float>();
	m_kvModTime = packet->Read<float>();
	m_kvModDepth = packet->Read<float>();
	m_kvRefHF = packet->Read<float>();
	m_kvRefLF = packet->Read<float>();
	m_kvRoomRolloff = packet->Read<float>();
	m_kvAirAbsorpGainHF = packet->Read<float>();
}

void CSoundDspEAXReverbComponent::OnEntitySpawn()
{
	//BaseEnvSoundDspEAXReverb::OnEntitySpawn(); // Not calling BaseEnvSoundDspEAXReverb::OnEntitySpawn() to skip the dsp effect lookup
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	al::EfxEaxReverbProperties props {};
	props.flDensity = m_kvDensity;
	props.flDiffusion = m_kvDiffusion;
	props.flGain = m_kvGain;
	props.flGainHF = m_kvGainHF;
	props.flGainLF = m_kvGainLF;
	props.flDecayTime = m_kvDecay;
	props.flDecayHFRatio = m_kvDecayHF;
	props.flDecayLFRatio = m_kvDecayLF;
	props.iDecayHFLimit = m_kvDecayHFLimit;
	props.flReflectionsGain = m_kvReflectionsGain;
	props.flReflectionsDelay = m_kvReflectionsDelay;
	props.flReflectionsPan = {m_kvReflectionsPan.x, m_kvReflectionsPan.y, m_kvReflectionsPan.z};
	props.flLateReverbGain = m_kvLateGain;
	props.flLateReverbDelay = m_kvLateDelay;
	props.flLateReverbPan = {m_kvLatePan.x, m_kvLatePan.y, m_kvLatePan.z};
	props.flEchoTime = m_kvEchoTime;
	props.flEchoDepth = m_kvEchoDepth;
	props.flModulationTime = m_kvModTime;
	props.flModulationDepth = m_kvModDepth;
	props.flHFReference = m_kvRefHF;
	props.flLFReference = m_kvRefLF;
	props.flRoomRolloffFactor = m_kvRoomRolloff;
	props.flAirAbsorptionGainHF = m_kvAirAbsorpGainHF;
	m_dsp = soundSys->CreateEffect(props);
}
void CSoundDspEAXReverbComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////////////

void CEnvSoundDspEAXReverb::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundDspEAXReverbComponent>();
}
