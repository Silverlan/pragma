/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SKELETON_UTIL_HPP__
#define __SKELETON_UTIL_HPP__
#include "pragma/networkdefinitions.h"
#include <iostream>
#include <vector>
#include <mathutil/glmutil.h>
#include <string>
#include <unordered_map>
#include "pragma/math/orientation.h"
#include "pragma/physics/jointinfo.h"

import udm;

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
