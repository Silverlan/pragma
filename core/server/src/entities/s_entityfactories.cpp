/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/s_baseentity.h"

entfactory_newglobal_def(DLLSERVER,Server,SBaseEntity);

ServerEntityNetworkMap::ServerEntityNetworkMap()
	: m_factoryID(1)
{}

void ServerEntityNetworkMap::RegisterFactory(const std::type_info &info)
{
	m_factoryIDs.insert(std::unordered_map<size_t,unsigned int>::value_type(info.hash_code(),m_factoryID));
	m_factoryID++;
}

void ServerEntityNetworkMap::GetFactoryIDs(std::unordered_map<size_t,unsigned int> **factories) {*factories = &m_factoryIDs;}

unsigned int ServerEntityNetworkMap::GetFactoryID(const std::type_info &info)
{
	std::unordered_map<size_t,unsigned int>::iterator i = m_factoryIDs.find(info.hash_code());
	if(i == m_factoryIDs.end())
		return 0;
	return i->second;
}

DLLSERVER ServerEntityNetworkMap *g_SvEntityNetworkMap = NULL;
DLLSERVER void LinkNetworkedEntityServer(const std::type_info &info)
{
	if(g_SvEntityNetworkMap == NULL)
	{
		static ServerEntityNetworkMap map;
		g_SvEntityNetworkMap = &map;
	}
	g_SvEntityNetworkMap->RegisterFactory(info);
}

#include "pragma/networking/networkedentities.h"