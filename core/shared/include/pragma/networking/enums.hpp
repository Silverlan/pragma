/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __PRAGMA_NETWORKING_ENUMS_HPP__
#define __PRAGMA_NETWORKING_ENUMS_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <string>

namespace nwm {enum class Protocol : uint32_t; enum class ClientDropped : int8_t;};
namespace pragma::networking
{
	using Port = uint16_t;
	enum class ErrorCode : uint32_t
	{
		Success = 0,
		UnableToStartServer,
		UnableToConnect,
		UnableToAcceptClient,
		UnableToListenOnPort,
		ServerNotRunning,
		UnknownError,
		UnableToDropClient,
		InvalidClient,

		// Clientside
		InvalidConnectionHandle,
		UnableToInitializeClient,

		// Generic
		GenericError,
		UnableToSendPacket,
		MessageTooLarge,

		Count
	};
	enum class Protocol : uint8_t
	{
		FastUnreliable = 0,
		SlowReliable,

		Count
	};
	enum class DropReason : int8_t
	{
		Disconnected = 0,
		Timeout,
		Kicked,
		Shutdown,
		Error,
		AuthenticationFailed,

		Count
	};
	DLLNETWORK std::string drop_reason_to_string(DropReason reason);
	DLLNETWORK nwm::Protocol get_nwm_protocol(Protocol protocol);
	DLLNETWORK nwm::ClientDropped get_nwm_drop_reason(DropReason reason);
	DLLNETWORK DropReason get_pragma_drop_reason(nwm::ClientDropped reason);
};

#endif
