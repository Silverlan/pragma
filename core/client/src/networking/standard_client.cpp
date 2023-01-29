/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/networking/standard_client.hpp"
#include <pragma/networking/error.hpp>

std::string pragma::networking::StandardClient::GetIdentifier() const { return m_clientConnection->GetLocalAddress().ToString(); }
bool pragma::networking::StandardClient::Connect(const std::string &ip, Port port, Error &outErr)
{
	if(m_clientConnection != nullptr) {
		m_clientConnection->Disconnect();
		m_clientConnection = nullptr;
	}
	try {
		m_clientConnection = NWMClientConnection::Create(ip, port);
	}
	catch(NWMException &e) {
		outErr = {ErrorCode::UnableToConnect, e.what()};
		return false;
	}
	return true;
}
bool pragma::networking::StandardClient::Disconnect(Error &outErr)
{
	m_clientConnection->Disconnect();
	return true;
}
bool pragma::networking::StandardClient::SendPacket(Protocol protocol, NetPacket &packet, Error &outErr)
{
	m_clientConnection->SendPacket(get_nwm_protocol(protocol), packet);
	return true;
}
bool pragma::networking::StandardClient::IsRunning() const { return m_clientConnection != nullptr; }
bool pragma::networking::StandardClient::IsDisconnected() const { return m_clientConnection->IsDisconnected(); }
bool pragma::networking::StandardClient::PollEvents(pragma::networking::Error &outErr)
{
	m_clientConnection->PollEvents();
	return true;
}
uint16_t pragma::networking::StandardClient::GetLatency() const { return m_clientConnection->GetLatency(); }
void pragma::networking::StandardClient::SetTimeoutDuration(float duration) { m_clientConnection->SetTimeoutDuration(duration); }
std::optional<std::string> pragma::networking::StandardClient::GetIP() const { return m_clientConnection->GetIP(); }
std::optional<pragma::networking::Port> pragma::networking::StandardClient::GetLocalTCPPort() const { return m_clientConnection->GetLocalTCPPort(); }
std::optional<pragma::networking::Port> pragma::networking::StandardClient::GetLocalUDPPort() const { return m_clientConnection->GetLocalUDPPort(); }
