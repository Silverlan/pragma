#include "stdafx_client.h"
#include "pragma/networking/iclient.hpp"

std::optional<pragma::networking::IPAddress> pragma::networking::IClient::GetIPAddress() const
{
	auto ip = GetIP();
	auto port = GetLocalTCPPort();
	if(ip.has_value() == false || port.has_value() == false)
		return {};
	return pragma::networking::IPAddress{*ip,*port};
}
