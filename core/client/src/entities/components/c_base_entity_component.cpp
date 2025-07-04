// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_entity_component.hpp"

extern DLLCLIENT CGame *c_game;

using namespace pragma;

decltype(CEntityComponentManager::INVALID_COMPONENT) CEntityComponentManager::INVALID_COMPONENT = std::numeric_limits<decltype(CEntityComponentManager::INVALID_COMPONENT)>::max();
NetEventId CBaseNetComponent::FindNetEvent(const std::string &evName) const { return c_game->FindNetEvent(evName); }

const std::vector<ComponentId> &CEntityComponentManager::GetServerComponentIdToClientComponentIdTable() const { return const_cast<CEntityComponentManager *>(this)->GetServerComponentIdToClientComponentIdTable(); }
std::vector<ComponentId> &CEntityComponentManager::GetServerComponentIdToClientComponentIdTable() { return m_svComponentToClComponentTable; }
