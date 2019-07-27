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
	class IServerClient;
	enum class Protocol : uint8_t;
	class DLLSERVER ServerEventInterface
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
		template<class TServer,typename... TARGS>
			static std::unique_ptr<TServer,void(*)(TServer*)> Create(TARGS&& ...args);
		virtual ~IServer()=default;
		bool Start(Error &outErr,uint16_t port,bool useP2PIfAvailable=false);
		virtual bool Heartbeat()=0;
		virtual bool PollEvents(Error &outErr)=0;
		virtual void SetTimeoutDuration(float duration)=0;
		virtual std::optional<std::string> GetHostIP() const {return {};}
		virtual std::optional<Port> GetHostPort() const {return {};}
		virtual std::optional<uint64_t> GetSteamId() const {return {};}
		virtual bool IsPeerToPeer() const=0;
		// Note: The identifier HAS to match the directory name of the networking module!
		virtual std::string GetNetworkLayerIdentifier() const=0;
		bool Shutdown(Error &outErr);
		bool SendPacket(Protocol protocol,NetPacket &packet,const ClientRecipientFilter &rf,Error &outErr);
		void AddClient(const std::shared_ptr<IServerClient> &client);
		template<class TServerClient,typename... TARGS>
			std::shared_ptr<TServerClient> AddClient(TARGS&& ...args);
		bool DropClient(const IServerClient &client,pragma::networking::DropReason reason,Error &outErr);

		void SetEventInterface(const ServerEventInterface &eventHandler);

		bool IsRunning() const;
		const std::vector<std::shared_ptr<IServerClient>> &GetClients() const;

		// These have to be called by the implementation of IServer
		void HandlePacket(IServerClient &client,NetPacket &packet);
		void OnClientConnected(IServerClient &client);
		void OnClientDropped(IServerClient &client,DropReason reason);
	protected:
		IServer()=default;
		virtual bool DoStart(Error &outErr,uint16_t port,bool useP2PIfAvailable=false)=0;
		const ServerEventInterface &GetEventInterface() const;
		virtual bool DoShutdown(Error &outErr)=0;
	private:
		bool m_bRunning = true;
		std::vector<std::shared_ptr<IServerClient>> m_clients = {};
		ServerEventInterface m_eventInterface = {};
	};
};

template<class TServerClient,typename... TARGS>
	std::shared_ptr<TServerClient> pragma::networking::IServer::AddClient(TARGS&& ...args)
{
	auto cl = TServerClient::Create<TServerClient>(std::forward<TARGS>(args)...);
	AddClient(cl);
	if(m_eventInterface.onClientConnected)
		m_eventInterface.onClientConnected(*cl);
	return cl;
}

template<class TServer,typename... TARGS>
	std::unique_ptr<TServer,void(*)(TServer*)> pragma::networking::IServer::Create(TARGS&& ...args)
{
	std::unique_ptr<TServer,void(*)(TServer*)> r {new TServer{std::forward<TARGS>(args)...},[](TServer *ptr) {
		ptr->Shutdown();
		delete ptr;
	}};
	return r;
}

#endif
