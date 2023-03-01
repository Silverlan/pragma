/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/component_member_reference.hpp"
#include "pragma/entities/entity_component_manager.hpp"

using namespace pragma;

std::optional<ComponentMemberReference> ComponentMemberReference::Create(BaseEntityComponent &component, pragma::ComponentMemberIndex index)
{
	auto *info = component.GetMemberInfo(index);
	if(!info)
		return {};
	return ComponentMemberReference {info->GetName()};
}
std::optional<ComponentMemberReference> ComponentMemberReference::Create(const EntityComponentManager &manager, ComponentId componentId, pragma::ComponentMemberIndex index)
{
	auto *info = manager.GetComponentInfo(componentId);
	if(!info || index >= info->members.size())
		return {};
	auto &memberInfo = info->members[index];
	return ComponentMemberReference {memberInfo.GetName()};
}
ComponentMemberReference::ComponentMemberReference(const std::string &memberName) : m_name {pragma::get_normalized_component_member_name(memberName)}, m_nameHash {pragma::get_component_member_name_hash(memberName)} {}
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
