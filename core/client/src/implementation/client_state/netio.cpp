// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/networking/error.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/engine.h>
#include <pragma/networking/nwm_util.h>

module pragma.client;

import :client_state;

extern DLLNETWORK Engine *engine;
void ClientState::SendPacket(const std::string &name, NetPacket &packet, pragma::networking::Protocol protocol)
{
	auto ID = GetServerMessageID(name);
	if(ID == 0 || m_client == nullptr)
		return;
	packet.SetMessageID(ID);
	pragma::networking::Error err;
	if(m_client->SendPacket(protocol, packet, err) == false)
		Con::cwar << "Unable to send packet '" << name << "': " << err.GetMessage() << Con::endl;
}
void ClientState::SendPacket(const std::string &name, NetPacket &packet) { SendPacket(name, packet, pragma::networking::Protocol::FastUnreliable); }
void ClientState::SendPacket(const std::string &name, pragma::networking::Protocol protocol)
{
	NetPacket packet {};
	SendPacket(name, packet, protocol);
}
