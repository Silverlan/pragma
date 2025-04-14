/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_entity_component_member_register.hpp"

using namespace pragma;

pragma::DynamicMemberRegister::DynamicMemberRegister() {}
void pragma::DynamicMemberRegister::ReserveMembers(uint32_t count)
{
	m_members.reserve(m_members.size() + count);
	m_memberNameToIndex.reserve(m_memberNameToIndex.size() + count);
}
void pragma::DynamicMemberRegister::ClearMembers()
{
	m_members.clear();
	m_memberNameToIndex.clear();
}
size_t pragma::DynamicMemberRegister::GetDynamicMemberStartOffset() const { return dynamic_cast<const BaseEntityComponent *>(this)->GetStaticMemberCount(); }
void pragma::DynamicMemberRegister::RemoveMember(ComponentMemberIndex idx)
{
	auto offset = GetDynamicMemberStartOffset();
	if(idx < offset)
		return;
	auto dynamicIdx = idx - offset;
	if(dynamicIdx >= m_members.size())
		return;
	auto member = std::move(m_members[dynamicIdx]);
	m_members.erase(m_members.begin() + dynamicIdx);
	auto itMap = m_memberNameToIndex.find(member.GetName());
	if(itMap != m_memberNameToIndex.end())
		m_memberNameToIndex.erase(itMap);
	for(auto &[name, propIndex] : m_memberNameToIndex) {
		if(propIndex >= idx)
			--propIndex;
	}
	UpdateMemberNameMap();
	OnMemberRemoved(member, idx);
}
void pragma::DynamicMemberRegister::RemoveMember(const std::string &name)
{
	auto id = GetMemberIndex(name);
	if(!id.has_value())
		return;
	RemoveMember(*id);
}

void pragma::DynamicMemberRegister::UpdateMemberNameMap()
{
	// Obsolete
}

std::optional<pragma::ComponentMemberIndex> pragma::DynamicMemberRegister::GetMemberIndex(const std::string &name) const
{
	auto lname = name;
	ustring::to_lower(lname);
	auto it = m_memberNameToIndex.find(lname);
	if(it == m_memberNameToIndex.end())
		return {};
	return it->second;
}
const pragma::ComponentMemberInfo *pragma::DynamicMemberRegister::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto offset = GetDynamicMemberStartOffset();
	if(idx < offset || idx >= offset + m_members.size())
		return nullptr;
	return &m_members[idx - offset];
}

pragma::ComponentMemberIndex pragma::DynamicMemberRegister::RegisterMember(pragma::ComponentMemberInfo &&memberInfo)
{
	std::string lmemberName = memberInfo.GetName();
	ustring::to_lower(lmemberName);
	if(m_memberNameToIndex.find(lmemberName) != m_memberNameToIndex.end())
		RemoveMember(lmemberName);
	auto itName = m_memberNameToIndex.find(lmemberName);
	auto offset = GetDynamicMemberStartOffset();
	uint32_t idx;
	if(itName != m_memberNameToIndex.end()) {
		idx = itName->second;
		m_members[idx - offset] = std::move(memberInfo);
	}
	else {
		idx = m_members.size() + offset;
		m_members.push_back(std::move(memberInfo));
	}

	m_memberNameToIndex[lmemberName] = idx;
	OnMemberRegistered(m_members[idx - offset], idx);
	return idx;
}
pragma::ComponentMemberIndex pragma::DynamicMemberRegister::RegisterMember(const pragma::ComponentMemberInfo &memberInfo)
{
	auto cpy = memberInfo;
	return RegisterMember(std::move(cpy));
}
