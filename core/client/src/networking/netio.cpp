#include "stdafx_client.h"
#include "pragma/clientstate/clientutil.h"
#include "pragma/networking/iclient.hpp"
#include <pragma/networking/error.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/engine.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/networking/wvclient.h"

extern DLLENGINE Engine *engine;
void ClientState::SendPacket(const std::string &name,NetPacket &packet,pragma::networking::Protocol protocol)
{
	auto ID = GetServerMessageID(name);
	if(ID == 0)
		return;
	packet.SetMessageID(ID);
	pragma::networking::Error err;
	if(m_client->SendPacket(protocol,packet,err) == false)
		Con::cwar<<"WARNING: Unable to send packet '"<<name<<"': "<<err.GetMessage()<<Con::endl;
}
void ClientState::SendPacket(const std::string &name,NetPacket &packet)
{
	SendPacket(name,packet,pragma::networking::Protocol::FastUnreliable);
}
void ClientState::SendPacket(const std::string &name,pragma::networking::Protocol protocol)
{
	SendPacket(name,NetPacket{},protocol);
}
