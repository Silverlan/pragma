// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:networking.master_server;

export import pragma.shared;

export namespace pragma::networking {
	class DLLSERVER MasterServerRegistration {
	  public:
		using SteamId = uint64_t;
		struct DLLSERVER CallbackEvents {
			std::function<void(SteamId, bool)> onAuthCompleted = nullptr;
		};

		static std::unique_ptr<MasterServerRegistration> Register(const util::Library &steamworksLibrary, const GameServerInfo &serverInfo);
		~MasterServerRegistration();

		void UpdateServerData() const;
		GameServerInfo &GetServerInfo();

		void SetCallbackEvents(const CallbackEvents &callbackEvents);

		void AuthenticateAndAddClient(uint64_t steamId, std::vector<char> &token, const std::string &clientName);
		void DropClient(SteamId steamId);
		void SetClientName(SteamId steamId, const std::string &name);
		void SetClientScore(SteamId steamId, int32_t score);
	  private:
		MasterServerRegistration() = default;
		bool Initialize(const util::Library &library);
		bool (*m_register_server)(const GameServerInfo &) = nullptr;
		void (*m_unregister_server)() = nullptr;
		void (*m_update_server_data)() = nullptr;
		void (*m_steamworks_get_server_data)(GameServerInfo **) = nullptr;

		void (*m_steamworks_authenticate_and_add_client)(uint64_t, std::vector<char> &, const std::string &) = nullptr;
		void (*m_steamworks_drop_client)(uint64_t) = nullptr;
		void (*m_steamworks_set_client_name)(uint64_t, const std::string &) = nullptr;
		void (*m_steamworks_set_client_score)(uint64_t, int32_t) = nullptr;
		void (*m_steamworks_set_master_server_callback_events)(const CallbackEvents &) = nullptr;

		CallbackEvents m_callbackEvents = {};
	};
};
