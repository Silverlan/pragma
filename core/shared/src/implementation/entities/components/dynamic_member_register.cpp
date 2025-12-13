// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.dynamic_member_register;

using namespace pragma;

DynamicMemberRegister::DynamicMemberRegister() {}
void DynamicMemberRegister::ReserveMembers(uint32_t count)
{
	m_members.reserve(m_members.size() + count);
	m_memberNameToIndex.reserve(m_memberNameToIndex.size() + count);
}
void DynamicMemberRegister::ClearMembers()
{
	m_members.clear();
	m_memberNameToIndex.clear();
}
size_t DynamicMemberRegister::GetDynamicMemberStartOffset() const { return dynamic_cast<const BaseEntityComponent *>(this)->GetStaticMemberCount(); }
void DynamicMemberRegister::RemoveMember(ComponentMemberIndex idx)
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
void DynamicMemberRegister::RemoveMember(const std::string &name)
{
	auto id = GetMemberIndex(name);
	if(!id.has_value())
		return;
	RemoveMember(*id);
}

void DynamicMemberRegister::UpdateMemberNameMap()
{
	// Obsolete
}

std::optional<ComponentMemberIndex> DynamicMemberRegister::GetMemberIndex(const std::string &name) const
{
	auto lname = name;
	string::to_lower(lname);
	auto it = m_memberNameToIndex.find(lname);
	if(it == m_memberNameToIndex.end())
		return {};
	return it->second;
}
const ComponentMemberInfo *DynamicMemberRegister::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto offset = GetDynamicMemberStartOffset();
	if(idx < offset || idx >= offset + m_members.size())
		return nullptr;
	return &m_members[idx - offset];
}

ComponentMemberIndex DynamicMemberRegister::RegisterMember(ComponentMemberInfo &&memberInfo)
{
	std::string lmemberName = memberInfo.GetName();
	string::to_lower(lmemberName);
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
ComponentMemberIndex DynamicMemberRegister::RegisterMember(const ComponentMemberInfo &memberInfo)
{
	auto cpy = memberInfo;
	return RegisterMember(std::move(cpy));
}
