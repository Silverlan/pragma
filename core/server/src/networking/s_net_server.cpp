#include "stdafx_server.h"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/s_net_server.h"
#include <pragma/console/conout.h>

void NET_cl_send(pragma::networking::IServerClient &session,NetPacket packet)
{
	std::string msg = packet->ReadString();
	Con::csv<<"Received cl_send message from client '"<<session.GetIdentifier()<<"': "<<msg<<Con::endl;
}