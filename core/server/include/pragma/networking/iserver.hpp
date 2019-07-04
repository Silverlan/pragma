#ifndef __PRAGMA_ISERVER_HPP__
#define __PRAGMA_ISERVER_HPP__

#include "pragma/serverdefinitions.h"
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_message_tracker.hpp>
#include <cinttypes>
#include <optional>
#include <functional>

class NetPacket;
namespace pragma::networking
{
	enum class Protocol : uint8_t;
	class ServerEventInterface
	{
	public:
		std::function<void(IServerClient&)> onClientConnected = nullptr;
		std::function<void(IServerClient&,pragma::networking::DropReason)> onClientDropped = nullptr;
		std::function<void(IServerClient&,NetPacket&)> handlePacket = nullptr;
	};
	
	class Error;
	class IServerClient;
	class ClientRecipientFilter;
	class DLLSERVER IServer
		: public pragma::networking::MessageTracker
	{
	public:
		template<class TServer,class... Args>
			static std::unique_ptr<TServer,void(*)(TServer*)> Create(Args... args);
		virtual ~IServer()=default;
		virtual bool Start(Error &outErr)=0;
		virtual bool Heartbeat()=0;
		virtual bool PollEvents(Error &outErr)=0;
		virtual void SetTimeoutDuration(float duration)=0;
		virtual std::optional<std::string> GetHostIP() const {return {};}
		virtual std::optional<Port> GetLocalTCPPort() const {return {};}
		virtual std::optional<Port> GetLocalUDPPort() const {return {};}
		bool Shutdown(Error &outErr);
		bool SendPacket(Protocol protocol,NetPacket &packet,const ClientRecipientFilter &rf,Error &outErr);
		void AddClient(const std::shared_ptr<IServerClient> &client);
		template<class TServerClient,class... Args>
			std::shared_ptr<TServerClient> AddClient(Args... args);
		bool DropClient(const IServerClient &client,pragma::networking::DropReason reason,Error &outErr);

		void SetEventInterface(const ServerEventInterface &eventHandler);

		bool IsRunning() const;
		const std::vector<std::shared_ptr<IServerClient>> &GetClients() const;
	protected:
		IServer()=default;
		const ServerEventInterface &GetEventInterface() const;
		void HandlePacket(IServerClient &client,NetPacket &packet);
		virtual bool DoShutdown(Error &outErr)=0;
	private:
		bool m_bRunning = true;
		std::vector<std::shared_ptr<IServerClient>> m_clients = {};
		ServerEventInterface m_eventInterface = {};
	};
};

template<class TServerClient,class... Args>
	std::shared_ptr<TServerClient> pragma::networking::IServer::AddClient(Args... args)
{
	auto cl = TServerClient::Create<TServerClient>(args...);
	AddClient(cl);
	if(m_eventInterface.onClientConnected)
		m_eventInterface.onClientConnected(*cl);
	return cl;
}

template<class TServer,class... Args>
	std::unique_ptr<TServer,void(*)(TServer*)> pragma::networking::IServer::Create(Args... args)
{
	std::unique_ptr<TServer,void(*)(TServer*)> r {new TServer{args...},[](TServer *ptr) {
		ptr->Shutdown();
		delete ptr;
	}};
	return r;
}

#endif
