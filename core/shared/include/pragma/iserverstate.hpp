/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PR_ISERVERSTATE_HPP__
#define __PR_ISERVERSTATE_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_library.hpp>
#include <memory>
#include <optional>

class NetPacket;
class ServerState;
namespace util {
	class Library;
};
namespace pragma {
	struct DLLNETWORK IServerState final {
		void Initialize(::util::Library &lib);
		IServerState() = default;
		void (*create_server_state)(std::unique_ptr<ServerState> &) = nullptr;
		void (*start_server)(bool) = nullptr;
		void (*close_server)() = nullptr;
		bool (*is_server_running)() = nullptr;
		void (*get_server_steam_id)(std::optional<uint64_t> &) = nullptr;
		ServerState *(*get_server_state)() = nullptr;
		void (*clear_server_state)() = nullptr;
		void (*handle_local_host_player_server_packet)(NetPacket &) = nullptr;
		bool (*connect_local_host_player_client)() = nullptr;

		bool valid() const { return m_bValid; }
	  private:
		bool m_bValid = false;
	};
};

#endif
