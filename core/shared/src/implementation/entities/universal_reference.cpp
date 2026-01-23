// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.universal_reference;

using namespace pragma;

EntityURef::EntityURef(const EntityURef &other) : m_hEntity {other.m_hEntity}, m_identifier {other.m_identifier ? std::make_unique<EntityIdentifier>(*other.m_identifier) : nullptr} {}
EntityURef::EntityURef(EntityURef &&other) : m_hEntity {other.m_hEntity}, m_identifier {other.m_identifier ? std::move(other.m_identifier) : nullptr} {}
EntityURef::EntityURef(EntityIdentifier identifier)
{
	std::visit(
	  [this](auto &v) {
		  if constexpr(std::is_same_v<decltype(v), util::Uuid>)
			  m_identifier = (v != util::Uuid {}) ? std::make_unique<EntityIdentifier>(v) : nullptr;
		  else
			  m_identifier = std::make_unique<EntityIdentifier>(std::move(v));
	  },
	  identifier);
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
EntityURef::EntityURef(const ecs::BaseEntity &ent) : EntityURef {ent.GetUuid()} {}
void EntityURef::AttachEntityFilter(ecs::EntityIterator &it, const EntityIdentifier &identifier)
{
	std::visit(
	  [&it](auto &val) {
		  using T = decltype(val);
		  if constexpr(std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>, util::Uuid>)
			  it.AttachFilter<EntityIteratorFilterUuid>(val);
		  else
			  it.AttachFilter<EntityIteratorFilterNameOrClass>(val);
	  },
	  identifier);
}
ecs::BaseEntity *EntityURef::GetEntity(Game &game)
{
	if(!m_hEntity.IsValid()) {
		if(!m_identifier)
			return nullptr;
		ecs::EntityIterator entIt {game, ecs::EntityIterator::FilterFlags::Any};
		AttachEntityFilter(entIt, *m_identifier);
		auto it = entIt.begin();
		if(it == entIt.end())
			return nullptr;
		m_hEntity = it->GetHandle();
	}
	return m_hEntity.get();
}
std::optional<util::Uuid> EntityURef::GetUuid() const
{
	if(!m_identifier)
		return {};
	return std::visit(
	  [](auto &val) -> std::optional<util::Uuid> {
		  using T = decltype(val);
		  if constexpr(std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>, util::Uuid>)
			  return val;
		  return {};
	  },
	  *m_identifier);
}
std::optional<std::string> EntityURef::GetClassOrName() const
{
	if(!m_identifier)
		return {};
	return std::visit(
	  [](auto &val) -> std::optional<std::string> {
		  using T = decltype(val);
		  if constexpr(!std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>, util::Uuid>)
			  return val;
		  return {};
	  },
	  *m_identifier);
}
bool EntityURef::HasEntityReference() const { return m_identifier != nullptr; }
void EntityURef::ToStringArgs(std::stringstream &outSs) const
{
	outSs << "[ID:";
	if(!m_identifier)
		outSs << "NULL]";
	else {
		std::visit(
		  [&outSs](auto &val) {
			  using T = decltype(val);
			  if constexpr(std::is_same_v<T, util::Uuid &>)
				  outSs << util::uuid_to_string(val) << "]";
			  else
				  outSs << val << "]";
		  },
		  *m_identifier);
	}
}
std::string EntityURef::ToString() const
{
	std::stringstream ss;
	ss << "EntityURef";
	ToStringArgs(ss);
	return ss.str();
}

//////////

EntityUComponentRef::EntityUComponentRef() : EntityUComponentRef {util::Uuid {}, INVALID_COMPONENT_ID} {}
EntityUComponentRef::EntityUComponentRef(EntityIdentifier identifier, ComponentId componentId) : EntityURef {std::move(identifier)}, m_componentId {componentId} {}
EntityUComponentRef::EntityUComponentRef(EntityIdentifier identifier, const std::string &componentName) : EntityURef {std::move(identifier)}, m_componentName {componentName.empty() ? nullptr : std::make_unique<std::string>(componentName)} {}
EntityUComponentRef::EntityUComponentRef(const ecs::BaseEntity &ent, ComponentId componentId) : EntityURef {ent}, m_componentId {componentId} {}
EntityUComponentRef::EntityUComponentRef(const ecs::BaseEntity &ent, const std::string &componentName) : EntityURef {ent}, m_componentName {componentName.empty() ? nullptr : std::make_unique<std::string>(componentName)} {}
EntityUComponentRef::EntityUComponentRef(const EntityUComponentRef &other) { operator=(other); }
EntityUComponentRef::EntityUComponentRef(EntityUComponentRef &&other) { operator=(std::move(other)); }
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
bool EntityUComponentRef::HasComponentReference() const { return m_componentName != nullptr || m_componentId != INVALID_COMPONENT_ID; }
BaseEntityComponent *EntityUComponentRef::GetComponent(Game &game)
{
	if(!m_hComponent.IsValid()) {
		if(m_componentId == INVALID_COMPONENT_ID) {
			if(!m_componentName)
				return nullptr; // Unreachable?
			if(!game.GetEntityComponentManager().GetComponentTypeId(*m_componentName, m_componentId))
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
void EntityUComponentRef::ToStringArgs(std::stringstream &outSs) const
{
	EntityURef::ToStringArgs(outSs);
	outSs << "[Cmp:";
	if(!m_componentName && m_componentId == INVALID_COMPONENT_ID)
		outSs << "NULL]";
	else if(m_componentId != INVALID_COMPONENT_ID)
		outSs << m_componentId << "]";
	else
		outSs << *m_componentName << "]";
}
std::string EntityUComponentRef::ToString() const
{
	std::stringstream ss;
	ss << "EntityUComponentRef";
	ToStringArgs(ss);
	return ss.str();
}

//////////

EntityUComponentMemberRef::EntityUComponentMemberRef() : EntityUComponentMemberRef {util::Uuid {}, INVALID_COMPONENT_ID, ""} {}
EntityUComponentMemberRef::EntityUComponentMemberRef(EntityIdentifier identifier, ComponentId componentId, const std::string &memberName) : EntityUComponentRef {std::move(identifier), componentId}, m_memberRef {memberName} {}
EntityUComponentMemberRef::EntityUComponentMemberRef(EntityIdentifier identifier, const std::string &componentName, const std::string &memberName) : EntityUComponentRef {std::move(identifier), componentName}, m_memberRef {memberName} {}
EntityUComponentMemberRef::EntityUComponentMemberRef(const ecs::BaseEntity &ent, ComponentId componentId, const std::string &memberName) : EntityUComponentRef {ent, componentId}, m_memberRef {memberName} {}
EntityUComponentMemberRef::EntityUComponentMemberRef(const ecs::BaseEntity &ent, const std::string &componentName, const std::string &memberName) : EntityUComponentRef {ent, componentName}, m_memberRef {memberName} {}
EntityUComponentMemberRef::EntityUComponentMemberRef(const std::string &path) : EntityUComponentMemberRef {} { ecs::BaseEntity::ParseUri(path, *this); }
const ComponentMemberInfo *EntityUComponentMemberRef::GetMemberInfo(Game &game) const
{
	auto *c = GetComponent(game);
	if(!c)
		return nullptr;
	return m_memberRef.GetMemberInfo(*c);
}
ComponentMemberIndex EntityUComponentMemberRef::GetMemberIndex() const { return m_memberRef.GetMemberIndex(); }
const std::string &EntityUComponentMemberRef::GetMemberName() const { return m_memberRef.GetMemberName(); }
void EntityUComponentMemberRef::UpdateMemberIndex(Game &game) const
{
	if(GetMemberIndex() == INVALID_COMPONENT_ID)
		GetMemberInfo(game);
}

bool EntityUComponentMemberRef::HasMemberReference() const { return !m_memberRef.GetMemberName().empty(); }
void EntityUComponentMemberRef::ToStringArgs(std::stringstream &outSs) const
{
	EntityUComponentRef::ToStringArgs(outSs);
	outSs << "[Mem:";
	auto idx = m_memberRef.GetMemberIndex();
	auto &name = m_memberRef.GetMemberName();
	if(name.empty() && idx == INVALID_COMPONENT_MEMBER_INDEX)
		outSs << "NULL]";
	else if(idx != INVALID_COMPONENT_MEMBER_INDEX)
		outSs << idx << "]";
	else
		outSs << name << "]";
}
std::string EntityUComponentMemberRef::ToString() const
{
	std::stringstream ss;
	ss << "EntityUComponentMemberRef";
	ToStringArgs(ss);
	return ss.str();
}

////////////////////

MultiEntityURef::MultiEntityURef(EntityIdentifier identifier)
{
	std::visit(
	  [this](auto &v) {
		  if constexpr(std::is_same_v<decltype(v), util::Uuid>)
			  m_identifier = (v != util::Uuid {}) ? std::make_unique<EntityIdentifier>(v) : nullptr;
		  else
			  m_identifier = std::make_unique<EntityIdentifier>(std::move(v));
	  },
	  identifier);
}
MultiEntityURef::MultiEntityURef(const MultiEntityURef &other) : m_identifier {other.m_identifier ? std::make_unique<EntityIdentifier>(*other.m_identifier) : nullptr} {}
MultiEntityURef::MultiEntityURef(MultiEntityURef &&other) : m_identifier {other.m_identifier ? std::move(other.m_identifier) : nullptr} {}
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
MultiEntityURef::MultiEntityURef(const ecs::BaseEntity &ent) : MultiEntityURef {ent.GetUuid()} {}
void MultiEntityURef::FindEntities(Game &game, std::vector<ecs::BaseEntity *> &outEnts) const
{
	if(!m_identifier)
		return;
	ecs::EntityIterator entIt {game, ecs::EntityIterator::FilterFlags::Any};
	std::visit(
	  [&entIt](auto &val) {
		  using T = decltype(val);
		  if constexpr(std::is_same_v<std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>, util::Uuid>)
			  entIt.AttachFilter<EntityIteratorFilterUuid>(val);
		  else
			  entIt.AttachFilter<EntityIteratorFilterNameOrClass>(val);
	  },
	  *m_identifier);
	for(auto *ent : entIt) {
		if(outEnts.size() == outEnts.capacity())
			outEnts.reserve(outEnts.size() * 1.5 + 5);
		outEnts.push_back(ent);
	}
}
bool MultiEntityURef::HasEntityReference() const { return m_identifier != nullptr; }
void MultiEntityURef::ToStringArgs(std::stringstream &outSs) const
{
	outSs << "[ID:";
	if(!m_identifier)
		outSs << "NULL]";
	else {
		std::visit(
		  [&outSs](auto &val) {
			  using T = decltype(val);
			  if constexpr(std::is_same_v<T, util::Uuid &>)
				  outSs << util::uuid_to_string(val) << "]";
			  else
				  outSs << val << "]";
		  },
		  *m_identifier);
	}
}
std::string MultiEntityURef::ToString() const
{
	std::stringstream ss;
	ss << "MultiEntityURef";
	ToStringArgs(ss);
	return ss.str();
}

//////////

MultiEntityUComponentRef::MultiEntityUComponentRef() : MultiEntityUComponentRef {util::Uuid {}, INVALID_COMPONENT_ID} {}
MultiEntityUComponentRef::MultiEntityUComponentRef(EntityIdentifier identifier, ComponentId componentId) : MultiEntityURef {std::move(identifier)}, m_componentId {componentId} {}
MultiEntityUComponentRef::MultiEntityUComponentRef(EntityIdentifier identifier, const std::string &componentName) : MultiEntityURef {std::move(identifier)}, m_componentName {componentName.empty() ? nullptr : std::make_unique<std::string>(componentName)} {}
MultiEntityUComponentRef::MultiEntityUComponentRef(const ecs::BaseEntity &ent, ComponentId componentId) : MultiEntityURef {ent}, m_componentId {componentId} {}
MultiEntityUComponentRef::MultiEntityUComponentRef(const ecs::BaseEntity &ent, const std::string &componentName) : MultiEntityURef {ent}, m_componentName {componentName.empty() ? nullptr : std::make_unique<std::string>(componentName)} {}
MultiEntityUComponentRef::MultiEntityUComponentRef(const MultiEntityUComponentRef &other) : MultiEntityURef {other} { operator=(other); }
MultiEntityUComponentRef::MultiEntityUComponentRef(MultiEntityUComponentRef &&other) { operator=(std::move(other)); }
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
bool MultiEntityUComponentRef::HasComponentReference() const { return m_componentName != nullptr || m_componentId != INVALID_COMPONENT_ID; }
void MultiEntityUComponentRef::FindComponents(Game &game, std::vector<BaseEntityComponent *> &outComponents) const
{
	if(!m_identifier)
		return;
	ecs::EntityIterator entIt {game, ecs::EntityIterator::FilterFlags::Any};
	if(m_componentId == INVALID_COMPONENT_ID) {
		if(!m_componentName)
			return; // Unreachable?
		entIt.AttachFilter<EntityIteratorFilterComponent>(*m_componentName);
		EntityURef::AttachEntityFilter(entIt, *m_identifier);
		for(auto *ent : entIt) {
			if(outComponents.size() == outComponents.capacity())
				outComponents.reserve(outComponents.size() * 1.5 + 5);
			outComponents.push_back(ent->FindComponent(*m_componentName).get());
		}
		return;
	}
	entIt.AttachFilter<EntityIteratorFilterComponent>(m_componentId);
	EntityURef::AttachEntityFilter(entIt, *m_identifier);
	for(auto *ent : entIt) {
		if(outComponents.size() == outComponents.capacity())
			outComponents.reserve(outComponents.size() * 1.5 + 5);
		outComponents.push_back(ent->FindComponent(m_componentId).get());
	}
}
void MultiEntityUComponentRef::ToStringArgs(std::stringstream &outSs) const
{
	MultiEntityURef::ToStringArgs(outSs);
	outSs << "[Cmp:";
	if(!m_componentName && m_componentId == INVALID_COMPONENT_ID)
		outSs << "NULL]";
	else if(m_componentId != INVALID_COMPONENT_ID)
		outSs << m_componentId << "]";
	else
		outSs << *m_componentName << "]";
}
std::string MultiEntityUComponentRef::ToString() const
{
	std::stringstream ss;
	ss << "MultiEntityUComponentRef";
	ToStringArgs(ss);
	return ss.str();
}

//////////

std::ostream &operator<<(std::ostream &out, const EntityURef &ref)
{
	out << ref.ToString();
	return out;
}
std::ostream &operator<<(std::ostream &out, const EntityUComponentRef &ref)
{
	out << ref.ToString();
	return out;
}
std::ostream &operator<<(std::ostream &out, const EntityUComponentMemberRef &ref)
{
	out << ref.ToString();
	return out;
}
std::ostream &operator<<(std::ostream &out, const MultiEntityURef &ref)
{
	out << ref.ToString();
	return out;
}
std::ostream &operator<<(std::ostream &out, const MultiEntityUComponentRef &ref)
{
	out << ref.ToString();
	return out;
}
