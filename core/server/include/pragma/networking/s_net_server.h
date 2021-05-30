/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_NET_SERVER_H__
#define __S_NET_SERVER_H__
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include "pragma/networking/netmessages.h"

DLLSERVER void NET_cl_send(pragma::networking::IServerClient &session,NetPacket packet);
REGISTER_NETMESSAGE_SV(cl_send,NET_cl_send);
#endif