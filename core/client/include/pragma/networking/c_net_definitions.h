/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_NET_DEFINITIONS_
#define __C_NET_DEFINITIONS_
#include "pragma/clientdefinitions.h"
#include "pragma/networkdefinitions.h"
#include "pragma/networking/netmessages.h"
#define DECLARE_NETMESSAGE_CL(netname)                                                                                                                                                                                                                                                           \
	DLLCLIENT void NET_cl_##netname(NetPacket packet);                                                                                                                                                                                                                                           \
	REGISTER_NETMESSAGE_CL(netname, NET_cl_##netname);

#endif
