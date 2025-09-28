// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_NET_DEFINITIONS_
#define __S_NET_DEFINITIONS_
#define DECLARE_NETMESSAGE_SV(netname)                                                                                                                                                                                                                                                           \
	DLLSERVER void NET_sv_##netname(pragma::networking::IServerClient &session, NetPacket packet);                                                                                                                                                                                               \
	REGISTER_NETMESSAGE_SV(netname, NET_sv_##netname);

#endif
