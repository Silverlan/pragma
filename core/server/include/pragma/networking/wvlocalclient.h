#ifndef __WVLOCALCLIENT_H__
#define __WVLOCALCLIENT_H__

#include "pragma/serverdefinitions.h"
#include "pragma/networking/wvserverclient.h"

class NetPacket;
class DLLSERVER WVLocalClient
	: public WVServerClient
{
public:
	WVLocalClient();
	virtual bool IsUDPConnected() const override;
	virtual bool IsTCPConnected() const override;
	virtual bool IsFullyConnected() const override;

	// Must only be called from main thread!
	virtual void SendPacket(const NetPacket &p,nwm::Protocol protocol=nwm::Protocol::TCP) override;
};

#endif