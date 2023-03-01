/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_ICLIENT_HPP__
#define __PRAGMA_ICLIENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_message_tracker.hpp>
#include <pragma/networking/ip_address.hpp>
#include <string>
#include <optional>

namespace pragma::networking {
	class Error;

	class DLLCLIENT ClientEventInterface {
	  public:
		std::function<void()> onConnected = nullptr;
		std::function<void()> onDisconnected = nullptr;
		std::function<void()> onConnectionClosed = nullptr;
		std::function<void(Protocol, NetPacket &)> onPacketSent = nullptr;
		std::function<void(NetPacket &)> handlePacket = nullptr;
	};

	class DLLCLIENT IClient : public pragma::networking::MessageTracker {
	  public:
		virtual ~IClient() = default;
		virtual std::string GetIdentifier() const = 0;
		virtual bool Connect(const std::string &ip, Port port, Error &outErr) = 0;
		// Steamworks only!
		virtual bool Connect(uint64_t steamId, Error &outErr) = 0;
		virtual bool Disconnect(Error &outErr) = 0;
		virtual bool SendPacket(Protocol protocol, NetPacket &packet, Error &outErr) = 0;
		virtual bool IsRunning() const = 0;
		virtual bool IsDisconnected() const = 0;
		virtual bool PollEvents(pragma::networking::Error &outErr) = 0;
		virtual uint16_t GetLatency() const = 0;
		virtual void SetTimeoutDuration(float duration) = 0;
		virtual std::optional<std::string> GetIP() const = 0;
		virtual std::optional<Port> GetLocalTCPPort() const { return {}; }
		virtual std::optional<Port> GetLocalUDPPort() const { return {}; }
		virtual std::string GetNetworkLayerIdentifier() const = 0;
		std::optional<networking::IPAddress> GetIPAddress() const;
		void SetEventInterface(const ClientEventInterface &eventHandler);

		// These have to be called by the implementation of IClient
		void HandlePacket(NetPacket &packet);
		void OnPacketSent(Protocol protocol, NetPacket &packet);
		void OnConnected();
		void OnDisconnected();
		void OnConnectionClosed();
	  protected:
		const ClientEventInterface &GetEventInterface() const;
	  private:
		ClientEventInterface m_eventInterface = {};
	};
};

#endif
