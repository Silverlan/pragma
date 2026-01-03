// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.ai;

import :ai;
import :entities.components.character;

using namespace pragma;

DISPOSITION SAIComponent::GetDefaultDisposition()
{
	auto *charComponent = static_cast<SCharacterComponent *>(GetEntity().GetCharacterComponent().get());
	auto *faction = (charComponent != nullptr) ? charComponent->GetFaction() : nullptr;
	if(faction == nullptr)
		return DISPOSITION::NEUTRAL;
	return faction->GetDefaultDisposition();
}
void SAIComponent::SetRelationship(ecs::BaseEntity *ent, DISPOSITION disp, bool revert, int priority)
{
	if(ent == nullptr)
		return;
	ClearRelationship(ent);
	m_entityRelationships[math::to_integral(disp)].push_back(pragma::util::make_shared<NPCRelationship>(std::shared_ptr<EntityHandle>(new EntityHandle(ent->GetHandle())), priority));
	if(revert == true && ent->IsNPC()) {
		auto sAiComponent = ent->GetComponent<SAIComponent>();
		if(sAiComponent.expired() == false)
			sAiComponent.get()->SetRelationship(&GetEntity(), disp, false, priority);
	}
}
void SAIComponent::SetRelationship(EntityHandle &hEnt, DISPOSITION disp, bool revert, int priority)
{
	if(!hEnt.valid())
		return;
	SetRelationship(hEnt.get(), disp, revert, priority);
}
void SAIComponent::SetRelationship(std::string className, DISPOSITION disp, int priority)
{
	string::to_lower(className);
	ClearRelationship(className);
	m_classRelationships[math::to_integral(disp)].push_back(pragma::util::make_shared<NPCRelationship>(pragma::util::make_shared<std::string>(className), priority));
}
void SAIComponent::SetRelationship(Faction &faction, DISPOSITION disp, int priority)
{
	ClearRelationship(faction);
	m_factionRelationships[static_cast<int>(disp)].push_back(pragma::util::make_shared<NPCRelationship>(faction.shared_from_this(), priority));
}
void SAIComponent::ClearRelationships()
{
	auto num = math::to_integral(DISPOSITION::COUNT);
	for(auto i = decltype(num) {0}; i < num; ++i) {
		m_entityRelationships[i].clear();
		m_classRelationships[i].clear();
		m_factionRelationships[i].clear();
	}
}
void SAIComponent::ClearRelationship(ecs::BaseEntity *ent)
{
	if(ent == nullptr)
		return;
	for(auto &rels : m_entityRelationships) {
		auto it = std::find_if(rels.begin(), rels.end(), [ent](const std::shared_ptr<NPCRelationship> &rel) {
			auto ptrEntHandle = std::static_pointer_cast<EntityHandle>(rel->data);
			return (ptrEntHandle != nullptr && ptrEntHandle->valid() && (*ptrEntHandle).get() == ent) ? true : false;
		});
		if(it != rels.end())
			rels.erase(it);
	}
}
void SAIComponent::ClearRelationship(EntityHandle &hEnt)
{
	if(!hEnt.valid())
		return;
	ClearRelationship(hEnt.get());
}
void SAIComponent::ClearRelationship(std::string className)
{
	string::to_lower(className);
	for(auto &rels : m_entityRelationships) {
		auto it = std::find_if(rels.begin(), rels.end(), [&className](const std::shared_ptr<NPCRelationship> &rel) {
			auto ptrClassName = std::static_pointer_cast<std::string>(rel->data);
			return (ptrClassName != nullptr && *ptrClassName == className) ? true : false;
		});
		if(it != rels.end())
			rels.erase(it);
	}
}
void SAIComponent::ClearRelationship(Faction &faction)
{
	for(auto &rels : m_entityRelationships) {
		auto it = std::find_if(rels.begin(), rels.end(), [&faction](const std::shared_ptr<NPCRelationship> &rel) {
			auto ptrFaction = std::static_pointer_cast<Faction>(rel->data);
			return (ptrFaction != nullptr && *ptrFaction == faction) ? true : false;
		});
		if(it != rels.end())
			rels.erase(it);
	}
}
DISPOSITION SAIComponent::GetDisposition(EntityHandle &hEnt, int *priority)
{
	if(!hEnt.valid())
		return GetDefaultDisposition();
	return GetDisposition(hEnt.get(), priority);
}
DISPOSITION SAIComponent::GetDisposition(ecs::BaseEntity *ent, int *priority)
{
	if(ent == &GetEntity()) {
		if(priority != nullptr)
			*priority = 0;
		return DISPOSITION::LIKE;
	}
	//Faction *factionThis = GetFaction();
	auto bFoundFaction = false;
	int32_t prio = -1;
	auto disp = GetDisposition(ent->GetClass(), &prio);
	if(ent->IsNPC() || ent->IsPlayer()) {
		auto *charComponent = static_cast<SCharacterComponent *>(ent->GetCharacterComponent().get());
		auto *factionEnt = (charComponent != nullptr) ? charComponent->GetFaction() : nullptr;
		if(factionEnt != nullptr) {
			int32_t prioFaction;
			auto dispFaction = GetDisposition(*factionEnt, &prioFaction);
			if(prioFaction >= prio) {
				prio = prioFaction;
				disp = dispFaction;
			}
		}
	}
	auto bFound = false;
	for(auto i = decltype(m_entityRelationships.size()) {0}; i < m_entityRelationships.size(); ++i) {
		auto &rels = m_entityRelationships[i];
		for(auto &rel : rels) {
			auto ptrEntityHandle = std::static_pointer_cast<EntityHandle>(rel->data);
			if(ptrEntityHandle != nullptr && ptrEntityHandle->valid() && ptrEntityHandle->get() == ent) {
				bFound = true;
				if(bFoundFaction == false && rel->priority > prio) {
					disp = static_cast<DISPOSITION>(i);
					prio = rel->priority;
				}
				break;
			}
			if(bFound == true)
				break;
		}
	}
	if(priority != nullptr)
		*priority = prio;
	return disp;
}
DISPOSITION SAIComponent::GetDisposition(std::string className, int *priority)
{
	string::to_lower(className);
	auto *charComponent = static_cast<SCharacterComponent *>(GetEntity().GetCharacterComponent().get());
	auto *factionThis = (charComponent != nullptr) ? charComponent->GetFaction() : nullptr;
	auto bFoundFaction = false;
	int32_t prio = -1;
	auto disp = GetDefaultDisposition();
	if(factionThis != nullptr) {
		bFoundFaction = true;
		disp = factionThis->GetDisposition(className, &prio);
	}
	auto bFound = false;
	for(auto i = decltype(m_classRelationships.size()) {0}; i < m_classRelationships.size(); ++i) {
		auto &rels = m_classRelationships[i];
		for(auto &rel : rels) {
			auto ptrClass = std::static_pointer_cast<std::string>(rel->data);
			if(ptrClass != nullptr && *ptrClass == className) {
				bFound = true;
				if(bFoundFaction == false && rel->priority > prio) {
					disp = DISPOSITION(i);
					prio = rel->priority;
				}
				break;
			}
			if(bFound == true)
				break;
		}
	}
	if(priority != nullptr)
		*priority = prio;
	return disp;
}
DISPOSITION SAIComponent::GetDisposition(Faction &faction, int *priority)
{
	auto *charComponent = static_cast<SCharacterComponent *>(GetEntity().GetCharacterComponent().get());
	auto *factionThis = (charComponent != nullptr) ? charComponent->GetFaction() : nullptr;
	auto bFoundFaction = false;
	int32_t prio = -1;
	auto disp = GetDefaultDisposition();
	if(factionThis != nullptr) {
		bFoundFaction = true;
		disp = factionThis->GetDisposition(faction, &prio);
	}
	auto bFound = false;
	for(auto i = decltype(m_factionRelationships.size()) {0}; i < m_factionRelationships.size(); ++i) {
		auto &rels = m_factionRelationships[i];
		for(auto &rel : rels) {
			auto ptrFaction = std::static_pointer_cast<Faction>(rel->data);
			if(ptrFaction != nullptr && *ptrFaction == faction) {
				bFound = true;
				if(bFoundFaction == false && rel->priority > prio) {
					disp = DISPOSITION(i);
					prio = rel->priority;
				}
				break;
			}
			if(bFound == true)
				break;
		}
	}
	if(priority != nullptr)
		*priority = prio;
	return disp;
}
