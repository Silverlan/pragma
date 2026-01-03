// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.registration;

import :entities;
import :game;
import :server_state;

#undef GetClassName

export namespace server_entities {
	using Factory = std::function<SBaseEntity *(pragma::ServerState *)>;
	class ServerEntityRegistry {
	  public:
		static ServerEntityRegistry &Instance()
		{
			static ServerEntityRegistry inst;
			return inst;
		}

		void RegisterEntity(const std::string &localName, std::type_index type, Factory creator);
		Factory FindFactory(const std::string &localName) const;
		Factory FindFactory(std::type_index type) const;
		void GetRegisteredClassNames(std::vector<std::string> &outNames) const;
		std::optional<std::string_view> GetClassName(std::type_index type) const;
		template<class T>
		std::optional<std::string_view> GetClassName() const
		{
			return GetClassName(typeid(T));
		}

		uint32_t RegisterNetworkedEntity(std::type_index type);
		std::optional<uint32_t> GetNetworkFactoryID(std::type_index type) const;
	  private:
		ServerEntityRegistry() = default;
		~ServerEntityRegistry() = default;
		ServerEntityRegistry(const ServerEntityRegistry &) = delete;
		ServerEntityRegistry &operator=(const ServerEntityRegistry &) = delete;

		std::unordered_map<std::type_index, Factory> m_factories;
		std::unordered_map<std::string, std::type_index> m_classNameToTypeIndex;
		std::unordered_map<std::type_index, std::string> m_typeIndexToClassName;
		std::unordered_map<std::type_index, uint32_t> m_networkFactoryIDs;
		uint32_t m_nextNetworkFactoryID {1};
	};

	template<typename T>
	void register_entity(const char *localName)
	{
		ServerEntityRegistry::Instance().RegisterEntity(std::string(localName), typeid(T), [](pragma::ServerState *server) -> SBaseEntity * {
			if(!server)
				return nullptr;
			auto *game = pragma::ServerState::Get()->GetGameState();
			if(!game)
				return nullptr;
			auto *ent = game->template CreateEntity<T>();
			return static_cast<SBaseEntity *>(ent);
		});
	}

	template<typename T>
	uint32_t register_networked_entity()
	{
		return ServerEntityRegistry::Instance().RegisterNetworkedEntity(typeid(T));
	}

	void register_entities();
}
