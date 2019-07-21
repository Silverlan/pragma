#ifndef __PR_STEAM_NETWORKING_SHARED_HPP__
#define __PR_STEAM_NETWORKING_SHARED_HPP__

#undef ENABLE_STEAM_SERVER_SUPPORT
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifdef ENABLE_STEAM_SERVER_SUPPORT
#include <steam/steam_api.h>
#endif
#include <pragma/networking/enums.hpp>

// https://stackoverflow.com/a/7477384/2482983
// +5 for ports
constexpr size_t MAX_IP_CHAR_LENGTH = 45 +6;

inline int get_send_flags(pragma::networking::Protocol protocol)
{
	int sendFlags = k_nSteamNetworkingSend_NoNagle; // TODO: Allow caller to control nagle?
	switch(protocol)
	{
	case pragma::networking::Protocol::FastUnreliable:
		sendFlags |= k_nSteamNetworkingSend_Unreliable;
		break;
	case pragma::networking::Protocol::SlowReliable:
	default:
		sendFlags |= k_nSteamNetworkingSend_Reliable;
		break;
	}
	return sendFlags;
}

#endif
