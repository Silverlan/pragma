// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_NET_SERVER_H__
#define __S_NET_SERVER_H__
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include "pragma/networking/netmessages.h"

DLLSERVER void NET_cl_send(pragma::networking::IServerClient &session, NetPacket packet);
REGISTER_NETMESSAGE_SV(cl_send, NET_cl_send);
#endif
