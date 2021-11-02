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

EntityURef::EntityURef(const EntityURef &other)
	: m_hEntity{other.m_hEntity},m_identifier{other.m_identifier ? std::make_unique<EntityIdentifier>(*other.m_identifier) : nullptr}
{}
EntityURef::EntityURef(EntityURef &&other)
	: m_hEntity{other.m_hEntity},m_identifier{other.m_identifier ? std::move(other.m_identifier) : nullptr}
{}
EntityURef::EntityURef(EntityIdentifier identifier)
{
	std::visit([this](auto &v) {
		if constexpr(std::is_same_v<decltype(v),util::Uuid>)
			m_identifier = (v != util::Uuid{}) ? std::make_unique<EntityIdentifier>(v) : nullptr;
		else
			m_identifier = std::make_unique<EntityIdentifier>(std::move(v));
	},identifier);
}
EntityURef &EntityURef::operator=(const EntityURef &other)
{
	m_hEntity = other.m_hEntity;
	m_identifier = {other.m_identifier ? std::make_unique<EntityIdentifier>(*other.m_identifier) : nullptr};
	return *this;
}
EntityURef &EntityURef::operator=(EntityURef &&other)
{
	m_hEntity = other.m_hEntity;
	m_identifier = {other.m_identifier ? std::move(other.m_identifier) : nullptr};
	return *this;
}
EntityURef::EntityURef(const BaseEntity &ent)
	: EntityURef{ent.GetUuid()}
{}
void EntityURef::AttachEntityFilter(EntityIterator &it,const EntityIdentifier &identifier)
{
	std::visit([&it](auto &val) {
		using T = decltype(val);
		if constexpr(std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>,util::Uuid>)
			it.AttachFilter<EntityIteratorFilterUuid>(val);
		else
			it.AttachFilter<EntityIteratorFilterNameOrClass>(val);
	},identifier);
}
BaseEntity *EntityURef::GetEntity(Game &game)
{
	if(!m_hEntity.IsValid())
	{
		if(!m_identifier)
			return nullptr;
		EntityIterator entIt {game};
		AttachEntityFilter(entIt,*m_identifier);
		auto it = entIt.begin();
		if(it == entIt.end())
			return nullptr;
		m_hEntity = it->GetHandle();
	}
	return m_hEntity.get();
}
bool EntityURef::HasEntityReference() const
{
	return m_identifier != nullptr;
}

//////////

EntityUComponentRef::EntityUComponentRef()
	: EntityUComponentRef{util::Uuid{},pragma::INVALID_COMPONENT_ID}
{}
EntityUComponentRef::EntityUComponentRef(EntityIdentifier identifier,ComponentId componentId)
	: EntityURef{std::move(identifier)},m_componentId{componentId}
{}
EntityUComponentRef::EntityUComponentRef(EntityIdentifier identifier,const std::string &componentName)
	: EntityURef{std::move(identifier)},m_componentName{componentName.empty() ? nullptr : std::make_unique<std::string>(componentName)}
{}
EntityUComponentRef::EntityUComponentRef(const BaseEntity &ent,ComponentId componentId)
	: EntityURef{ent},m_componentId{componentId}
{}
EntityUComponentRef::EntityUComponentRef(const BaseEntity &ent,const std::string &componentName)
	: EntityURef{ent},m_componentName{componentName.empty() ? nullptr : std::make_unique<std::string>(componentName)}
{}
EntityUComponentRef::EntityUComponentRef(const EntityUComponentRef &other)
{
	operator=(other);
}
EntityUComponentRef::EntityUComponentRef(EntityUComponentRef &&other)
{
	operator=(std::move(other));
}
EntityUComponentRef &EntityUComponentRef::operator=(const EntityUComponentRef &other)
{
	EntityURef::operator=(other);
	m_componentId = other.m_componentId;
	m_hComponent = other.m_hComponent;
	if(other.m_componentName)
		m_componentName = other.m_componentName ? std::make_unique<std::string>(*other.m_componentName) : nullptr;
	return *this;
}
EntityUComponentRef &EntityUComponentRef::operator=(EntityUComponentRef &&other)
{
	m_componentId = other.m_componentId;
	m_hComponent = other.m_hComponent;
	if(other.m_componentName)
		m_componentName = other.m_componentName ? std::move(other.m_componentName) : nullptr;
	EntityURef::operator=(std::move(other));
	return *this;
}
bool EntityUComponentRef::HasComponentReference() const
{
	return m_componentName != nullptr || m_componentId != INVALID_COMPONENT_ID;
}
BaseEntityComponent *EntityUComponentRef::GetComponent(Game &game)
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

