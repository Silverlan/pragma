// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.server;
import :entities.components.audio.dsp.chorus;

using namespace pragma;

bool SSoundDspChorusComponent::OnSetKeyValue(const std::string &key, const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key, val))
		return true;
	if(pragma::string::compare<std::string>(key, "waveform", false))
		m_kvWaveform = pragma::util::to_int(val);
	else if(pragma::string::compare<std::string>(key, "phase", false))
		m_kvPhase = pragma::util::to_int(val);
	else if(pragma::string::compare<std::string>(key, "rate", false))
		m_kvRate = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "depth", false))
		m_kvDepth = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "feedback", false))
		m_kvFeedback = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "delay", false))
		m_kvDelay = pragma::util::to_float(val);
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
void SSoundDspChorusComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDspChorus::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspChorusComponent>();
}
