/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/c_baseentity.h"

entfactory_newglobal_def(DLLCLIENT,Client,CBaseEntity);

ClientEntityNetworkMap::ClientEntityNetworkMap()
	: m_factoryID(1)
{}

void ClientEntityNetworkMap::RegisterFactory(CBaseEntity*(*factory)(unsigned int))
{
	m_factories.insert(std::unordered_map<unsigned int,CBaseEntity*(*)(unsigned int)>::value_type(m_factoryID,factory));
	m_factoryID++;
}

void ClientEntityNetworkMap::GetFactories(std::unordered_map<unsigned int,CBaseEntity*(*)(unsigned int)> **factories) {*factories = &m_factories;}

CBaseEntity*(*ClientEntityNetworkMap::GetFactory(unsigned int ID))(unsigned int)
{
	std::unordered_map<unsigned int,CBaseEntity*(*)(unsigned int)>::iterator i = m_factories.find(ID);
	if(i == m_factories.end())
		return NULL;
	return i->second;
}

DLLCLIENT ClientEntityNetworkMap *g_ClEntityNetworkMap = NULL;
DLLCLIENT void LinkNetworkedEntityClient(CBaseEntity*(*factory)(unsigned int))
{
	if(g_ClEntityNetworkMap == NULL)
	{
		static ClientEntityNetworkMap map;
		g_ClEntityNetworkMap = &map;
	}
	g_ClEntityNetworkMap->RegisterFactory(factory);
}

#include "pragma/networking/networkedentities.h"