EntityUComponentMemberRef::EntityUComponentMemberRef()
	: EntityUComponentMemberRef{util::Uuid{},pragma::INVALID_COMPONENT_ID,""}
{}
EntityUComponentMemberRef::EntityUComponentMemberRef(EntityIdentifier identifier,ComponentId componentId,const std::string &memberName)
	: EntityUComponentRef{std::move(identifier),componentId},m_memberRef{memberName}
{}
EntityUComponentMemberRef::EntityUComponentMemberRef(EntityIdentifier identifier,const std::string &componentName,const std::string &memberName)
	: EntityUComponentRef{std::move(identifier),componentName},m_memberRef{memberName}
{}
EntityUComponentMemberRef::EntityUComponentMemberRef(const BaseEntity &ent,ComponentId componentId,const std::string &memberName)
	: EntityUComponentRef{ent,componentId},m_memberRef{memberName}
{}
EntityUComponentMemberRef::EntityUComponentMemberRef(const BaseEntity &ent,const std::string &componentName,const std::string &memberName)
	: EntityUComponentRef{ent,componentName},m_memberRef{memberName}
{}
const ComponentMemberInfo *EntityUComponentMemberRef::GetMemberInfo(Game &game) const
{
	auto *c = GetComponent(game);
	if(!c)
		return nullptr;
	return m_memberRef.GetMemberInfo(*c);
}

bool EntityUComponentMemberRef::HasMemberReference() const
{
	return !m_memberRef.GetMemberName().empty();
}

////////////////////

MultiEntityURef::MultiEntityURef(EntityIdentifier identifier)
{
	std::visit([this](auto &v) {
		if constexpr(std::is_same_v<decltype(v),util::Uuid>)
			m_identifier = (v != util::Uuid{}) ? std::make_unique<EntityIdentifier>(v) : nullptr;
		else
			m_identifier = std::make_unique<EntityIdentifier>(std::move(v));
	},identifier);
}
MultiEntityURef::MultiEntityURef(const MultiEntityURef &other)
	: m_identifier{other.m_identifier ? std::make_unique<EntityIdentifier>(*other.m_identifier) : nullptr}
{}
MultiEntityURef::MultiEntityURef(MultiEntityURef &&other)
	: m_identifier{other.m_identifier ? std::move(other.m_identifier) : nullptr}
{}
MultiEntityURef &MultiEntityURef::operator=(const MultiEntityURef &other)
{
	m_identifier = {other.m_identifier ? std::make_unique<EntityIdentifier>(*other.m_identifier) : nullptr};
	return *this;
}
MultiEntityURef &MultiEntityURef::operator=(MultiEntityURef &&other)
{
	m_identifier = {other.m_identifier ? std::move(other.m_identifier) : nullptr};
	return *this;
}
MultiEntityURef::MultiEntityURef(const BaseEntity &ent)
	: MultiEntityURef{ent.GetUuid()}
{}
void MultiEntityURef::FindEntities(Game &game,std::vector<BaseEntity*> &outEnts) const
{
	if(!m_identifier)
		return;
	EntityIterator entIt {game};
	std::visit([&entIt](auto &val) {
		using T = decltype(val);
		if constexpr(std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>,util::Uuid>)
			entIt.AttachFilter<EntityIteratorFilterUuid>(val);
		else
			entIt.AttachFilter<EntityIteratorFilterNameOrClass>(val);
	},*m_identifier);
	for(auto *ent : entIt)
	{
		if(outEnts.size() == outEnts.capacity())
			outEnts.reserve(outEnts.size() *1.5 +5);
		outEnts.push_back(ent);
	}
}
bool MultiEntityURef::HasEntityReference() const
{
	return m_identifier != nullptr;
}

