/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_ISERVER_CLIENT_HPP__
#define __PRAGMA_ISERVER_CLIENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/networking/enums.hpp"
#include "pragma/networking/ip_address.hpp"
#include <cinttypes>

struct Resource;
class NetPacket;
namespace pragma {
	class SPlayerComponent;
};
namespace pragma::networking {
	enum class DropReason : int8_t;
	class Error;
	class DLLSERVER IServerClient : public std::enable_shared_from_this<IServerClient> {
	  public:
		template<class TServerClient, typename... TARGS>
		static std::shared_ptr<TServerClient> Create(TARGS &&...args);
		~IServerClient();
		virtual void Initialize() {};
		virtual bool Drop(DropReason reason, pragma::networking::Error &outErr) = 0;
		virtual uint16_t GetLatency() const = 0;
		virtual std::string GetIdentifier() const = 0;
		std::optional<IPAddress> GetIPAddress() const;
		virtual std::optional<std::string> GetIP() const = 0;
		virtual std::optional<Port> GetPort() const = 0;
		virtual bool IsListenServerHost() const = 0;
		virtual bool SendPacket(pragma::networking::Protocol protocol, NetPacket &packet, pragma::networking::Error &outErr) = 0;

		enum class TransferState : uint32_t { Initial = 0, Started, Complete };

		pragma::SPlayerComponent *GetPlayer() const;
		void SetPlayer(pragma::SPlayerComponent &pl);
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
		mutable pragma::ComponentHandle<pragma::SPlayerComponent> m_player = {};
		bool m_bTransferring = false;
		std::vector<std::shared_ptr<Resource>> m_resourceTransfer;
		TransferState m_initialResourceTransferState = TransferState::Initial;

		uint8_t m_snapshotId = 0;
		std::vector<std::string> m_scheduledResources; // Scheduled resource files for download

		// TODO: Move this somewhere else?
		uint64_t m_steamId = 0;
	};
};

template<class TServerClient, typename... TARGS>
std::shared_ptr<TServerClient> pragma::networking::IServerClient::Create(TARGS &&...args)
{
	auto p = std::shared_ptr<TServerClient> {new TServerClient {std::forward<TARGS>(args)...}};
	p->Initialize();
	return p;
}

#endif
