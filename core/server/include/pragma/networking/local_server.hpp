#ifndef __PRAGMA_LOCAL_SERVER_HPP__
#define __PRAGMA_LOCAL_SERVER_HPP__

#include "pragma/networking/iserver.hpp"
#include "pragma/networking/iserver_client.hpp"

namespace pragma::networking
{
	class Error;
	class DLLSERVER LocalServer
		: public IServer
	{
	public:
		virtual bool Start(Error &outErr) override;
		virtual bool PollEvents(Error &outErr) override;
		virtual bool Heartbeat() override;
		virtual void SetTimeoutDuration(float duration) override;
	protected:
		virtual bool DoShutdown(Error &outErr) override;
	};

	class DLLSERVER LocalServerClient
		: public IServerClient
	{
	public:
		LocalServerClient()=default;
		~LocalServerClient();
		virtual uint16_t GetLatency() const override;
		virtual std::string GetIdentifier() const override;
		virtual std::optional<std::string> GetIP() const override;
		virtual std::optional<Port> GetPort() const override;
		virtual bool IsListenServerHost() const override;
		virtual bool SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr) override;
		virtual bool Drop(DropReason reason,pragma::networking::Error &outErr) override;
	};
};

#endif
