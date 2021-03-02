/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENTITYFACTORIES_H__
#define __C_ENTITYFACTORIES_H__
#include "pragma/entities/entityfactories.h"
#include "pragma/clientdefinitions.h"
#include "pragma/clientstate/clientstate.h"
class CBaseEntity;
entfactory_newglobal_dec(DLLCLIENT,Client,CBaseEntity);

// Entity Link Map
extern DLLCLIENT ClientState *client;
class CGame;
#define LINK_ENTITY_TO_CLASS(localname,classname) \
	static CBaseEntity *CreateClient##classname##Entity() \
	{ \
		CGame *game = client->GetGameState(); \
		if(game == NULL) \
			return NULL; \
		return static_cast<CBaseEntity*>(game->CreateEntity<classname>()); \
	} \
	__reg_ent_Client *__reg_ent_Client_##classname = new __reg_ent_Client(#localname,typeid(classname),&CreateClient##classname##Entity);

// Network Link Map
class DLLCLIENT ClientEntityNetworkMap
{
private:
	std::unordered_map<unsigned int,CBaseEntity*(*)(unsigned int)> m_factories;
	unsigned int m_factoryID;
public:
	ClientEntityNetworkMap();
	void GetFactories(std::unordered_map<unsigned int,CBaseEntity*(*)(unsigned int)> **factories);
	void RegisterFactory(CBaseEntity*(*factory)(unsigned int));
	CBaseEntity*(*GetFactory(unsigned int ID))(unsigned int);
};

DLLCLIENT void LinkNetworkedEntityClient(CBaseEntity*(*factory)(unsigned int));

class DLLCLIENT __reg_ent_link_cl
{
public:
	__reg_ent_link_cl(CBaseEntity*(*factory)(unsigned int))
	{
		LinkNetworkedEntityClient(factory);
		delete this;
	}
};
#define __LINK_NETWORKED_ENTITY(classname,identifier) \
	static CBaseEntity *CreateClient##classname##EntityLinked(unsigned int idx) \
	{ \
		CGame *game = client->GetGameState(); \
		if(game == NULL) \
			return NULL; \
		return static_cast<CBaseEntity*>(game->CreateEntity<classname>(idx)); \
	} \
	__reg_ent_link_cl *__reg_ent_link_cl_##identifier = new __reg_ent_link_cl(&CreateClient##classname##EntityLinked);
#define _LINK_NETWORKED_ENTITY(classname,identifier) \
	__LINK_NETWORKED_ENTITY(classname,identifier);
#define LINK_NETWORKED_ENTITY(svclassname,classname) \
	_LINK_NETWORKED_ENTITY(classname,__COUNTER__);

#endif