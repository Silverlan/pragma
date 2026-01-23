// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:networking.iserver_client;

export import :networking.resource;
export import pragma.shared;

export {
	namespace pragma::networking {
		class DLLSERVER IServerClient : public std::enable_shared_from_this<IServerClient> {
		  public:
			template<class TServerClient, typename... TARGS>
			static std::shared_ptr<TServerClient> Create(TARGS &&...args)
			{
				auto p = std::shared_ptr<TServerClient> {new TServerClient {std::forward<TARGS>(args)...}};
				p->Initialize();
				return p;
			}
			~IServerClient();
			virtual void Initialize() {};
			virtual bool Drop(DropReason reason, Error &outErr) = 0;
			virtual uint16_t GetLatency() const = 0;
			virtual std::string GetIdentifier() const = 0;
			std::optional<IPAddress> GetIPAddress() const;
			virtual std::optional<std::string> GetIP() const = 0;
			virtual std::optional<Port> GetPort() const = 0;
			virtual bool IsListenServerHost() const = 0;
			virtual bool SendPacket(Protocol protocol, NetPacket &packet, Error &outErr) = 0;

			enum class TransferState : uint32_t { Initial = 0, Started, Complete };

			BasePlayerComponent *GetPlayer() const;
			void SetPlayer(BasePlayerComponent &pl);
			const std::vector<std::shared_ptr<Resource>> &GetResourceTransfer() const;
			bool AddResource(const std::string &fileName, bool stream = true);
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

			// TODO: These don't really belong here
			void SetSteamId(uint64_t steamId);
			uint64_t GetSteamId() const;
		  protected:
			IServerClient() = default;
		  private:
			mutable ComponentHandle<BasePlayerComponent> m_player = {};
			bool m_bTransferring = false;
			std::vector<std::shared_ptr<Resource>> m_resourceTransfer;
			TransferState m_initialResourceTransferState = TransferState::Initial;

			uint8_t m_snapshotId = 0;
			std::vector<std::string> m_scheduledResources; // Scheduled resource files for download

			// TODO: Move this somewhere else?
			uint64_t m_steamId = 0;
		};
	};
}
