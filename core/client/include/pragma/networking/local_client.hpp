/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __PRAGMA_LOCAL_CLIENT_HPP__
#define __PRAGMA_LOCAL_CLIENT_HPP__

#include "pragma/networking/iclient.hpp"

namespace pragma::networking
{
	class Error;
	class DLLCLIENT LocalClient
		: public IClient
	{
	public:
		virtual std::string GetIdentifier() const override;
		virtual bool Connect(const std::string &ip,Port port,Error &outErr) override;
		virtual bool Connect(uint64_t steamId,Error &outErr) override {return false;}
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
		virtual std::string GetNetworkLayerIdentifier() const override {return "local";}
	};
};

#endif
