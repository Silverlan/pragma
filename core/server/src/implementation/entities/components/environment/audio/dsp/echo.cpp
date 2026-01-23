// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.audio.dsp.echo;

using namespace pragma;

bool SSoundDspEchoComponent::OnSetKeyValue(const std::string &key, const std::string &val)
{
	if(SBaseSoundDspComponent::OnSetKeyValue(key, val))
		return true;
	if(pragma::string::compare<std::string>(key, "delay", false))
		m_kvDelay = util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "lrdelay", false))
		m_kvLRDelay = util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "damping", false))
		m_kvDamping = util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "feedback", false))
		m_kvFeedback = util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "spread", false))
		m_kvSpread = util::to_float(val);
	else
		return false;
	return true;
}

void SSoundDspEchoComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_kvDelay);
	packet->Write<float>(m_kvLRDelay);
	packet->Write<float>(m_kvDamping);
	packet->Write<float>(m_kvFeedback);
	packet->Write<float>(m_kvSpread);
}
void SSoundDspEchoComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////////

void EnvSoundDspEcho::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundDspEchoComponent>();
}
