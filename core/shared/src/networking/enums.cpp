#include "stdafx_shared.h"
#include "pragma/networking/enums.hpp"
#include <networkmanager/interface/nwm_manager.hpp>

std::string pragma::networking::drop_reason_to_string(DropReason reason)
{
	switch(reason)
	{
	case DropReason::Disconnected:
		return "User disconnected from server!";
	case DropReason::Timeout:
		return "User timed out!";
	case DropReason::Kicked:
		return "User was kicked from the server!";
	case DropReason::Shutdown:
		return "Server shut down!";
	case DropReason::Error:
	default:
		return "User has been dropped due to an error!";
	}
	static_assert(umath::to_integral(DropReason::Count) == 5);
}
nwm::Protocol pragma::networking::get_nwm_protocol(Protocol protocol)
{
	switch(protocol)
	{
	case Protocol::SlowReliable:
		return nwm::Protocol::TCP;
	case Protocol::FastUnreliable:
	default:
		return nwm::Protocol::UDP;
	}
	static_assert(umath::to_integral(Protocol::Count) == 2);
}
nwm::ClientDropped pragma::networking::get_nwm_drop_reason(DropReason reason)
{
	switch(reason)
	{
	case DropReason::Disconnected:
		return nwm::ClientDropped::Disconnected;
	case DropReason::Timeout:
		return nwm::ClientDropped::Timeout;
	case DropReason::Kicked:
		return nwm::ClientDropped::Kicked;
	case DropReason::Shutdown:
		return nwm::ClientDropped::Shutdown;
	case DropReason::Error:
	default:
		return nwm::ClientDropped::Error;
	}
	static_assert(umath::to_integral(DropReason::Count) == 5);
}
pragma::networking::DropReason pragma::networking::get_pragma_drop_reason(nwm::ClientDropped reason)
{
	switch(reason)
	{
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
