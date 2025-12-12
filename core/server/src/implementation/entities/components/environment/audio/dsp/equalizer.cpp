// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.server;
import :entities.components.audio.dsp.equalizer;

using namespace pragma;

bool SSoundDspEqualizerComponent::OnSetKeyValue(const std::string &key, const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key, val))
		return true;
	if(pragma::string::compare<std::string>(key, "low_gain", false))
		m_lowGain = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "low_cutoff", false))
		m_lowCutoff = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "mid1_gain", false))
		m_mid1Gain = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "mid1_center", false))
		m_mid1Center = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "mid1_width", false))
		m_mid1Width = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "mid2_gain", false))
		m_mid2Gain = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "mid2_center", false))
		m_mid2Center = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "mid2_width", false))
		m_mid2Width = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "high_gain", false))
		m_highGain = pragma::util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "high_cutoff", false))
		m_highCutoff = pragma::util::to_float(val);
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
void SSoundDspEqualizerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDspEqualizer::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspEqualizerComponent>();
}
