/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/lua_entity_iterator.hpp"

extern DLLNETWORK Engine *engine;

LuaBaseEntityIterator::LuaBaseEntityIterator(const BaseEntityIterator &iterator) : m_iterator(iterator) {}
LuaBaseEntityIterator::LuaBaseEntityIterator(const LuaBaseEntityIterator &other) : m_iterator(other.m_iterator) {}
LuaBaseEntityIterator &LuaBaseEntityIterator::operator++()
{
	++m_iterator;
	return *this;
}
LuaBaseEntityIterator LuaBaseEntityIterator::operator++(int) { return LuaBaseEntityIterator(m_iterator++); }
luabind::object LuaBaseEntityIterator::operator*()
{
	auto *ent = *m_iterator;
	return (ent != nullptr) ? ent->GetLuaObject() : luabind::object {};
}
luabind::object LuaBaseEntityIterator::operator->()
{
	auto *ent = *m_iterator;
	return (ent != nullptr) ? ent->GetLuaObject() : luabind::object {};
}
bool LuaBaseEntityIterator::operator==(const LuaBaseEntityIterator &other) { return m_iterator == other.m_iterator; }
bool LuaBaseEntityIterator::operator!=(const LuaBaseEntityIterator &other) { return m_iterator != other.m_iterator; }

////////////

LuaEntityIterator::LuaEntityIterator(lua_State *l, EntityIterator::FilterFlags filterFlags) : m_iterator(std::make_shared<EntityIterator>(*engine->GetNetworkState(l)->GetGameState(), filterFlags)) {}
LuaEntityIterator::LuaEntityIterator(lua_State *l, pragma::ComponentId componentId, EntityIterator::FilterFlags filterFlags) : m_iterator(std::make_shared<EntityIterator>(*engine->GetNetworkState(l)->GetGameState(), componentId, filterFlags)) {}
LuaEntityIterator::LuaEntityIterator(lua_State *l, const std::string &componentName, EntityIterator::FilterFlags filterFlags) : m_iterator(std::make_shared<EntityIterator>(*engine->GetNetworkState(l)->GetGameState(), componentName, filterFlags)) {}
LuaBaseEntityIterator LuaEntityIterator::begin() const { return LuaBaseEntityIterator(m_iterator->begin()); }
LuaBaseEntityIterator LuaEntityIterator::end() const { return LuaBaseEntityIterator(m_iterator->end()); }
void LuaEntityIterator::AttachFilter(LuaEntityIteratorFilterBase &filter) { filter.Attach(*m_iterator); }
EntityIterator &LuaEntityIterator::GetIterator() { return *m_iterator; }

////////////

CEntityComponentIterator::CEntityComponentIterator(Game &game, FilterFlags filterFlags) : EntityIterator {game, filterFlags} {}
CEntityComponentIterator::CEntityComponentIterator(Game &game, pragma::ComponentId componentId, FilterFlags filterFlags) : EntityIterator {game, componentId, filterFlags} {}
CEntityComponentIterator::CEntityComponentIterator(Game &game, const std::string &componentName, FilterFlags filterFlags) : EntityIterator {game, componentName, filterFlags} {}
CEntityComponentIterator::CEntityComponentIterator(std::vector<BaseEntity *> &ents) : m_ents(&ents) {}
CEntityComponentIterator &CEntityComponentIterator::operator++()
{
	while(++m_currentIndex < m_ents->size()) {
		auto *ent = m_ents->at(m_currentIndex);
		if(ent != nullptr && ent->HasComponent(m_componentId))
			break;
	}
	return *this;
}
CEntityComponentIterator CEntityComponentIterator::operator++(int)
{
	auto r = *this;
	operator++();
	return r;
}
pragma::BaseEntityComponent &CEntityComponentIterator::operator*() { return *operator->(); }
pragma::BaseEntityComponent *CEntityComponentIterator::operator->() { return m_ents->at(m_currentIndex)->FindComponent(m_componentId).get(); }
bool CEntityComponentIterator::operator==(const CEntityComponentIterator &other) { return m_ents == other.m_ents && m_currentIndex == other.m_currentIndex; }
bool CEntityComponentIterator::operator!=(const CEntityComponentIterator &other) { return !operator==(other); }

////////////

LuaEntityIteratorFilterName::LuaEntityIteratorFilterName(const std::string &name, bool caseSensitive, bool exactMatch) : m_name(name), m_bCaseSensitive(caseSensitive), m_bExactMatch(exactMatch) {}
void LuaEntityIteratorFilterName::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterName>(m_name, m_bCaseSensitive, m_bExactMatch); }

////////////

LuaEntityIteratorFilterModel::LuaEntityIteratorFilterModel(const std::string &mdlName) : m_modelName {mdlName} {}
void LuaEntityIteratorFilterModel::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterModel>(m_modelName); }

////////////

LuaEntityIteratorFilterUuid::LuaEntityIteratorFilterUuid(const std::string &uuid) : m_uuid {uuid} {}
void LuaEntityIteratorFilterUuid::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterUuid>(util::uuid_string_to_bytes(m_uuid)); }

////////////

