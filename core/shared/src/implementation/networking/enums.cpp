// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :networking.enums;

std::string pragma::networking::drop_reason_to_string(DropReason reason)
{
	switch(reason) {
	case DropReason::Disconnected:
		return "User disconnected from server!";
	case DropReason::Timeout:
		return "User timed out!";
	case DropReason::Kicked:
		return "User was kicked from the server!";
	case DropReason::Shutdown:
		return "Server shut down!";
	case DropReason::AuthenticationFailed:
		return "Authentication failed!";
	case DropReason::Error:
	default:
		return "User has been dropped due to an error!";
	}
	static_assert(pragma::math::to_integral(DropReason::Count) == 6);
}
nwm::Protocol pragma::networking::get_nwm_protocol(Protocol protocol)
{
	switch(protocol) {
	case Protocol::SlowReliable:
		return nwm::Protocol::TCP;
	case Protocol::FastUnreliable:
	default:
		return nwm::Protocol::UDP;
	}
	static_assert(pragma::math::to_integral(Protocol::Count) == 2);
}
nwm::ClientDropped pragma::networking::get_nwm_drop_reason(DropReason reason)
{
	switch(reason) {
	case DropReason::Disconnected:
		return nwm::ClientDropped::Disconnected;
	case DropReason::Timeout:
		return nwm::ClientDropped::Timeout;
	case DropReason::Kicked:
		return nwm::ClientDropped::Kicked;
	case DropReason::Shutdown:
		return nwm::ClientDropped::Shutdown;
	case DropReason::AuthenticationFailed:
		return nwm::ClientDropped::Error;
	case DropReason::Error:
	default:
		return nwm::ClientDropped::Error;
	}
	static_assert(pragma::math::to_integral(DropReason::Count) == 6);
}
pragma::networking::DropReason pragma::networking::get_pragma_drop_reason(nwm::ClientDropped reason)
{
	switch(reason) {
	case nwm::ClientDropped::Disconnected:
		return DropReason::Disconnected;
	case nwm::ClientDropped::Timeout:
		return DropReason::Timeout;
	case nwm::ClientDropped::Kicked:
		return DropReason::Kicked;
	case nwm::ClientDropped::Shutdown:
		return DropReason::Shutdown;
	case nwm::ClientDropped::Error:
	default:
		return DropReason::Error;
	}
}
