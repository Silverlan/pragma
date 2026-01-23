// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :entities.member_reference;

using namespace pragma;

std::optional<ComponentMemberReference> ComponentMemberReference::Create(BaseEntityComponent &component, ComponentMemberIndex index)
{
	auto *info = component.GetMemberInfo(index);
	if(!info)
		return {};
	return ComponentMemberReference {info->GetName()};
}
std::optional<ComponentMemberReference> ComponentMemberReference::Create(const EntityComponentManager &manager, ComponentId componentId, ComponentMemberIndex index)
{
	auto *info = manager.GetComponentInfo(componentId);
	if(!info || index >= info->members.size())
		return {};
	auto &memberInfo = info->members[index];
	return ComponentMemberReference {memberInfo.GetName()};
}
ComponentMemberReference::ComponentMemberReference(const std::string &memberName) : m_name {get_normalized_component_member_name(memberName)}, m_nameHash {get_component_member_name_hash(memberName)} {}
const ComponentMemberInfo *ComponentMemberReference::GetMemberInfo(const BaseEntityComponent &component) const
{
	auto *info = component.GetMemberInfo(m_index);
	if(!info || info->GetNameHash() != m_nameHash) {
		auto newIndex = component.GetMemberIndex(m_name);
		if(!newIndex.has_value())
			return nullptr; // Invalid reference
		assert(component.GetMemberInfo(*newIndex)->GetNameHash() == m_nameHash);
		m_index = *newIndex; // Index has changed
		info = component.GetMemberInfo(m_index);
	}
	return info;
}
