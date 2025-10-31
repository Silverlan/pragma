// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

#undef GetClassName

module pragma.client;

import :entities.registration;

using namespace client_entities;

void ClientEntityRegistry::RegisterEntity(const std::string &localName,
                                         std::type_index type,
                                         Factory creator)
{
    m_factories[type] = std::move(creator);
    m_classNameToTypeIndex.insert(std::make_pair(localName, type));
}

std::optional<std::string_view> ClientEntityRegistry::GetClassName(std::type_index type) const
{
    auto it = m_typeIndexToClassName.find(type);
    if (it == m_typeIndexToClassName.end())
        return {};
    return it->second;
}

void ClientEntityRegistry::GetRegisteredClassNames(std::vector<std::string> &outNames) const
{
    outNames.reserve(outNames.size() +m_classNameToTypeIndex.size());
    for (auto &[className, typeIndex] : m_classNameToTypeIndex)
        outNames.push_back(className);
}

Factory ClientEntityRegistry::FindFactory(const std::string &localName) const
{
    auto itType = m_classNameToTypeIndex.find(localName);
    if(itType == m_classNameToTypeIndex.end())
        return nullptr;
    return FindFactory(itType->second);
}

Factory ClientEntityRegistry::FindFactory(std::type_index type) const
{
    auto itFactory = m_factories.find(type);
    if (itFactory == m_factories.end())
        return nullptr;
    auto &factory = itFactory->second;
    return factory;
}

uint32_t ClientEntityRegistry::RegisterNetworkedEntity(const NetworkedFactory &factory)
{
    auto ID = m_nextNetworkFactoryID++;
    m_networkedFactories.insert(std::make_pair(ID, factory));
    return ID;
}

const NetworkedFactory *ClientEntityRegistry::GetNetworkedFactory(uint32_t ID) const
{
    auto it = m_networkedFactories.find(ID);
    if(it == m_networkedFactories.end())
        return nullptr;
    return &it->second;
}
