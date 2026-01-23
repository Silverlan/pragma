// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:networking.ip_address;

export import std.compat;

export namespace pragma::networking {
	class DLLNETWORK IPAddress {
	  public:
		IPAddress();
		IPAddress(const std::string &ip, uint16_t port);

		std::string ToString() const;
		const std::string &GetIP() const;
		uint16_t GetPort() const;
	  private:
		std::string m_ip = "";
		uint16_t m_port = 0;
	};
};
