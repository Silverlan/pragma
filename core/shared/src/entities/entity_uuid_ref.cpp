/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/entity_uuid_ref.hpp"
#include "pragma/entities/entity_iterator.hpp"

using namespace pragma;

EntityUuidRef::EntityUuidRef(const BaseEntity &ent)
	: EntityUuidRef{ent.GetUuid()}
{}
BaseEntity *EntityUuidRef::GetEntity(Game &game)
{
	if(!m_hEntity.IsValid())
	{
		EntityIterator entIt {game};
		entIt.AttachFilter<EntityIteratorFilterUuid>(m_uuid);
		auto it = entIt.begin();
		if(it == entIt.end())
			return nullptr;
		m_hEntity = it->GetHandle();
	}
	return m_hEntity.get();
}

//////////

EntityUuidComponentRef::EntityUuidComponentRef(util::Uuid uuid,ComponentId componentId)
	: EntityUuidRef{uuid},m_componentId{componentId}
{}
EntityUuidComponentRef::EntityUuidComponentRef(util::Uuid uuid,const std::string &componentName)
	: EntityUuidRef{uuid},m_componentName{std::make_unique<std::string>(componentName)}
{}
EntityUuidComponentRef::EntityUuidComponentRef(const BaseEntity &ent,ComponentId componentId)
	: EntityUuidRef{ent},m_componentId{componentId}
{}
EntityUuidComponentRef::EntityUuidComponentRef(const BaseEntity &ent,const std::string &componentName)
	: EntityUuidRef{ent},m_componentName{std::make_unique<std::string>(componentName)}
{}
EntityUuidComponentRef::EntityUuidComponentRef(const EntityUuidComponentRef &other)
	: EntityUuidRef{other}
{
	operator=(other);
}
EntityUuidComponentRef &EntityUuidComponentRef::operator=(const EntityUuidComponentRef &other)
{
	m_componentId = other.m_componentId;
	m_hComponent = other.m_hComponent;
	if(other.m_componentName)
		m_componentName = std::make_unique<std::string>(*other.m_componentName);
	return *this;
}
BaseEntityComponent *EntityUuidComponentRef::GetComponent(Game &game)
{
	if(!m_hComponent.IsValid())
	{
		if(m_componentId == INVALID_COMPONENT_ID)
		{
			if(!m_componentName)
				return nullptr; // Unreachable?
			if(!game.GetEntityComponentManager().GetComponentTypeId(*m_componentName,m_componentId))
				return nullptr; // Unknown component, may not have been registered yet
			m_componentName = nullptr;
		}
		auto *ent = GetEntity(game);
		if(!ent)
			return nullptr;
		m_hComponent = ent->FindComponent(m_componentId);
	}
	return m_hComponent.get();
}

//////////

EntityUuidComponentMemberRef::EntityUuidComponentMemberRef(util::Uuid uuid,ComponentId componentId,const std::string &memberName)
	: EntityUuidComponentRef{uuid,componentId},m_memberRef{memberName}
{}
EntityUuidComponentMemberRef::EntityUuidComponentMemberRef(util::Uuid uuid,const std::string &componentName,const std::string &memberName)
	: EntityUuidComponentRef{uuid,componentName},m_memberRef{memberName}
{}
EntityUuidComponentMemberRef::EntityUuidComponentMemberRef(const BaseEntity &ent,ComponentId componentId,const std::string &memberName)
	: EntityUuidComponentRef{ent,componentId},m_memberRef{memberName}
{}
EntityUuidComponentMemberRef::EntityUuidComponentMemberRef(const BaseEntity &ent,const std::string &componentName,const std::string &memberName)
	: EntityUuidComponentRef{ent,componentName},m_memberRef{memberName}
{}
const ComponentMemberInfo *EntityUuidComponentMemberRef::GetMemberInfo(Game &game) const
{
	auto *c = GetComponent(game);
	if(!c)
		return nullptr;
	return m_memberRef.GetMemberInfo(*c);
}
