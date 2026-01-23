// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.entity;
import :game;

using namespace pragma;

decltype(CEntityComponentManager::INVALID_COMPONENT) CEntityComponentManager::INVALID_COMPONENT = std::numeric_limits<decltype(INVALID_COMPONENT)>::max();
NetEventId CBaseNetComponent::FindNetEvent(const std::string &evName) const { return get_cgame()->FindNetEvent(evName); }

const std::vector<ComponentId> &CEntityComponentManager::GetServerComponentIdToClientComponentIdTable() const { return const_cast<CEntityComponentManager *>(this)->GetServerComponentIdToClientComponentIdTable(); }
std::vector<ComponentId> &CEntityComponentManager::GetServerComponentIdToClientComponentIdTable() { return m_svComponentToClComponentTable; }
