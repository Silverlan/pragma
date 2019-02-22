#include "stdafx_client.h"
#include "pragma/entities/components/c_entity_component.hpp"

extern DLLCLIENT CGame *c_game;

using namespace pragma;

NetEventId CBaseNetComponent::FindNetEvent(const std::string &evName) const
{
	return c_game->FindNetEvent(evName);
}

const std::vector<ComponentId> &CEntityComponentManager::GetServerComponentIdToClientComponentIdTable() const {return const_cast<CEntityComponentManager*>(this)->GetServerComponentIdToClientComponentIdTable();}
std::vector<ComponentId> &CEntityComponentManager::GetServerComponentIdToClientComponentIdTable() {return m_svComponentToClComponentTable;}
