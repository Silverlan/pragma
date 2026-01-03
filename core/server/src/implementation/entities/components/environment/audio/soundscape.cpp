// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.audio.soundscape;

using namespace pragma;

void SSoundScapeComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSoundScape);
	packet->Write<float>(m_kvRadius);
	packet->Write<unsigned int>(CUInt32(m_positions.size()));
	std::unordered_map<unsigned int, std::string>::iterator it;
	for(it = m_positions.begin(); it != m_positions.end(); it++) {
		packet->Write<unsigned int>(it->first);
		packet->WriteString(it->second);
	}
}

void SSoundScapeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////

void EnvSoundScape::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSoundScapeComponent>();
}
