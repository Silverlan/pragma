// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:networking.local_host;

export import :networking.iserver;
export import :networking.iserver_client;

export namespace pragma::networking {
	class DLLSERVER LocalServer : public IServer {
	  public:
		virtual bool PollEvents(Error &outErr) override;
		virtual bool Heartbeat() override;
		virtual void SetTimeoutDuration(float duration) override;
		virtual bool IsPeerToPeer() const override { return false; }
		virtual std::string GetNetworkLayerIdentifier() const override { return "local"; }
	  protected:
		virtual bool DoShutdown(Error &outErr) override;
		virtual bool DoStart(Error &outErr, uint16_t port, bool useP2PIfAvailable = false) override;
	};

	class DLLSERVER LocalServerClient : public IServerClient {
	  public:
		LocalServerClient() = default;
		~LocalServerClient();
		virtual uint16_t GetLatency() const override;
		virtual std::string GetIdentifier() const override;
		virtual std::optional<std::string> GetIP() const override;
		virtual std::optional<Port> GetPort() const override;
		virtual bool IsListenServerHost() const override;
		virtual bool SendPacket(Protocol protocol, NetPacket &packet, Error &outErr) override;
		virtual bool Drop(DropReason reason, Error &outErr) override;
	};
};
