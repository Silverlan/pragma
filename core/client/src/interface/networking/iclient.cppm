// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:networking.iclient;

export import pragma.shared;

export namespace pragma::networking {
	class DLLCLIENT ClientEventInterface {
	  public:
		std::function<void()> onConnected = nullptr;
		std::function<void()> onDisconnected = nullptr;
		std::function<void()> onConnectionClosed = nullptr;
		std::function<void(Protocol, NetPacket &)> onPacketSent = nullptr;
		std::function<void(NetPacket &)> handlePacket = nullptr;
	};

	class DLLCLIENT IClient : public MessageTracker {
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
		virtual bool PollEvents(Error &outErr) = 0;
		virtual uint16_t GetLatency() const = 0;
		virtual void SetTimeoutDuration(float duration) = 0;
		virtual std::optional<std::string> GetIP() const = 0;
		virtual std::optional<Port> GetLocalTCPPort() const { return {}; }
		virtual std::optional<Port> GetLocalUDPPort() const { return {}; }
		virtual std::string GetNetworkLayerIdentifier() const = 0;
		std::optional<IPAddress> GetIPAddress() const;
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
