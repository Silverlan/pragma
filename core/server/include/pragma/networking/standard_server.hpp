/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __PRAGMA_STANDARD_SERVER_HPP__
#define __PRAGMA_STANDARD_SERVER_HPP__

#include <pragma/networking/enums.hpp>
#include "pragma/networking/iserver.hpp"
#include "pragma/networking/iserver_client.hpp"
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <servermanager/interface/sv_nwm_serverclient.hpp>
#include <networkmanager/udp_handler/udp_message_dispatcher.h>
#include <sharedutils/util_weak_handle.hpp>

namespace nwm {enum class Protocol : uint32_t;};
namespace pragma {class SPlayerComponent;};
namespace pragma::networking
{
	class StandardServer;
	class StandardServerClient;
	class DLLSERVER NWMActiveServer
		: public nwm::Server
	{
	public:
		static std::unique_ptr<NWMActiveServer> Create(uint16_t tcpPort,uint16_t udpPort,nwm::ConnectionType conType=nwm::ConnectionType::UDP);
		static std::unique_ptr<NWMActiveServer> Create(uint16_t port,nwm::ConnectionType conType=nwm::ConnectionType::UDP);
		void SetServer(StandardServer &server);
		void Heartbeat();
		virtual void PollEvents() override;

		nwm::ServerClient &GetNWMClient(StandardServerClient &cl) const;
		StandardServerClient &GetPragmaClient(nwm::ServerClient &cl) const;

		friend nwm::Server;
	protected:
		virtual bool HandleAsyncPacket(const NWMEndpoint &ep,NWMSession *session,uint32_t id,NetPacket &packet) override;
		virtual bool HandlePacket(const NWMEndpoint &ep,nwm::ServerClient *cl,unsigned int id,NetPacket &packet) override;
		virtual void OnClientConnected(nwm::ServerClient *cl) override;
		virtual void OnClientDropped(nwm::ServerClient *cl,nwm::ClientDropped reason) override;
		virtual void OnClosed() override;

		virtual void OnPacketSent(const NWMEndpoint &ep,const NetPacket &packet) override;
		virtual void OnPacketReceived(const NWMEndpoint &ep,nwm::ServerClient *cl,unsigned int id,NetPacket &packet) override;

		virtual std::shared_ptr<nwm::ServerClient> CreateClient() override;
		std::unique_ptr<UDPMessageDispatcher> m_dispatcher;
		ChronoTimePoint m_lastHeartBeat;
		StandardServer *m_server;

		std::unordered_map<nwm::ServerClient*,StandardServerClient*> m_nwmToPragmaClient = {};

		NWMActiveServer(const std::shared_ptr<SVNWMUDPConnection> &udp,const std::shared_ptr<SVNWMTCPConnection> &tcp);
	};

	class NWMActiveServerClient
		: public nwm::ServerClient
	{
	public:
		friend nwm::Server;
	protected:
		NWMActiveServerClient(nwm::Server *manager);
		virtual void OnClosed() override;
	public:
		virtual ~NWMActiveServerClient() override;
	};

	class DLLSERVER StandardServer
		: public IServer
	{
	public:
		virtual bool Heartbeat() override;
		virtual std::optional<std::string> GetHostIP() const override;
		virtual std::optional<Port> GetHostPort() const override;
		virtual bool PollEvents(Error &outErr) override;
		virtual void SetTimeoutDuration(float duration) override;
	protected:
		virtual bool DoStart(Error &outErr,uint16_t port,bool useP2PIfAvailable=false) override;
		virtual bool DoShutdown(Error &outErr) override;
	private:
		std::unique_ptr<NWMActiveServer> m_server = nullptr;
	};

	class DLLSERVER StandardServerClient
		: public IServerClient
	{
	public:
		StandardServerClient();
		virtual uint16_t GetLatency() const override;
		virtual std::string GetIdentifier() const override;
		virtual std::optional<std::string> GetIP() const override;
		virtual std::optional<Port> GetPort() const override;
		virtual bool IsListenServerHost() const override;
		virtual bool Drop(DropReason reason,pragma::networking::Error &outErr) override;
		virtual bool SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr) override;

		void SetNWMClient(nwm::ServerClient *cl);
		nwm::ServerClient &GetNWMClient() const;
	private:
		mutable nwm::ServerClient *m_nwmClient = nullptr;
	};
};

#endif
