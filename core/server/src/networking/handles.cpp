/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include <pragma/engine.h>
#include "pragma/networking/netmessages.h"

extern DLLENGINE Engine *engine;;
bool ServerState::HandlePacket(pragma::networking::IServerClient &session,NetPacket &packet)
{
	unsigned int ID = packet.GetMessageID();
	SVNetMessage *msg = GetNetMessage(ID);
	if(msg == nullptr)
		return false;
	msg->handler(session,packet);
	return true;
}
