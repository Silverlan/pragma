// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientstate/clientstate.h"
#include "pragma/entities/c_baseentity.h"
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <string>
#include <mutex>
#include <optional>

#undef GetClassName

export module pragma.client.entities:registration;

export namespace client_entities {
    using Factory = std::function<CBaseEntity*(ClientState*)>;
    using NetworkedFactory = std::function<CBaseEntity*(ClientState*, uint32_t)>;
    class ClientEntityRegistry {
    public:
        static ClientEntityRegistry &Instance() {
            static ClientEntityRegistry inst;
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

        uint32_t RegisterNetworkedEntity(const NetworkedFactory &factory);
        const NetworkedFactory *GetNetworkedFactory(uint32_t ID) const;
    private:
        ClientEntityRegistry() = default;
        ~ClientEntityRegistry() = default;
        ClientEntityRegistry(const ClientEntityRegistry&) = delete;
        ClientEntityRegistry& operator=(const ClientEntityRegistry&) = delete;

        std::unordered_map<std::type_index, Factory> m_factories;
        std::unordered_map<std::string, std::type_index> m_classNameToTypeIndex;
        std::unordered_map<std::type_index, std::string> m_typeIndexToClassName;

        std::unordered_map<uint32_t, NetworkedFactory> m_networkedFactories;
        uint32_t m_nextNetworkFactoryID{1};
    };

    template<typename T>
    void register_entity(const char *localName) {
        ClientEntityRegistry::Instance().RegisterEntity(
            std::string(localName),
            typeid(T),
            [](ClientState *client) -> CBaseEntity* {
                if (!client) return
                    nullptr;
                CGame *game = client->GetGameState();
                if (!game) return
                    nullptr;
                auto *ent = game->template CreateEntity<T>();
                return static_cast<CBaseEntity*>(ent);
            }
        );
    }

    template<typename T>
    uint32_t register_networked_entity() {
        return ClientEntityRegistry::Instance().RegisterNetworkedEntity([](ClientState *client, uint32_t idx) -> CBaseEntity* {
            CGame *game = client->GetGameState();                                                                                                                                                                                                                                                    \
            if(game == NULL)                                                                                                                                                                                                                                                                         \
                return NULL;                                                                                                                                                                                                                                                                         \
            return static_cast<CBaseEntity *>(game->CreateEntity<T>(idx));
        });
    }
}
