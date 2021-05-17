/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/composite_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <sharedutils/util_hash.hpp>
#include <unordered_set>
#include <udm.hpp>

using namespace pragma;

ComponentEventId CompositeComponent::EVENT_ON_ENTITY_ADDED = INVALID_COMPONENT_ID;
ComponentEventId CompositeComponent::EVENT_ON_ENTITY_REMOVED = INVALID_COMPONENT_ID;
void CompositeComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_ENTITY_ADDED = componentManager.RegisterEvent("ON_COMPOSITE_ENTITY_ADDED");
	EVENT_ON_ENTITY_REMOVED = componentManager.RegisterEvent("ON_COMPOSITE_ENTITY_REMOVED");
}

CompositeComponent::CompositeComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void CompositeComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}

void CompositeComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	for(auto &hEnt : m_ents)
	{
		if(!hEnt.IsValid())
			continue;
		hEnt->RemoveSafely();
	}
}

std::vector<EntityHandle>::const_iterator CompositeComponent::FindEntity(BaseEntity &ent) const
{
	return std::find_if(m_ents.begin(),m_ents.end(),[&ent](const EntityHandle &hEnt) {
		return hEnt.get() == &ent;
	});
}
void CompositeComponent::AddEntity(BaseEntity &ent)
{
	if(FindEntity(ent) != m_ents.end())
		return;
	m_ents.push_back(ent.GetHandle());
	BroadcastEvent(EVENT_ON_ENTITY_ADDED,CECompositeEntityChanged{ent});
}
void CompositeComponent::RemoveEntity(BaseEntity &ent)
{
	auto it = FindEntity(ent);
	if(it == m_ents.end())
		return;
	m_ents.erase(it);
	BroadcastEvent(EVENT_ON_ENTITY_REMOVED,CECompositeEntityChanged{ent});
}
const std::vector<EntityHandle> &CompositeComponent::GetEntities() const {return m_ents;}

luabind::object CompositeComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CompositeComponentHandleWrapper>(l);}

void CompositeComponent::Save(udm::LinkedPropertyWrapper &udm)
{
	BaseEntityComponent::Save(udm);
	std::vector<std::string> ents;
	ents.reserve(m_ents.size());
	for(auto &hEnt : m_ents)
		ents.push_back(util::uuid_to_string(hEnt.get()->GetUuid()));
	udm["entities"] = ents;
}
void CompositeComponent::Load(udm::LinkedPropertyWrapper &udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);

	std::vector<std::string> ents;
	udm["entities"](ents);
	m_ents.reserve(ents.size());

	auto toHash = [](const util::Uuid &uuid) -> util::Hash {
		return util::hash_combine<uint64_t>(util::hash_combine<uint64_t>(0,uuid[0]),uuid[1]);
	};
	std::unordered_set<util::Hash> set;
	for(auto uuid : ents)
		set.insert(toHash(util::uuid_string_to_bytes(uuid)));
	EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(),EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	for(auto *ent : entIt)
	{
		auto it = set.find(toHash(ent->GetUuid()));
		if(it == set.end())
			continue;
		m_ents.push_back(ent->GetHandle());
	}
}

CECompositeEntityChanged::CECompositeEntityChanged(BaseEntity &ent)
	: ent{ent}
{}
void CECompositeEntityChanged::PushArguments(lua_State *l)
{
	ent.GetLuaObject()->push(l);
}
