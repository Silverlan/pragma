/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_flanger.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <sharedutils/util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound_dsp_flanger, EnvSoundDspFlanger);

bool SSoundDspFlangerComponent::OnSetKeyValue(const std::string &key, const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key, val))
		return true;
	if(ustring::compare<std::string>(key, "waveform", false))
		m_kvWaveform = util::to_int(val);
	else if(ustring::compare<std::string>(key, "phase", false))
		m_kvPhase = util::to_int(val);
	else if(ustring::compare<std::string>(key, "rate", false))
		m_kvRate = util::to_float(val);
	else if(ustring::compare<std::string>(key, "depth", false))
		m_kvDepth = util::to_float(val);
	else if(ustring::compare<std::string>(key, "feedback", false))
		m_kvFeedback = util::to_float(val);
	else if(ustring::compare<std::string>(key, "delay", false))
		m_kvDelay = util::to_float(val);
	else
		return false;
	return true;
}

void SSoundDspFlangerComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<int>(m_kvWaveform);
	packet->Write<int>(m_kvPhase);
	packet->Write<float>(m_kvRate);
	packet->Write<float>(m_kvDepth);
	packet->Write<float>(m_kvFeedback);
	packet->Write<float>(m_kvDelay);
}
void SSoundDspFlangerComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDspFlanger::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspFlangerComponent>();
}
