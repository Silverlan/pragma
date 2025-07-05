// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __SKELETON_UTIL_HPP__
#define __SKELETON_UTIL_HPP__
#include "pragma/networkdefinitions.h"
#include <udm_types.hpp>
#include <iostream>
#include <vector>
#include <mathutil/glmutil.h>
#include <string>
#include <unordered_map>
#include "pragma/math/orientation.h"
#include "pragma/physics/jointinfo.h"

struct DLLNETWORK BoneList // Simplified Skeleton without an hierarchy
{
  private:
	std::vector<std::string> m_bones;
	std::unordered_map<std::string, uint32_t> m_boneIDs;
  public:
	uint32_t AddBone(const std::string &name);
	int32_t GetBoneID(const std::string &name) const;
	uint32_t GetBoneCount() const;
};

#endif
