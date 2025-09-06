// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/game/s_game.h"
#include <stdexcept>

#undef GetClassName

module pragma.server.entities.registration;
import pragma.server.server_state;

using namespace server_entities;

void ServerEntityRegistry::RegisterEntity(const std::string &localName,
                                         std::type_index type,
                                         Factory creator)
{
    m_factories[type] = std::move(creator);
    m_classNameToTypeIndex.insert(std::make_pair(localName, type));
}

std::optional<std::string_view> ServerEntityRegistry::GetClassName(std::type_index type) const
{
    auto it = m_typeIndexToClassName.find(type);
    if (it == m_typeIndexToClassName.end())
        return {};
    return it->second;
}

void ServerEntityRegistry::GetRegisteredClassNames(std::vector<std::string> &outNames) const
{
    outNames.reserve(outNames.size() +m_classNameToTypeIndex.size());
    for (auto &[className, typeIndex] : m_classNameToTypeIndex)
        outNames.push_back(className);
}

Factory ServerEntityRegistry::FindFactory(const std::string &localName) const
{
    auto itType = m_classNameToTypeIndex.find(localName);
    if(itType == m_classNameToTypeIndex.end())
        return nullptr;
    return FindFactory(itType->second);
}

Factory ServerEntityRegistry::FindFactory(std::type_index type) const
{
    auto itFactory = m_factories.find(type);
    if (itFactory == m_factories.end())
        return nullptr;
    auto &factory = itFactory->second;
    return factory;
}

uint32_t ServerEntityRegistry::RegisterNetworkedEntity(std::type_index type)
{
    auto it = m_networkFactoryIDs.find(type);
    if (it != m_networkFactoryIDs.end())
        return it->second;
    uint32_t id = m_nextNetworkFactoryID++;
    m_networkFactoryIDs.emplace(type, id);
    return id;
}

std::optional<uint32_t> ServerEntityRegistry::GetNetworkFactoryID(std::type_index type) const
{
    auto it = m_networkFactoryIDs.find(type);
    if (it == m_networkFactoryIDs.end())
        return std::nullopt;
    return it->second;
}
