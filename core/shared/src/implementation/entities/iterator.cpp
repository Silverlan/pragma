// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.iterator;

std::size_t EntityContainer::Size() const { return ents.size(); }
pragma::ecs::BaseEntity *EntityContainer::At(std::size_t index) { return ents.at(index); }

std::size_t ComponentContainer::Size() const { return components.size(); }
pragma::ecs::BaseEntity *ComponentContainer::At(std::size_t index)
{
	auto *component = components.at(index);
	return (component != nullptr) ? &component->GetEntity() : nullptr;
}

/////////////////

EntityIteratorData::EntityIteratorData(pragma::Game &game) : game(game), entities(std::make_unique<EntityContainer>(game.GetBaseEntities(), game.GetBaseEntityCount())) {}
EntityIteratorData::EntityIteratorData(pragma::Game &game, const std::vector<pragma::BaseEntityComponent *> &components, std::size_t count) : game(game), entities(std::make_unique<ComponentContainer>(components, count)) {}
std::size_t EntityIteratorData::GetCount() const { return entities->Count(); }
bool EntityIteratorData::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) const
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
bool BaseEntityIterator::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) const { return m_iteratorData ? m_iteratorData->ShouldPass(ent, index) : false; }
std::size_t BaseEntityIterator::GetCount() const { return m_iteratorData ? m_iteratorData->GetCount() : 0; }
BaseEntityIterator &BaseEntityIterator::operator++()
{
	if(!m_iteratorData)
		return *this;
	auto numEnts = m_iteratorData->entities->Size();
	while((m_currentIndex = pragma::math::min(m_currentIndex + 1u, numEnts)) < numEnts) {
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
pragma::ecs::BaseEntity *BaseEntityIterator::operator*() { return operator->(); }
pragma::ecs::BaseEntity *BaseEntityIterator::operator->() { return m_iteratorData ? m_iteratorData->entities->At(m_currentIndex) : nullptr; }
bool BaseEntityIterator::operator==(const BaseEntityIterator &other)
{
	if(!m_iteratorData)
		return false;
	return m_iteratorData->entities.get() == other.m_iteratorData->entities.get() && m_currentIndex == other.m_currentIndex;
}
bool BaseEntityIterator::operator!=(const BaseEntityIterator &other) { return !operator==(other); }

/////////////////

pragma::ecs::EntityIterator::EntityIterator(Game &game, bool) : m_iteratorData(pragma::util::make_shared<EntityIteratorData>(game)) {}
pragma::ecs::EntityIterator::EntityIterator(Game &game, FilterFlags filterFlags) : EntityIterator(game, false)
{
	if(filterFlags != FilterFlags::None)
		AttachFilter<EntityIteratorFilterFlags>(filterFlags);
}
pragma::ecs::EntityIterator::EntityIterator(Game &game, ComponentId componentId, FilterFlags filterFlags)
{
	if(componentId != INVALID_COMPONENT_ID) {
		std::size_t count;
		auto &components = game.GetEntityComponentManager().GetComponents(componentId, count);
		m_iteratorData = pragma::util::make_shared<EntityIteratorData>(game, components, count); // AttachFilter<EntityIteratorFilterComponent>(componentId);
	}
	if(m_iteratorData == nullptr)
		return;
	if(filterFlags != FilterFlags::None)
		AttachFilter<EntityIteratorFilterFlags>(filterFlags);
}
pragma::ecs::EntityIterator::EntityIterator(Game &game, const std::string &componentName, FilterFlags filterFlags)
{
	auto componentId = INVALID_COMPONENT_ID;
	if(game.GetEntityComponentManager().GetComponentTypeId(componentName, componentId) == true && componentId != INVALID_COMPONENT_ID) {
		std::size_t count;
		auto &components = game.GetEntityComponentManager().GetComponents(componentId, count);
		m_iteratorData = pragma::util::make_shared<EntityIteratorData>(game, components, count);
	}
	if(m_iteratorData == nullptr)
		return;
	// AttachFilter<EntityIteratorFilterComponent>(componentName);
	if(filterFlags != FilterFlags::None)
		AttachFilter<EntityIteratorFilterFlags>(filterFlags);
}
std::size_t pragma::ecs::EntityIterator::GetCount() const { return m_iteratorData ? m_iteratorData->GetCount() : 0; }
BaseEntityIterator pragma::ecs::EntityIterator::begin() const { return BaseEntityIterator {m_iteratorData, false}; }
BaseEntityIterator pragma::ecs::EntityIterator::end() const { return BaseEntityIterator {m_iteratorData, true}; }
void pragma::ecs::EntityIterator::SetBaseComponentType(ComponentId componentId)
{
	if(!m_iteratorData)
		return;
	std::size_t count;
	auto &components = m_iteratorData->game.GetEntityComponentManager().GetComponents(componentId, count);
	m_iteratorData->entities = std::make_unique<ComponentContainer>(components, count);
}
void pragma::ecs::EntityIterator::SetBaseComponentType(std::type_index typeIndex)
{
	if(!m_iteratorData)
		return;
	auto componentId = INVALID_COMPONENT_ID;
	m_iteratorData->game.GetEntityComponentManager().GetComponentId(typeIndex, componentId);
	SetBaseComponentType(componentId);
}
void pragma::ecs::EntityIterator::SetBaseComponentType(const std::string &componentName)
{
	if(!m_iteratorData)
		return;
	auto &componentManager = m_iteratorData->game.GetEntityComponentManager();
	auto componentId = INVALID_COMPONENT_ID;
	componentManager.GetComponentTypeId(componentName, componentId);
	SetBaseComponentType(componentId);
}
