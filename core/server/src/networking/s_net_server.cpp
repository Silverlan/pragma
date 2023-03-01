/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/s_net_server.h"
#include <pragma/console/conout.h>

void NET_cl_send(pragma::networking::IServerClient &session, NetPacket packet)
{
	std::string msg = packet->ReadString();
	Con::csv << "Received cl_send message from client '" << session.GetIdentifier() << "': " << msg << Con::endl;
}
