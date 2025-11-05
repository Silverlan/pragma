// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.animation.bone_list;

uint32_t BoneList::AddBone(const std::string &name)
{
	if(GetBoneID(name) != -1)
		return std::numeric_limits<uint32_t>::max();
	auto ID = static_cast<uint32_t>(m_bones.size());
	m_bones.push_back(name);
	m_boneIDs[name] = ID;
	return ID;
}

uint32_t BoneList::GetBoneCount() const { return static_cast<uint32_t>(m_bones.size()); }

int32_t BoneList::GetBoneID(const std::string &name) const
{
	auto it = m_boneIDs.find(name);
	if(it == m_boneIDs.end())
		return -1;
	return it->second;
}
