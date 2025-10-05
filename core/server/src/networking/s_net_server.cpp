// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/serverdefinitions.h"

import pragma.shared;

void NET_cl_send(pragma::networking::IServerClient &session, NetPacket packet)
{
	std::string msg = packet->ReadString();
	Con::csv << "Received cl_send message from client '" << session.GetIdentifier() << "': " << msg << Con::endl;
}
