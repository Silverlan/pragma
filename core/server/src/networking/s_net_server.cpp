#include "stdafx_server.h"
#include "pragma/networking/wvserverclient.h"
#include "pragma/networking/s_net_server.h"
#include <pragma/console/conout.h>

void NET_cl_send(WVServerClient *session,NetPacket packet)
{
	std::string msg = packet->ReadString();
	Con::csv<<"Received cl_send message from client '"<<session->GetIP()<<"': "<<msg<<Con::endl;
}