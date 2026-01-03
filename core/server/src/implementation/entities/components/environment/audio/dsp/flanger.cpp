// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.audio.dsp.flanger;

using namespace pragma;

bool SSoundDspFlangerComponent::OnSetKeyValue(const std::string &key, const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key, val))
		return true;
	if(pragma::string::compare<std::string>(key, "waveform", false))
		m_kvWaveform = util::to_int(val);
	else if(pragma::string::compare<std::string>(key, "phase", false))
		m_kvPhase = util::to_int(val);
	else if(pragma::string::compare<std::string>(key, "rate", false))
		m_kvRate = util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "depth", false))
		m_kvDepth = util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "feedback", false))
		m_kvFeedback = util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "delay", false))
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
void SSoundDspFlangerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDspFlanger::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspFlangerComponent>();
}
