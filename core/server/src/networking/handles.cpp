// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include <pragma/engine.h>
#include "pragma/networking/netmessages.h"

import pragma.server.server_state;

extern DLLNETWORK Engine *engine;

bool ServerState::HandlePacket(pragma::networking::IServerClient &session, NetPacket &packet)
{
	unsigned int ID = packet.GetMessageID();
	SVNetMessage *msg = GetNetMessage(ID);
	if(msg == nullptr)
		return false;
	msg->handler(session, packet);
	return true;
}
