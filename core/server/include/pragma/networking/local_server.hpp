/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_LOCAL_SERVER_HPP__
#define __PRAGMA_LOCAL_SERVER_HPP__

#include "pragma/networking/iserver.hpp"
#include "pragma/networking/iserver_client.hpp"

namespace pragma::networking {
	class Error;
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
		virtual bool SendPacket(pragma::networking::Protocol protocol, NetPacket &packet, pragma::networking::Error &outErr) override;
		virtual bool Drop(DropReason reason, pragma::networking::Error &outErr) override;
	};
};

#endif
