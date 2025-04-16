/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_equalizer.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <sharedutils/util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound_dsp_equalizer, EnvSoundDspEqualizer);

bool SSoundDspEqualizerComponent::OnSetKeyValue(const std::string &key, const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key, val))
		return true;
	if(ustring::compare<std::string>(key, "low_gain", false))
		m_lowGain = util::to_float(val);
	else if(ustring::compare<std::string>(key, "low_cutoff", false))
		m_lowCutoff = util::to_float(val);
	else if(ustring::compare<std::string>(key, "mid1_gain", false))
		m_mid1Gain = util::to_float(val);
	else if(ustring::compare<std::string>(key, "mid1_center", false))
		m_mid1Center = util::to_float(val);
	else if(ustring::compare<std::string>(key, "mid1_width", false))
		m_mid1Width = util::to_float(val);
	else if(ustring::compare<std::string>(key, "mid2_gain", false))
		m_mid2Gain = util::to_float(val);
	else if(ustring::compare<std::string>(key, "mid2_center", false))
		m_mid2Center = util::to_float(val);
	else if(ustring::compare<std::string>(key, "mid2_width", false))
		m_mid2Width = util::to_float(val);
	else if(ustring::compare<std::string>(key, "high_gain", false))
		m_highGain = util::to_float(val);
	else if(ustring::compare<std::string>(key, "high_cutoff", false))
		m_highCutoff = util::to_float(val);
	else
		return false;
	return true;
}

void SSoundDspEqualizerComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_lowGain);
	packet->Write<float>(m_lowCutoff);
	packet->Write<float>(m_mid1Gain);
	packet->Write<float>(m_mid1Center);
	packet->Write<float>(m_mid1Width);
	packet->Write<float>(m_mid2Gain);
	packet->Write<float>(m_mid2Center);
	packet->Write<float>(m_mid2Width);
	packet->Write<float>(m_highGain);
	packet->Write<float>(m_highCutoff);
}
void SSoundDspEqualizerComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDspEqualizer::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspEqualizerComponent>();
}
