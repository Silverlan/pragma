/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENTITY_UUID_REF_HPP__
#define __ENTITY_UUID_REF_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/types.hpp"
#include "pragma/entities/component_member_reference.hpp"
#include <sharedutils/util.h>

class Game;
namespace pragma
{
	struct DLLNETWORK EntityUuidRef
	{
		EntityUuidRef(util::Uuid uuid)
			: m_uuid{uuid}
		{}
		EntityUuidRef(const BaseEntity &ent);
		BaseEntity *GetEntity(Game &game);
		const BaseEntity *GetEntity(Game &game) const {return const_cast<EntityUuidRef*>(this)->GetEntity(game);}
	protected:
		util::Uuid m_uuid;
		EntityHandle m_hEntity;
	};

	struct DLLNETWORK EntityUuidComponentRef
		: EntityUuidRef
	{
		EntityUuidComponentRef(util::Uuid uuid,ComponentId componentId);
		EntityUuidComponentRef(util::Uuid uuid,const std::string &componentName);
		EntityUuidComponentRef(const BaseEntity &ent,ComponentId componentId);
		EntityUuidComponentRef(const BaseEntity &ent,const std::string &componentName);
		EntityUuidComponentRef(const EntityUuidComponentRef &other);
		EntityUuidComponentRef &operator=(const EntityUuidComponentRef &other);
		BaseEntityComponent *GetComponent(Game &game);
		const BaseEntityComponent *GetComponent(Game &game) const {return const_cast<EntityUuidComponentRef*>(this)->GetComponent(game);}
	protected:
		ComponentId m_componentId = INVALID_COMPONENT_ID;
		std::unique_ptr<std::string> m_componentName = nullptr;
		ComponentHandle<BaseEntityComponent> m_hComponent;
	};

	struct DLLNETWORK EntityUuidComponentMemberRef
		: EntityUuidComponentRef
	{
		EntityUuidComponentMemberRef(util::Uuid uuid,ComponentId componentId,const std::string &memberName);
		EntityUuidComponentMemberRef(util::Uuid uuid,const std::string &componentName,const std::string &memberName);
		EntityUuidComponentMemberRef(const BaseEntity &ent,ComponentId componentId,const std::string &memberName);
		EntityUuidComponentMemberRef(const BaseEntity &ent,const std::string &componentName,const std::string &memberName);
		EntityUuidComponentMemberRef(const EntityUuidComponentMemberRef&)=default;
		const ComponentMemberInfo *GetMemberInfo(Game &game) const;

		EntityUuidComponentMemberRef &operator=(const EntityUuidComponentMemberRef&)=default;
		ComponentMemberReference &operator*() {return m_memberRef;}
		const ComponentMemberReference &operator*() const {return const_cast<EntityUuidComponentMemberRef*>(this)->operator*();}
		ComponentMemberReference *operator->() {return &m_memberRef;}
		const ComponentMemberReference *operator->() const {return const_cast<EntityUuidComponentMemberRef*>(this)->operator->();}
	protected:
		ComponentMemberReference m_memberRef;
	};
};

#endif
