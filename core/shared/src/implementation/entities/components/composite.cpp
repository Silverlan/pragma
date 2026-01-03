// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.composite;

using namespace pragma::ecs;
using namespace compositeComponent;

CompositeGroup::CompositeGroup(CompositeComponent &compositeC, const std::string &name) : m_compositeComponent {&compositeC}, m_groupName {name} {}
CompositeGroup::~CompositeGroup() { ClearEntities(false); }
std::unordered_map<CompositeGroup::UuidHash, EntityHandle>::const_iterator CompositeGroup::FindEntity(BaseEntity &ent) const
{
	auto hash = util::get_uuid_hash(ent.GetUuid());
	return m_ents.find(hash);
}
void CompositeGroup::AddEntity(BaseEntity &ent)
{
	if(FindEntity(ent) != m_ents.end())
		return;
	m_ents[util::get_uuid_hash(ent.GetUuid())] = ent.GetHandle();
	m_compositeComponent->BroadcastEvent(EVENT_ON_ENTITY_ADDED, events::CECompositeEntityChanged {*this, ent});
}
void CompositeGroup::RemoveEntity(BaseEntity &ent)
{
	auto it = FindEntity(ent);
	if(it == m_ents.end())
		return;
	m_ents.erase(it);
	m_compositeComponent->BroadcastEvent(EVENT_ON_ENTITY_REMOVED, events::CECompositeEntityChanged {*this, ent});
}
CompositeGroup *CompositeGroup::FindChildGroup(const std::string &name)
{
	auto it = std::find_if(m_childGroups.begin(), m_childGroups.end(), [&name](const std::unique_ptr<CompositeGroup> &cg) { return cg->GetGroupName() == name; });
	return (it != m_childGroups.end()) ? it->get() : nullptr;
}
CompositeGroup &CompositeGroup::AddChildGroup(const std::string &groupName)
{
	auto *group = FindChildGroup(groupName);
	if(group)
		return *group;
	m_childGroups.push_back(std::make_unique<CompositeGroup>(*m_compositeComponent, groupName));
	m_childGroups.back()->m_parent = this;
	return *m_childGroups.back();
}
void CompositeGroup::ClearEntities(bool safely)
{
	auto ents = std::move(m_ents);
	for(auto &pair : ents) {
		if(!pair.second.valid())
			continue;
		if(safely) {
			auto compositeC = pair.second->GetComponent<CompositeComponent>();
			if(compositeC.valid())
				compositeC->ClearEntities(safely);
			pair.second->RemoveSafely();
		}
		else
			pair.second->Remove();
	}
	m_ents.clear();

	for(auto &childGroup : m_childGroups)
		childGroup->ClearEntities(safely);
}

////////////////

void CompositeComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	compositeComponent::EVENT_ON_ENTITY_ADDED = registerEvent("ON_COMPOSITE_ENTITY_ADDED", ComponentEventInfo::Type::Broadcast);
	compositeComponent::EVENT_ON_ENTITY_REMOVED = registerEvent("ON_COMPOSITE_ENTITY_REMOVED", ComponentEventInfo::Type::Broadcast);
}

CompositeComponent::CompositeComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_rootGroup {std::make_unique<CompositeGroup>(*this, "root")} {}
void CompositeComponent::Initialize() { BaseEntityComponent::Initialize(); }

void CompositeComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	m_rootGroup = nullptr;
}

void CompositeComponent::ClearEntities(bool safely) { m_rootGroup->ClearEntities(safely); }

void CompositeComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

static void write_group(udm::LinkedPropertyWrapperArg udmGroup, const CompositeGroup &group)
{
	std::vector<std::string> ents;
	auto &groupEnts = group.GetEntities();
	ents.reserve(groupEnts.size());
	for(auto &pair : groupEnts) {
		if(!pair.second.valid())
			continue;
		ents.push_back(pragma::util::uuid_to_string(pair.second.get()->GetUuid()));
	}
	udmGroup["entities"] = ents;

	auto udmChildren = udmGroup["children"];
	for(auto &child : group.GetChildGroups())
		write_group(udmChildren[child->GetGroupName()], *child);
}
void CompositeComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	write_group(udm["rootGroup"], *m_rootGroup);
}
static void read_group(BaseEntity &ent, udm::LinkedPropertyWrapperArg udmGroup, CompositeGroup &group)
{
	std::vector<std::string> ents;
	udmGroup["entities"](ents);

	auto &groupEnts = group.GetEntities();
	groupEnts.reserve(ents.size());
	auto toHash = [](const pragma::util::Uuid &uuid) -> pragma::util::Hash { return pragma::util::hash_combine<uint64_t>(pragma::util::hash_combine<uint64_t>(0, uuid[0]), uuid[1]); };
	std::unordered_set<pragma::util::Hash> set;
	for(auto uuid : ents)
		set.insert(toHash(pragma::util::uuid_string_to_bytes(uuid)));
	EntityIterator entIt {*ent.GetNetworkState()->GetGameState(), EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	for(auto *ent : entIt) {
		auto it = set.find(toHash(ent->GetUuid()));
		if(it == set.end())
			continue;
		groupEnts[pragma::util::get_uuid_hash(ent->GetUuid())] = ent->GetHandle();
	}

	auto udmChildren = udmGroup["children"];
	for(auto &pair : udmChildren.ElIt()) {
		auto &udmChildGroup = pair.property;
		read_group(ent, udmChildGroup, group.AddChildGroup(std::string {pair.key}));
	}
}
void CompositeComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	read_group(GetEntity(), udm["rootGroup"], *m_rootGroup);
}

events::CECompositeEntityChanged::CECompositeEntityChanged(CompositeGroup &group, BaseEntity &ent) : ent {ent}, group {group} {}
void events::CECompositeEntityChanged::PushArguments(lua::State *l)
{
	Lua::Push<CompositeGroup *>(l, &group);
	ent.GetLuaObject().push(l);
}
