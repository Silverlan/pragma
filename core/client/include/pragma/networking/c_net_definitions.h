// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_NET_DEFINITIONS_
#define __C_NET_DEFINITIONS_
#define DECLARE_NETMESSAGE_CL(netname)                                                                                                                                                                                                                                                           \
	DLLCLIENT void NET_cl_##netname(NetPacket packet);                                                                                                                                                                                                                                           \
	REGISTER_NETMESSAGE_CL(netname, NET_cl_##netname);

#endif
