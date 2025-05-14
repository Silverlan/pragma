/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_distortion.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <sharedutils/util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_sound_dsp_distortion, EnvSoundDspDistortion);

bool SSoundDspDistortionComponent::OnSetKeyValue(const std::string &key, const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key, val))
		return true;
	if(ustring::compare<std::string>(key, "edge", false))
		m_kvEdge = util::to_float(val);
	else if(ustring::compare<std::string>(key, "gain", false))
		m_kvGain = util::to_float(val);
	else if(ustring::compare<std::string>(key, "lowpass_cutoff", false))
		m_kvLowpassCutoff = util::to_float(val);
	else if(ustring::compare<std::string>(key, "eqcenter", false))
		m_kvEqCenter = util::to_float(val);
	else if(ustring::compare<std::string>(key, "eqbandwith", false))
		m_kvEqBandwidth = util::to_float(val);
	else
		return false;
	return true;
}

void SSoundDspDistortionComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_kvEdge);
	packet->Write<float>(m_kvGain);
	packet->Write<float>(m_kvLowpassCutoff);
	packet->Write<float>(m_kvEqCenter);
	packet->Write<float>(m_kvEqBandwidth);
}
void SSoundDspDistortionComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDspDistortion::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspDistortionComponent>();
}
