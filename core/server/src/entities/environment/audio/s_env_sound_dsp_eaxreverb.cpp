/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_eaxreverb.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <sharedutils/util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound_dsp_eaxreverb, EnvSoundDspEAXReverb);

bool SSoundDspEAXReverbComponent::OnSetKeyValue(const std::string &key, const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key, val))
		return true;
	if(ustring::compare<std::string>(key, "density", false))
		m_kvDensity = util::to_float(val);
	else if(ustring::compare<std::string>(key, "diffusion", false))
		m_kvDiffusion = util::to_float(val);
	else if(ustring::compare<std::string>(key, "gain", false))
		m_kvGain = util::to_float(val);
	else if(ustring::compare<std::string>(key, "gain_hf", false))
		m_kvGainHF = util::to_float(val);
	else if(ustring::compare<std::string>(key, "gain_lf", false))
		m_kvGainLF = util::to_float(val);
	else if(ustring::compare<std::string>(key, "decay", false))
		m_kvDecay = util::to_float(val);
	else if(ustring::compare<std::string>(key, "decay_hf", false))
		m_kvDecayHF = util::to_float(val);
	else if(ustring::compare<std::string>(key, "decay_lf", false))
		m_kvDecayLF = util::to_float(val);
	else if(ustring::compare<std::string>(key, "decay_hf_limit", false))
		m_kvDecayHFLimit = util::to_int(val);
	else if(ustring::compare<std::string>(key, "reflections_gain", false))
		m_kvReflectionsGain = util::to_float(val);
	else if(ustring::compare<std::string>(key, "reflections_delay", false))
		m_kvReflectionsDelay = util::to_float(val);
	else if(ustring::compare<std::string>(key, "reflections_pan", false))
		m_kvReflectionsPan = uvec::create(val);
	else if(ustring::compare<std::string>(key, "late_gain", false))
		m_kvLateGain = util::to_float(val);
	else if(ustring::compare<std::string>(key, "late_delay", false))
		m_kvLateDelay = util::to_float(val);
	else if(ustring::compare<std::string>(key, "late_pan", false))
		m_kvLatePan = uvec::create(val);
	else if(ustring::compare<std::string>(key, "echo_time", false))
		m_kvEchoTime = util::to_float(val);
	else if(ustring::compare<std::string>(key, "echo_depth", false))
		m_kvEchoDepth = util::to_float(val);
	else if(ustring::compare<std::string>(key, "mod_time", false))
		m_kvModTime = util::to_float(val);
	else if(ustring::compare<std::string>(key, "mod_depth", false))
		m_kvModDepth = util::to_float(val);
	else if(ustring::compare<std::string>(key, "reference_hf", false))
		m_kvRefHF = util::to_float(val);
	else if(ustring::compare<std::string>(key, "reference_lf", false))
		m_kvRefLF = util::to_float(val);
	else if(ustring::compare<std::string>(key, "room_rolloff", false))
		m_kvRoomRolloff = util::to_float(val);
	else if(ustring::compare<std::string>(key, "air_absorp_gain_hf", false))
		m_kvAirAbsorpGainHF = util::to_float(val);
	else
		return false;
	return true;
}

void SSoundDspEAXReverbComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_kvDensity);
	packet->Write<float>(m_kvDiffusion);
	packet->Write<float>(m_kvGain);
	packet->Write<float>(m_kvGainHF);
	packet->Write<float>(m_kvGainLF);
	packet->Write<float>(m_kvDecay);
	packet->Write<float>(m_kvDecayHF);
	packet->Write<float>(m_kvDecayLF);
	packet->Write<int>(m_kvDecayHFLimit);
	packet->Write<float>(m_kvReflectionsGain);
	packet->Write<float>(m_kvReflectionsDelay);
	nwm::write_vector(packet, m_kvReflectionsPan);
	packet->Write<float>(m_kvLateGain);
	packet->Write<float>(m_kvLateDelay);
	nwm::write_vector(packet, m_kvLatePan);
	packet->Write<float>(m_kvEchoTime);
	packet->Write<float>(m_kvEchoDepth);
	packet->Write<float>(m_kvModTime);
	packet->Write<float>(m_kvModDepth);
	packet->Write<float>(m_kvRefHF);
	packet->Write<float>(m_kvRefLF);
	packet->Write<float>(m_kvRoomRolloff);
	packet->Write<float>(m_kvAirAbsorpGainHF);
}
void SSoundDspEAXReverbComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDspEAXReverb::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspEAXReverbComponent>();
}
