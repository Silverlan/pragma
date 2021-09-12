/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_entity_component_member_register.hpp"

using namespace pragma;

pragma::DynamicMemberRegister::DynamicMemberRegister()
{}
void pragma::DynamicMemberRegister::ReserveMembers(uint32_t count)
{
	m_members.reserve(m_members.size() +count);
	m_memberNameToIndex.reserve(m_memberNameToIndex.size() +count);
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
	auto it = m_members.begin() +idx;
	auto member = std::move(*it);
	m_members.erase(it);
	UpdateMemberNameMap();
	OnMemberRemoved(member,idx);
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
	m_memberNameToIndex.clear();
	m_memberNameToIndex.reserve(m_members.size());
	for(size_t idx = 0; auto &memberInfo : m_members)
	{
		auto lname = memberInfo.GetName();
		ustring::to_lower(lname);
		m_memberNameToIndex[lname] = idx++;
	}
}

std::optional<pragma::ComponentMemberIndex> pragma::DynamicMemberRegister::GetMemberIndex(const std::string &name) const
{
	auto lname = name;
	ustring::to_lower(lname);
	auto it = m_memberNameToIndex.find(lname);
	return (it != m_memberNameToIndex.end()) ? it->second : std::optional<pragma::ComponentMemberIndex>{};
}
const pragma::ComponentMemberInfo *pragma::DynamicMemberRegister::GetMemberInfo(ComponentMemberIndex idx) const
{
	if(idx >= m_members.size())
		return nullptr;
	return &m_members[idx];
}

pragma::ComponentMemberIndex pragma::DynamicMemberRegister::RegisterMember(pragma::ComponentMemberInfo &&memberInfo)
{
	auto lmemberName = memberInfo.GetName();
	ustring::to_lower(lmemberName);
	if(m_memberNameToIndex.find(lmemberName) != m_memberNameToIndex.end())
		RemoveMember(lmemberName);
	m_members.push_back(std::move(memberInfo));
	auto idx = m_members.size() -1;
	m_memberNameToIndex[lmemberName] = idx;
	OnMemberRegistered(m_members.back(),idx);
	return idx;
}
pragma::ComponentMemberIndex pragma::DynamicMemberRegister::RegisterMember(const pragma::ComponentMemberInfo &memberInfo)
{
	auto cpy = memberInfo;
	return RegisterMember(std::move(cpy));
}
