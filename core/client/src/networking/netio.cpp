/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientutil.h"
#include "pragma/networking/iclient.hpp"
#include <pragma/networking/error.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/engine.h>
#include <pragma/networking/nwm_util.h>

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
