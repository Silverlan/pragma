#ifndef __WVCLIENT_H__
#define __WVCLIENT_H__

#include <clientmanager/interface/cl_nwm_manager.hpp>
#include <pragma/networking/nwm_message_tracker.hpp>

class NetPacket;
class WVClient
	: public nwm::Client,
	public pragma::networking::MessageTracker
{
public:
	friend nwm::Client;
	static std::unique_ptr<WVClient> Create(const std::string &serverIp,unsigned short serverPort);

	bool IsDisconnected() const;
protected:
	virtual bool HandlePacket(const NWMEndpoint &ep,unsigned int id,NetPacket &packet) override;
	virtual void OnConnected() override;
	virtual void OnClosed() override;
	virtual void OnDisconnected(nwm::ClientDropped reason) override;

	virtual void OnPacketSent(const NWMEndpoint &ep,const NetPacket &packet) override;
	virtual void OnPacketReceived(const NWMEndpoint &ep,unsigned int id,NetPacket &packet) override;

	WVClient(const std::shared_ptr<CLNWMUDPConnection> &udp,std::shared_ptr<CLNWMTCPConnection> &tcp);
private:
	bool m_bDisconnected = false;
};

#endif