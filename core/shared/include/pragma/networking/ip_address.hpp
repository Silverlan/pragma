/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __IP_ADDRESS_HPP__
#define __IP_ADDRESS_HPP__

#include "pragma/networkdefinitions.h"
#include <networkmanager/wrappers/nwm_ip_address.hpp>

namespace pragma::networking
{
	class DLLNETWORK IPAddress
	{
	public:
		IPAddress();
		IPAddress(const std::string &ip,uint16_t port);

		std::string ToString() const;
		const std::string &GetIP() const;
		uint16_t GetPort() const;
	private:
		std::string m_ip = "";
		uint16_t m_port = 0;
	};
};

#endif
