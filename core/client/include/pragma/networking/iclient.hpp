#ifndef __PRAGMA_ICLIENT_HPP__
#define __PRAGMA_ICLIENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_message_tracker.hpp>
#include <string>
#include <optional>

namespace pragma::networking
{
	class Error;
	class DLLCLIENT IClient
		: public pragma::networking::MessageTracker
	{
	public:
		virtual ~IClient()=default;
		virtual std::string GetIdentifier() const=0;
		virtual bool Connect(const std::string &ip,Port port,Error &outErr)=0;
		virtual bool Disconnect(Error &outErr)=0;
		virtual bool SendPacket(Protocol protocol,NetPacket &packet,Error &outErr)=0;
		virtual bool IsRunning() const=0;
		virtual bool IsDisconnected() const=0;
		virtual bool PollEvents(pragma::networking::Error &outErr)=0;
		virtual uint16_t GetLatency() const=0;
		virtual void SetTimeoutDuration(float duration)=0;
		virtual std::optional<nwm::IPAddress> GetIPAddress() const=0;
		virtual std::optional<std::string> GetIP() const=0;
		virtual std::optional<Port> GetLocalTCPPort() const {return {};}
		virtual std::optional<Port> GetLocalUDPPort() const {return {};}
	};
};

#endif
