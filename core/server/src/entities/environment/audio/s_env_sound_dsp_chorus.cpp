// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_chorus.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/util.h>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound_dsp_chorus, EnvSoundDspChorus);

bool SSoundDspChorusComponent::OnSetKeyValue(const std::string &key, const std::string &val)
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

void SSoundDspChorusComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<int>(m_kvWaveform);
	packet->Write<int>(m_kvPhase);
	packet->Write<float>(m_kvRate);
	packet->Write<float>(m_kvDepth);
	packet->Write<float>(m_kvFeedback);
	packet->Write<float>(m_kvDelay);
}
void SSoundDspChorusComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDspChorus::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspChorusComponent>();
}
