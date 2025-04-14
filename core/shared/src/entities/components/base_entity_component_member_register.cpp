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
void pragma::DynamicMemberRegister::RemoveMember(ComponentMemberIndex idx)
{
	if(idx >= m_members.size())
		return;
	auto member = m_members[idx];
	//std::move(m_members[idx]);
	m_members.erase(m_members.begin() + idx);
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
	/*m_memberNameToIndex.clear();
	m_memberNameToIndex.reserve(m_members.size());
	for(size_t idx = 0; auto &memberInfo : m_members)
	{
		auto lname = memberInfo.GetName();
		ustring::to_lower(lname);
		m_memberNameToIndex[lname] = idx++;
	}*/
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
	if(idx >= m_members.size())
		return nullptr;
	return &m_members[idx];
}

pragma::ComponentMemberIndex pragma::DynamicMemberRegister::RegisterMember(pragma::ComponentMemberInfo &&memberInfo)
{
	std::string lmemberName = memberInfo.GetName();
	ustring::to_lower(lmemberName);
	if(m_memberNameToIndex.find(lmemberName) != m_memberNameToIndex.end())
		RemoveMember(lmemberName);
	auto itName = m_memberNameToIndex.find(lmemberName);
	uint32_t idx;
	if(itName != m_memberNameToIndex.end()) {
		idx = itName->second;
		m_members[idx] = std::move(memberInfo);
	}
	else {
		idx = m_members.size();
		m_members.push_back(std::move(memberInfo));
	}

	m_memberNameToIndex[lmemberName] = idx;
	OnMemberRegistered(m_members[idx], idx);
	return idx;
}
pragma::ComponentMemberIndex pragma::DynamicMemberRegister::RegisterMember(const pragma::ComponentMemberInfo &memberInfo)
{
	auto cpy = memberInfo;
	return RegisterMember(std::move(cpy));
}
