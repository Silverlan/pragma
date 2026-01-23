// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:networking.standard_server;

import :entities.components;
export import :networking.iserver;
export import :networking.iserver_client;
export import pragma.server_manager;

export namespace pragma::networking {
	class StandardServer;
	class StandardServerClient;
	class DLLSERVER NWMActiveServer : public nwm::Server {
	  public:
		static std::unique_ptr<NWMActiveServer> Create(uint16_t tcpPort, uint16_t udpPort, nwm::ConnectionType conType = nwm::ConnectionType::UDP);
		static std::unique_ptr<NWMActiveServer> Create(uint16_t port, nwm::ConnectionType conType = nwm::ConnectionType::UDP);
		void SetServer(StandardServer &server);
		void Heartbeat();
		virtual void PollEvents() override;

		nwm::ServerClient &GetNWMClient(StandardServerClient &cl) const;
		StandardServerClient &GetPragmaClient(nwm::ServerClient &cl) const;

		friend Server;
	  protected:
		virtual bool HandleAsyncPacket(const NWMEndpoint &ep, NWMSession *session, uint32_t id, NetPacket &packet) override;
		virtual bool HandlePacket(const NWMEndpoint &ep, nwm::ServerClient *cl, unsigned int id, NetPacket &packet) override;
		virtual void OnClientConnected(nwm::ServerClient *cl) override;
		virtual void OnClientDropped(nwm::ServerClient *cl, nwm::ClientDropped reason) override;
		virtual void OnClosed() override;

		virtual void OnPacketSent(const NWMEndpoint &ep, const NetPacket &packet) override;
		virtual void OnPacketReceived(const NWMEndpoint &ep, nwm::ServerClient *cl, unsigned int id, NetPacket &packet) override;

		virtual std::shared_ptr<nwm::ServerClient> CreateClient() override;
		std::unique_ptr<UDPMessageDispatcher> m_dispatcher;
		ChronoTimePoint m_lastHeartBeat;
		StandardServer *m_server;

		std::unordered_map<nwm::ServerClient *, StandardServerClient *> m_nwmToPragmaClient = {};

		NWMActiveServer(const std::shared_ptr<SVNWMUDPConnection> &udp, const std::shared_ptr<SVNWMTCPConnection> &tcp);
	};

	class NWMActiveServerClient : public nwm::ServerClient {
	  public:
		friend nwm::Server;
	  protected:
		NWMActiveServerClient(nwm::Server *manager);
		virtual void OnClosed() override;
	  public:
		virtual ~NWMActiveServerClient() override;
	};

	class DLLSERVER StandardServer : public IServer {
	  public:
		virtual bool Heartbeat() override;
		virtual std::optional<std::string> GetHostIP() const override;
		virtual std::optional<Port> GetHostPort() const override;
		virtual bool PollEvents(Error &outErr) override;
		virtual void SetTimeoutDuration(float duration) override;
	  protected:
		virtual bool DoStart(Error &outErr, uint16_t port, bool useP2PIfAvailable = false) override;
		virtual bool DoShutdown(Error &outErr) override;
	  private:
		std::unique_ptr<NWMActiveServer> m_server = nullptr;
	};

	class DLLSERVER StandardServerClient : public IServerClient {
	  public:
		StandardServerClient();
		virtual uint16_t GetLatency() const override;
		virtual std::string GetIdentifier() const override;
		virtual std::optional<std::string> GetIP() const override;
		virtual std::optional<Port> GetPort() const override;
		virtual bool IsListenServerHost() const override;
		virtual bool Drop(DropReason reason, Error &outErr) override;
		virtual bool SendPacket(Protocol protocol, NetPacket &packet, Error &outErr) override;

		void SetNWMClient(nwm::ServerClient *cl);
		nwm::ServerClient &GetNWMClient() const;
	  private:
		mutable nwm::ServerClient *m_nwmClient = nullptr;
	};
};