//////////

MultiEntityUComponentRef::MultiEntityUComponentRef()
	: MultiEntityUComponentRef{util::Uuid{},pragma::INVALID_COMPONENT_ID}
{}
MultiEntityUComponentRef::MultiEntityUComponentRef(EntityIdentifier identifier,ComponentId componentId)
	: MultiEntityURef{std::move(identifier)},m_componentId{componentId}
{}
MultiEntityUComponentRef::MultiEntityUComponentRef(EntityIdentifier identifier,const std::string &componentName)
	: MultiEntityURef{std::move(identifier)},m_componentName{componentName.empty() ? nullptr : std::make_unique<std::string>(componentName)}
{}
MultiEntityUComponentRef::MultiEntityUComponentRef(const BaseEntity &ent,ComponentId componentId)
	: MultiEntityURef{ent},m_componentId{componentId}
{}
MultiEntityUComponentRef::MultiEntityUComponentRef(const BaseEntity &ent,const std::string &componentName)
	: MultiEntityURef{ent},m_componentName{componentName.empty() ? nullptr : std::make_unique<std::string>(componentName)}
{}
MultiEntityUComponentRef::MultiEntityUComponentRef(const MultiEntityUComponentRef &other)
	: MultiEntityURef{other}
{
	operator=(other);
}
MultiEntityUComponentRef::MultiEntityUComponentRef(MultiEntityUComponentRef &&other)
{
	operator=(std::move(other));
}
MultiEntityUComponentRef &MultiEntityUComponentRef::operator=(const MultiEntityUComponentRef &other)
{
	MultiEntityURef::operator=(other);
	m_componentId = other.m_componentId;
	m_hComponent = other.m_hComponent;
	if(other.m_componentName)
		m_componentName = other.m_componentName ? std::make_unique<std::string>(*other.m_componentName) : nullptr;
	return *this;
}
MultiEntityUComponentRef &MultiEntityUComponentRef::operator=(MultiEntityUComponentRef &&other)
{
	m_componentId = other.m_componentId;
	m_hComponent = other.m_hComponent;
	if(other.m_componentName)
		m_componentName = other.m_componentName ? std::move(other.m_componentName) : nullptr;
	MultiEntityURef::operator=(std::move(other));
	return *this;
}
bool MultiEntityUComponentRef::HasComponentReference() const
{
	return m_componentName != nullptr || m_componentId != INVALID_COMPONENT_ID;
}
void MultiEntityUComponentRef::FindComponents(Game &game,std::vector<BaseEntityComponent*> &outComponents) const
{
	if(!m_identifier)
		return;
	EntityIterator entIt {game};
	if(m_componentId == INVALID_COMPONENT_ID)
	{
		if(!m_componentName)
			return; // Unreachable?
		entIt.AttachFilter<EntityIteratorFilterComponent>(*m_componentName);
		EntityURef::AttachEntityFilter(entIt,*m_identifier);
		for(auto *ent : entIt)
		{
			if(outComponents.size() == outComponents.capacity())
				outComponents.reserve(outComponents.size() *1.5 +5);
			outComponents.push_back(ent->FindComponent(*m_componentName).get());
		}
		return;
	}
	entIt.AttachFilter<EntityIteratorFilterComponent>(m_componentId);
	EntityURef::AttachEntityFilter(entIt,*m_identifier);
	for(auto *ent : entIt)
	{
		if(outComponents.size() == outComponents.capacity())
			outComponents.reserve(outComponents.size() *1.5 +5);
		outComponents.push_back(ent->FindComponent(m_componentId).get());
	}
}
