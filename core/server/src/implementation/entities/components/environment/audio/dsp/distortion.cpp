// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/networking/nwm_util.h>
#include <sharedutils/util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.audio.dsp.distortion;

using namespace pragma;

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
