#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include <pragma/engine.h>
#include "pragma/networking/netmessages.h"

extern DLLENGINE Engine *engine;;
bool ServerState::HandlePacket(pragma::networking::IServerClient &session,NetPacket &packet)
{
	unsigned int ID = packet.GetMessageID();
	SVNetMessage *msg = GetNetMessage(ID);
	if(msg == nullptr)
		return false;
	msg->handler(session,packet);
	return true;
}
