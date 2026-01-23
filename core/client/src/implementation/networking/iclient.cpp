// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :networking.iclient;

std::optional<pragma::networking::IPAddress> pragma::networking::IClient::GetIPAddress() const
{
	auto ip = GetIP();
	auto port = GetLocalTCPPort();
	if(ip.has_value() == false || port.has_value() == false)
		return {};
	return IPAddress {*ip, *port};
}

void pragma::networking::IClient::SetEventInterface(const ClientEventInterface &eventHandler) { m_eventInterface = eventHandler; }
const pragma::networking::ClientEventInterface &pragma::networking::IClient::GetEventInterface() const { return m_eventInterface; }

void pragma::networking::IClient::HandlePacket(NetPacket &packet)
{
	if(m_eventInterface.handlePacket)
		m_eventInterface.handlePacket(packet);
}
void pragma::networking::IClient::OnPacketSent(Protocol protocol, NetPacket &packet)
{
	if(m_eventInterface.onPacketSent)
		m_eventInterface.onPacketSent(protocol, packet);
}
void pragma::networking::IClient::OnConnected()
{
	if(m_eventInterface.onConnected)
		m_eventInterface.onConnected();
}
void pragma::networking::IClient::OnDisconnected()
{
	if(m_eventInterface.onDisconnected)
		m_eventInterface.onDisconnected();
}
void pragma::networking::IClient::OnConnectionClosed()
{
	if(m_eventInterface.onConnectionClosed)
		m_eventInterface.onConnectionClosed();
}
