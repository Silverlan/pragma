// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:networking.enums;

export import pragma.network_manager;

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

	CONSTEXPR_DLL_COMPAT uint32_t FSYS_SEARCH_RESOURCES = 4'096;

	enum class ServerQuery : uint32_t { First = 1500, Ping, Last };
};

export {
	namespace pragma {
		enum class SnapshotFlags : uint8_t { None = 0u, PhysicsData = 1u, ComponentData = PhysicsData << 1u };
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::SnapshotFlags)
};
