/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/entity_component_manager.hpp"

std::size_t EntityContainer::Size() const { return ents.size(); }
BaseEntity *EntityContainer::At(std::size_t index) { return ents.at(index); }

std::size_t ComponentContainer::Size() const { return components.size(); }
BaseEntity *ComponentContainer::At(std::size_t index)
{
	auto *component = components.at(index);
	return (component != nullptr) ? &component->GetEntity() : nullptr;
}

/////////////////

EntityIteratorData::EntityIteratorData(Game &game) : game(game), entities(std::make_unique<EntityContainer>(game.GetBaseEntities(), game.GetBaseEntityCount())) {}
EntityIteratorData::EntityIteratorData(Game &game, const std::vector<pragma::BaseEntityComponent *> &components, std::size_t count) : game(game), entities(std::make_unique<ComponentContainer>(components, count)) {}
std::size_t EntityIteratorData::GetCount() const { return entities->Count(); }
bool EntityIteratorData::ShouldPass(BaseEntity &ent, std::size_t index) const
{
	for(auto &fFilter : filters) {
		if(fFilter->ShouldPass(ent, index) == false)
			return false;
	}
	return true;
}

/////////////////

BaseEntityIterator::BaseEntityIterator(const std::shared_ptr<EntityIteratorData> &itData, bool bEndIterator) : m_iteratorData(itData)
{
	if(!m_iteratorData) {
		m_currentIndex = 0;
		return;
	}
	if(bEndIterator == true)
		m_currentIndex = m_iteratorData->entities->Size();
	else {
		auto *ent = (m_currentIndex < m_iteratorData->entities->Size()) ? m_iteratorData->entities->At(m_currentIndex) : nullptr;
		if(ent == nullptr || ShouldPass(*ent, m_currentIndex) == false)
			++(*this);
	}
}
bool BaseEntityIterator::ShouldPass(BaseEntity &ent, std::size_t index) const { return m_iteratorData ? m_iteratorData->ShouldPass(ent, index) : false; }
std::size_t BaseEntityIterator::GetCount() const { return m_iteratorData ? m_iteratorData->GetCount() : 0; }
BaseEntityIterator &BaseEntityIterator::operator++()
{
	if(!m_iteratorData)
		return *this;
	auto numEnts = m_iteratorData->entities->Size();
	while((m_currentIndex = umath::min(m_currentIndex + 1u, numEnts)) < numEnts) {
		auto *ent = m_iteratorData->entities->At(m_currentIndex);
		if(ent != nullptr && ShouldPass(*ent, m_currentIndex))
			break;
	}
	return *this;
}
BaseEntityIterator BaseEntityIterator::operator++(int)
{
	auto r = *this;
	operator++();
	return r;
}
BaseEntity *BaseEntityIterator::operator*() { return operator->(); }
BaseEntity *BaseEntityIterator::operator->() { return m_iteratorData ? m_iteratorData->entities->At(m_currentIndex) : nullptr; }
bool BaseEntityIterator::operator==(const BaseEntityIterator &other)
{
	if(!m_iteratorData)
		return false;
	return m_iteratorData->entities.get() == other.m_iteratorData->entities.get() && m_currentIndex == other.m_currentIndex;
}
bool BaseEntityIterator::operator!=(const BaseEntityIterator &other) { return !operator==(other); }

/////////////////

EntityIterator::EntityIterator(Game &game, bool) : m_iteratorData(std::make_shared<EntityIteratorData>(game)) {}
EntityIterator::EntityIterator(Game &game, FilterFlags filterFlags) : EntityIterator(game, false)
{
	if(filterFlags != FilterFlags::None)
		AttachFilter<EntityIteratorFilterFlags>(filterFlags);
}
EntityIterator::EntityIterator(Game &game, pragma::ComponentId componentId, FilterFlags filterFlags)
{
	if(componentId != pragma::INVALID_COMPONENT_ID) {
		std::size_t count;
		auto &components = game.GetEntityComponentManager().GetComponents(componentId, count);
		m_iteratorData = std::make_shared<EntityIteratorData>(game, components, count); // AttachFilter<EntityIteratorFilterComponent>(componentId);
	}
	if(m_iteratorData == nullptr)
		return;
	if(filterFlags != FilterFlags::None)
		AttachFilter<EntityIteratorFilterFlags>(filterFlags);
}
EntityIterator::EntityIterator(Game &game, const std::string &componentName, FilterFlags filterFlags)
{
	auto componentId = pragma::INVALID_COMPONENT_ID;
	if(game.GetEntityComponentManager().GetComponentTypeId(componentName, componentId) == true && componentId != pragma::INVALID_COMPONENT_ID) {
		std::size_t count;
		auto &components = game.GetEntityComponentManager().GetComponents(componentId, count);
		m_iteratorData = std::make_shared<EntityIteratorData>(game, components, count);
	}
	if(m_iteratorData == nullptr)
		return;
	// AttachFilter<EntityIteratorFilterComponent>(componentName);
	if(filterFlags != FilterFlags::None)
		AttachFilter<EntityIteratorFilterFlags>(filterFlags);
}
std::size_t EntityIterator::GetCount() const { return m_iteratorData ? m_iteratorData->GetCount() : 0; }
BaseEntityIterator EntityIterator::begin() const { return BaseEntityIterator {m_iteratorData, false}; }
BaseEntityIterator EntityIterator::end() const { return BaseEntityIterator {m_iteratorData, true}; }
void EntityIterator::SetBaseComponentType(pragma::ComponentId componentId)
{
	if(!m_iteratorData)
		return;
	std::size_t count;
	auto &components = m_iteratorData->game.GetEntityComponentManager().GetComponents(componentId, count);
	m_iteratorData->entities = std::make_unique<ComponentContainer>(components, count);
}
void EntityIterator::SetBaseComponentType(std::type_index typeIndex)
{
	if(!m_iteratorData)
		return;
	auto componentId = pragma::INVALID_COMPONENT_ID;
	m_iteratorData->game.GetEntityComponentManager().GetComponentId(typeIndex, componentId);
	SetBaseComponentType(componentId);
}
void EntityIterator::SetBaseComponentType(const std::string &componentName)
{
	if(!m_iteratorData)
		return;
	auto &componentManager = m_iteratorData->game.GetEntityComponentManager();
	auto componentId = pragma::INVALID_COMPONENT_ID;
	componentManager.GetComponentTypeId(componentName, componentId);
	SetBaseComponentType(componentId);
}
