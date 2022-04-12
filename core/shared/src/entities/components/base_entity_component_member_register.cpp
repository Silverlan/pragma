/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_entity_component_member_register.hpp"

using namespace pragma;
#pragma optimize("",off)
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
	// m_memberNameToIndex.clear();
}
void pragma::DynamicMemberRegister::RemoveMember(ComponentMemberIndex idx)
{
	auto it = m_members.find(idx);
	if(it == m_members.end())
		return;
	auto member = std::move(it->second);
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
	return (it != m_memberNameToIndex.end()) ? it->second : std::optional<pragma::ComponentMemberIndex>{};
}
const pragma::ComponentMemberInfo *pragma::DynamicMemberRegister::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto it = m_members.find(idx);
	return (it != m_members.end()) ? &it->second : nullptr;
}

pragma::ComponentMemberIndex pragma::DynamicMemberRegister::RegisterMember(pragma::ComponentMemberInfo &&memberInfo)
{
	auto lmemberName = memberInfo.GetName();
	ustring::to_lower(lmemberName);
	if(m_memberNameToIndex.find(lmemberName) != m_memberNameToIndex.end())
		RemoveMember(lmemberName);
	uint32_t idx;
	auto itName = m_memberNameToIndex.find(lmemberName);
	if(itName != m_memberNameToIndex.end())
		idx = itName->second;
	else
	{
		if(m_nextMemberIndex == std::numeric_limits<uint32_t>::max())
			m_nextMemberIndex = dynamic_cast<BaseEntityComponent*>(this)->GetStaticMemberCount();
		idx = m_nextMemberIndex++;
	}
	auto it = m_members.find(idx);
	if(it != m_members.end())
		it->second = std::move(memberInfo);
	else
		it = m_members.insert(std::make_pair(idx,std::move(memberInfo))).first;
	m_memberNameToIndex[lmemberName] = idx;
	OnMemberRegistered(it->second,idx);
	return idx;
}
pragma::ComponentMemberIndex pragma::DynamicMemberRegister::RegisterMember(const pragma::ComponentMemberInfo &memberInfo)
{
	auto cpy = memberInfo;
	return RegisterMember(std::move(cpy));
}
#pragma optimize("",on)
