/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __PRAGMA_STANDARD_CLIENT_HPP__
#define __PRAGMA_STANDARD_CLIENT_HPP__

#include "pragma/networking/iclient.hpp"
#include <clientmanager/interface/cl_nwm_manager.hpp>

namespace pragma::networking
{
	class StandardClient;
	class NWMClientConnection
		: public nwm::Client,
		public pragma::networking::MessageTracker
	{
	public:
		friend nwm::Client;
		static std::unique_ptr<NWMClientConnection> Create(const std::string &serverIp,unsigned short serverPort);
		void SetClient(StandardClient &client);

		bool IsDisconnected() const;
	protected:
		virtual bool HandlePacket(const NWMEndpoint &ep,unsigned int id,NetPacket &packet) override;
		virtual void OnConnected() override;
		virtual void OnClosed() override;
		virtual void OnDisconnected(nwm::ClientDropped reason) override;

		virtual void OnPacketSent(const NWMEndpoint &ep,const NetPacket &packet) override;
		virtual void OnPacketReceived(const NWMEndpoint &ep,unsigned int id,NetPacket &packet) override;

		NWMClientConnection(const std::shared_ptr<CLNWMUDPConnection> &udp,std::shared_ptr<CLNWMTCPConnection> &tcp);
	private:
		bool m_bDisconnected = false;
		StandardClient *m_client = nullptr;
	};

	class Error;
	class DLLCLIENT StandardClient
		: public IClient
	{
	public:
		virtual std::string GetIdentifier() const override;
		virtual bool Connect(const std::string &ip,Port port,Error &outErr) override;
		virtual bool Disconnect(Error &outErr) override;
		virtual bool SendPacket(Protocol protocol,NetPacket &packet,Error &outErr) override;
		virtual bool IsRunning() const override;
		virtual bool IsDisconnected() const override;
		virtual bool PollEvents(pragma::networking::Error &outErr) override;
		virtual uint16_t GetLatency() const override;
		virtual void SetTimeoutDuration(float duration) override;
		virtual std::optional<std::string> GetIP() const override;
		virtual std::optional<Port> GetLocalTCPPort() const override;
		virtual std::optional<Port> GetLocalUDPPort() const override;
	private:
		std::unique_ptr<NWMClientConnection> m_clientConnection = nullptr;
	};
};

#endif
