/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/networking/iclient.hpp"

std::optional<pragma::networking::IPAddress> pragma::networking::IClient::GetIPAddress() const
{
	auto ip = GetIP();
	auto port = GetLocalTCPPort();
	if(ip.has_value() == false || port.has_value() == false)
		return {};
	return pragma::networking::IPAddress{*ip,*port};
}

void pragma::networking::IClient::SetEventInterface(const ClientEventInterface &eventHandler) {m_eventInterface = eventHandler;}
const pragma::networking::ClientEventInterface &pragma::networking::IClient::GetEventInterface() const {return m_eventInterface;}

void pragma::networking::IClient::HandlePacket(NetPacket &packet)
{
	if(m_eventInterface.handlePacket)
		m_eventInterface.handlePacket(packet);
}
void pragma::networking::IClient::OnPacketSent(Protocol protocol,NetPacket &packet)
{
	if(m_eventInterface.onPacketSent)
		m_eventInterface.onPacketSent(protocol,packet);
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
