#ifndef __PRAGMA_ISERVER_CLIENT_HPP__
#define __PRAGMA_ISERVER_CLIENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/networking/enums.hpp"
#include <cinttypes>

class Resource;
class NetPacket;
namespace nwm {class IPAddress;};
namespace pragma {class SPlayerComponent;};
namespace pragma::networking
{
	enum class DropReason : int8_t;
	class Error;
	class DLLSERVER IServerClient
		: public std::enable_shared_from_this<IServerClient>
	{
	public:
		template<class TServerClient,class... Args>
			static std::shared_ptr<TServerClient> Create(Args... args);
		~IServerClient();
		virtual void Initialize() {};
		virtual bool Drop(DropReason reason,pragma::networking::Error &outErr)=0;
		virtual uint16_t GetLatency() const=0;
		virtual std::string GetIdentifier() const=0;
		virtual std::optional<nwm::IPAddress> GetIPAddress() const=0;
		virtual std::optional<std::string> GetIP() const=0;
		virtual std::optional<Port> GetPort() const=0;
		virtual bool IsListenServerHost() const=0;
		virtual bool SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr)=0;

		enum class TransferState : uint32_t
		{
			Initial = 0,
			Started,
			Complete
		};

		pragma::SPlayerComponent *GetPlayer() const;
		void SetPlayer(pragma::SPlayerComponent &pl);
		const std::vector<std::shared_ptr<Resource>> &GetResourceTransfer() const;
		bool AddResource(const std::string &fileName,bool stream=true);
		void RemoveResource(uint32_t i);
		void ClearResourceTransfer();
		bool IsInitialResourceTransferComplete() const;
		void SetInitialResourceTransferState(TransferState state);
		TransferState GetInitialResourceTransferState();
		void SetTransferComplete(bool b);
		bool IsTransferring() const;

		uint8_t SwapSnapshotId();
		void Reset();
		void ScheduleResource(const std::string &fileName);
		std::vector<std::string> &GetScheduledResources();
	protected:
		IServerClient()=default;
	private:
		mutable ::util::WeakHandle<pragma::SPlayerComponent> m_player = {};
		bool m_bTransferring = false;
		std::vector<std::shared_ptr<Resource>> m_resourceTransfer;
		TransferState m_initialResourceTransferState = TransferState::Initial;

		uint8_t m_snapshotId = 0;
		std::vector<std::string> m_scheduledResources; // Scheduled resource files for download
	};
};

template<class TServerClient,class... Args>
	std::shared_ptr<TServerClient> pragma::networking::IServerClient::Create(Args... args)
{
	auto p = std::shared_ptr<TServerClient>{new TServerClient{args...}};
	p->Initialize();
	return p;
}

#endif
