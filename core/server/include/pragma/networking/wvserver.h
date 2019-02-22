#ifndef __WVSERVER_H__
#define __WVSERVER_H__

#include "pragma/serverdefinitions.h"
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <networkmanager/udp_handler/udp_message_dispatcher.h>
#include <pragma/networking/nwm_message_tracker.hpp>

class NetPacket;
class WVServerClient;
class DLLSERVER WVServer
	: public nwm::Server,
	public pragma::networking::MessageTracker
{
public:
	static std::unique_ptr<WVServer> Create(uint16_t tcpPort,uint16_t udpPort,nwm::ConnectionType conType=nwm::ConnectionType::UDP);
	static std::unique_ptr<WVServer> Create(uint16_t port,nwm::ConnectionType conType=nwm::ConnectionType::UDP);
	void Heartbeat();
	virtual void PollEvents() override;
public:
	friend nwm::Server;
protected:
	virtual bool HandleAsyncPacket(const NWMEndpoint &ep,NWMSession *session,uint32_t id,NetPacket &packet) override;
	virtual bool HandlePacket(const NWMEndpoint &ep,nwm::ServerClient *cl,unsigned int id,NetPacket &packet) override;
	virtual void OnClientConnected(nwm::ServerClient *cl) override;
	virtual void OnClientDropped(nwm::ServerClient *cl,nwm::ClientDropped reason) override;
	virtual void OnClosed() override;

	virtual void OnPacketSent(const NWMEndpoint &ep,const NetPacket &packet) override;
	virtual void OnPacketReceived(const NWMEndpoint &ep,nwm::ServerClient *cl,unsigned int id,NetPacket &packet) override;

	virtual std::shared_ptr<nwm::ServerClient> CreateClient() override;
	std::unique_ptr<UDPMessageDispatcher> m_dispatcher;
	ChronoTimePoint m_lastHeartBeat;
	WVServer(const std::shared_ptr<SVNWMUDPConnection> &udp,const std::shared_ptr<SVNWMTCPConnection> &tcp);
};

#endif