#include "stdafx_server.h"
#include "pragma/networking/wvlocalclient.h"
#include <pragma/engine.h>
#include <pragma/networking/nwm_util.h>

extern DLLENGINE Engine *engine;
WVLocalClient::WVLocalClient()
	: WVServerClient(nullptr)
{
	m_conTCP.ip = m_conUDP.ip = "::1";
	m_bReady = true;
}

void WVLocalClient::SendPacket(const NetPacket &packet,nwm::Protocol protocol)
{
	NetPacket p = packet;
	p.SetTimeActivated(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
	p->SetOffset(0);
	engine->HandleLocalPlayerClientPacket(p);
}
bool WVLocalClient::IsUDPConnected() const {return true;}
bool WVLocalClient::IsTCPConnected() const {return true;}
bool WVLocalClient::IsFullyConnected() const {return true;}
