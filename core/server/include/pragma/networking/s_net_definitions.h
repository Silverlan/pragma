#ifndef __S_NET_DEFINITIONS_
#define __S_NET_DEFINITIONS_
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include "pragma/networking/netmessages.h"
#define DECLARE_NETMESSAGE_SV(netname) \
	DLLSERVER void NET_sv_##netname(pragma::networking::IServerClient &session,NetPacket packet); \
	REGISTER_NETMESSAGE_SV(netname,NET_sv_##netname);

#endif