// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "mathutil/umath.h"
#include "networkmanager/interface/nwm_manager.hpp"
#include <cinttypes>
#include <string>

export module pragma.shared:networking.enums;

export namespace pragma::networking {
	using Port = uint16_t;
	enum class ErrorCode : uint32_t {
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
	enum class Protocol : uint8_t {
		FastUnreliable = 0,
		SlowReliable,

		Count
	};
	enum class DropReason : int8_t {
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

	uint32_t FSYS_SEARCH_RESOURCES = 4'096;

	enum class WVQuery : unsigned int { FIRST = 1500, PING, LAST };
};

export namespace pragma {
	enum class SnapshotFlags : uint8_t { None = 0u, PhysicsData = 1u, ComponentData = PhysicsData << 1u };
	REGISTER_BASIC_BITWISE_OPERATORS(SnapshotFlags);
};