LuaEntityIteratorFilterClass::LuaEntityIteratorFilterClass(const std::string &className, bool caseSensitive, bool exactMatch) : m_className(className), m_bCaseSensitive(caseSensitive), m_bExactMatch(exactMatch) {}
void LuaEntityIteratorFilterClass::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterClass>(m_className, m_bCaseSensitive, m_bExactMatch); }

////////////

LuaEntityIteratorFilterNameOrClass::LuaEntityIteratorFilterNameOrClass(const std::string &name, bool caseSensitive, bool exactMatch) : m_name(name), m_bCaseSensitive(caseSensitive), m_bExactMatch(exactMatch) {}
void LuaEntityIteratorFilterNameOrClass::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterNameOrClass>(m_name, m_bCaseSensitive, m_bExactMatch); }

////////////

LuaEntityIteratorFilterEntity::LuaEntityIteratorFilterEntity(const std::string &name) : m_name(name) {}
void LuaEntityIteratorFilterEntity::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterEntity>(m_name); }

////////////

LuaEntityIteratorFilterSphere::LuaEntityIteratorFilterSphere(const Vector3 &origin, float radius) : m_origin(origin), m_radius(radius) {}
void LuaEntityIteratorFilterSphere::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterSphere>(m_origin, m_radius); }

////////////

LuaEntityIteratorFilterBox::LuaEntityIteratorFilterBox(const Vector3 &min, const Vector3 &max) : m_min(min), m_max(max) {}
void LuaEntityIteratorFilterBox::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterBox>(m_min, m_max); }

////////////

LuaEntityIteratorFilterCone::LuaEntityIteratorFilterCone(const Vector3 &origin, const Vector3 &dir, float radius, float angle) : m_origin(origin), m_direction(dir), m_radius(radius), m_angle(angle) {}
void LuaEntityIteratorFilterCone::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterCone>(m_origin, m_direction, m_radius, m_angle); }

////////////

LuaEntityIteratorFilterComponent::LuaEntityIteratorFilterComponent(luabind::object) : m_componentId(pragma::INVALID_COMPONENT_ID) {}
LuaEntityIteratorFilterComponent::LuaEntityIteratorFilterComponent(pragma::ComponentId componentId) : m_componentId(componentId) {}
LuaEntityIteratorFilterComponent::LuaEntityIteratorFilterComponent(lua_State *l, const std::string &componentName) { engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetComponentTypeId(componentName, m_componentId); }
void LuaEntityIteratorFilterComponent::Attach(EntityIterator &iterator) { iterator.AttachFilter<EntityIteratorFilterComponent>(m_componentId); }

////////////

LuaBaseEntityComponentIterator::LuaBaseEntityComponentIterator(const BaseEntityIterator &iterator) : m_iterator(iterator) {}
LuaBaseEntityComponentIterator::LuaBaseEntityComponentIterator(const LuaBaseEntityComponentIterator &other) : m_iterator(other.m_iterator) {}
LuaBaseEntityComponentIterator &LuaBaseEntityComponentIterator::operator++()
{
	++m_iterator;
	return *this;
}
LuaBaseEntityComponentIterator LuaBaseEntityComponentIterator::operator++(int) { return LuaBaseEntityComponentIterator(m_iterator++); }
std::pair<BaseEntity *, pragma::BaseEntityComponent *> LuaBaseEntityComponentIterator::operator*()
{
	auto *data = m_iterator.GetIteratorData();
	if(!data)
		return {};
	auto *c = static_cast<ComponentContainer *>(data->entities.get())->components[m_iterator.GetCurrentIndex()];
	if(!c)
		return {nullptr, nullptr};
	return {&c->GetEntity(), c};
}
std::pair<BaseEntity *, pragma::BaseEntityComponent *> LuaBaseEntityComponentIterator::operator->() { return operator*(); }
bool LuaBaseEntityComponentIterator::operator==(const LuaBaseEntityComponentIterator &other) { return m_iterator == other.m_iterator; }
bool LuaBaseEntityComponentIterator::operator!=(const LuaBaseEntityComponentIterator &other) { return m_iterator != other.m_iterator; }

////////////

LuaEntityComponentIterator::LuaEntityComponentIterator(lua_State *l, pragma::ComponentId componentId, EntityIterator::FilterFlags filterFlags) : m_iterator(std::make_shared<CEntityComponentIterator>(*engine->GetNetworkState(l)->GetGameState(), componentId, filterFlags)) {}
LuaEntityComponentIterator::LuaEntityComponentIterator(lua_State *l, const std::string &componentName, EntityIterator::FilterFlags filterFlags) : m_iterator(std::make_shared<CEntityComponentIterator>(*engine->GetNetworkState(l)->GetGameState(), componentName, filterFlags)) {}
LuaBaseEntityComponentIterator LuaEntityComponentIterator::begin() const { return LuaBaseEntityComponentIterator(m_iterator->begin()); }
LuaBaseEntityComponentIterator LuaEntityComponentIterator::end() const { return LuaBaseEntityComponentIterator(m_iterator->end()); }
void LuaEntityComponentIterator::AttachFilter(LuaEntityIteratorFilterBase &filter) { filter.Attach(*m_iterator); }
EntityIterator &LuaEntityComponentIterator::GetIterator() { return *m_iterator; }
