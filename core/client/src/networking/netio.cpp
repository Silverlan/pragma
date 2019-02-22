#include "stdafx_client.h"
#include "pragma/clientstate/clientutil.h"
#include <pragma/engine.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/networking/wvclient.h"

extern DLLENGINE Engine *engine;
void ClientState::SendPacketTCP(unsigned int ID,NetPacket &packet)
{
	packet.SetMessageID(ID);
	CallCallbacks<void,std::reference_wrapper<NetPacket>>("OnSendPacketTCP",std::ref(packet));
	if(!IsConnected())
	{
		packet.SetTimeActivated(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
		packet->SetOffset(0);
		engine->HandleLocalPlayerServerPacket(packet);
		return;
	}
	m_client->SendPacket(nwm::Protocol::TCP,packet);
}

void ClientState::SendPacketUDP(unsigned int ID,NetPacket &packet)
{
	packet.SetMessageID(ID);
	CallCallbacks<void,std::reference_wrapper<NetPacket>>("OnSendPacketUDP",std::ref(packet));
	if(!IsConnected())
	{
		packet.SetTimeActivated(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
		packet->SetOffset(0);
		engine->HandleLocalPlayerServerPacket(packet);
		return;
	}
	m_client->SendPacket(nwm::Protocol::UDP,packet);
}

void ClientState::SendPacketTCP(const std::string &name,NetPacket &packet)
{
	unsigned int ID = GetServerMessageID(name);
	if(ID == 0)
		return;
	SendPacketTCP(ID,packet);
}
void ClientState::SendPacketUDP(const std::string &name,NetPacket &packet)
{
	unsigned int ID = GetServerMessageID(name);
	if(ID == 0)
		return;
	SendPacketUDP(ID,packet);
}
void ClientState::SendPacketTCP(const std::string &name)
{
	NetPacket p;
	SendPacketTCP(name,p);
}

void ClientState::SendPacketUDP(const std::string &name)
{
	NetPacket p;
	SendPacketUDP(name,p);
}
