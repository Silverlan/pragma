/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __MASTER_SERVER_HPP__
#define __MASTER_SERVER_HPP__

#include "pragma/serverdefinitions.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <cinttypes>

namespace util {
	class Library;
};
namespace pragma::networking {
	struct GameServerInfo;
	class DLLSERVER MasterServerRegistration {
	  public:
		using SteamId = uint64_t;
		struct DLLSERVER CallbackEvents {
			std::function<void(SteamId, bool)> onAuthCompleted = nullptr;
		};

		static std::unique_ptr<MasterServerRegistration> Register(const util::Library &steamworksLibrary, const GameServerInfo &serverInfo);
		~MasterServerRegistration();

		void UpdateServerData() const;
		pragma::networking::GameServerInfo &GetServerInfo();

		void SetCallbackEvents(const CallbackEvents &callbackEvents);

		void AuthenticateAndAddClient(uint64_t steamId, std::vector<char> &token, const std::string &clientName);
		void DropClient(SteamId steamId);
		void SetClientName(SteamId steamId, const std::string &name);
		void SetClientScore(SteamId steamId, int32_t score);
	  private:
		MasterServerRegistration() = default;
		bool Initialize(const util::Library &library);
		bool (*m_register_server)(const pragma::networking::GameServerInfo &) = nullptr;
		void (*m_unregister_server)() = nullptr;
		void (*m_update_server_data)() = nullptr;
		void (*m_steamworks_get_server_data)(pragma::networking::GameServerInfo **) = nullptr;

		void (*m_steamworks_authenticate_and_add_client)(uint64_t, std::vector<char> &, const std::string &) = nullptr;
		void (*m_steamworks_drop_client)(uint64_t) = nullptr;
		void (*m_steamworks_set_client_name)(uint64_t, const std::string &) = nullptr;
		void (*m_steamworks_set_client_score)(uint64_t, int32_t) = nullptr;
		void (*m_steamworks_set_master_server_callback_events)(const CallbackEvents &) = nullptr;

		CallbackEvents m_callbackEvents = {};
	};
};

#endif
