// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.animation.bone_list;

export import std.compat;

export struct DLLNETWORK BoneList // Simplified Skeleton without an hierarchy
{
  private:
	std::vector<std::string> m_bones;
	std::unordered_map<std::string, uint32_t> m_boneIDs;
  public:
	uint32_t AddBone(const std::string &name);
	int32_t GetBoneID(const std::string &name) const;
	uint32_t GetBoneCount() const;
};
