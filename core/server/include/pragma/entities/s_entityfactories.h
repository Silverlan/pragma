/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_ENTITYFACTORIES_H__
#define __S_ENTITYFACTORIES_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/entityfactories.h"
#include <pragma/serverstate/serverstate.h>
class SBaseEntity;
entfactory_newglobal_dec(DLLSERVER, Server, SBaseEntity);

// Entity Link Map
class SBaseEntity;
extern ServerState *server;
class SGame;
#define LINK_ENTITY_TO_CLASS(localname, classname)                                                                                                                                                                                                                                               \
	static SBaseEntity *CreateServer##classname##Entity()                                                                                                                                                                                                                                        \
	{                                                                                                                                                                                                                                                                                            \
		SGame *game = server->GetGameState();                                                                                                                                                                                                                                                    \
		if(game == NULL)                                                                                                                                                                                                                                                                         \
			return NULL;                                                                                                                                                                                                                                                                         \
		return static_cast<SBaseEntity *>(game->CreateEntity<classname>());                                                                                                                                                                                                                      \
	}                                                                                                                                                                                                                                                                                            \
	__reg_ent_Server *__reg_ent_Server_##classname = new __reg_ent_Server(#localname, typeid(classname), &CreateServer##classname##Entity);

// Network Link Map
class DLLSERVER ServerEntityNetworkMap {
  private:
	std::unordered_map<size_t, unsigned int> m_factoryIDs;
	unsigned int m_factoryID;
  public:
	ServerEntityNetworkMap();
	void GetFactoryIDs(std::unordered_map<size_t, unsigned int> **factories);
	void RegisterFactory(const std::type_info &info);
	unsigned int GetFactoryID(const std::type_info &info);
};

DLLSERVER void LinkNetworkedEntityServer(const std::type_info &info);

class DLLSERVER __reg_ent_link_sv {
  public:
	__reg_ent_link_sv(const std::type_info &info)
	{
		LinkNetworkedEntityServer(info);
		delete this;
	}
};
#define __LINK_NETWORKED_ENTITY(classname, identifier) __reg_ent_link_sv *__reg_ent_link_sv_##identifier = new __reg_ent_link_sv(typeid(classname));
#define _LINK_NETWORKED_ENTITY(classname, identifier) __LINK_NETWORKED_ENTITY(classname, identifier);
#define LINK_NETWORKED_ENTITY(classname, dummy) _LINK_NETWORKED_ENTITY(classname, __COUNTER__);

#endif
