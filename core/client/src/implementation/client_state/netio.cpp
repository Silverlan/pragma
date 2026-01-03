// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :client_state;

#undef GetMessage

void pragma::ClientState::SendPacket(const std::string &name, NetPacket &packet, networking::Protocol protocol)
{
	auto ID = GetServerMessageID(name);
	if(ID == 0 || m_client == nullptr)
		return;
	packet.SetMessageID(ID);
	networking::Error err;
	if(m_client->SendPacket(protocol, packet, err) == false)
		Con::CWAR << "Unable to send packet '" << name << "': " << err.GetMessage() << Con::endl;
}
void pragma::ClientState::SendPacket(const std::string &name, NetPacket &packet) { SendPacket(name, packet, networking::Protocol::FastUnreliable); }
void pragma::ClientState::SendPacket(const std::string &name, networking::Protocol protocol)
{
	NetPacket packet {};
	SendPacket(name, packet, protocol);
}
