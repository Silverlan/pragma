/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_NET_DEFINITIONS_
#define __S_NET_DEFINITIONS_
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include "pragma/networking/netmessages.h"
#define DECLARE_NETMESSAGE_SV(netname) \
	DLLSERVER void NET_sv_##netname(pragma::networking::IServerClient &session,NetPacket packet); \
	REGISTER_NETMESSAGE_SV(netname,NET_sv_##netname);

#endif