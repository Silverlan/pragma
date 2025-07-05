// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ICOSPHERE_H__
#define __ICOSPHERE_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <vector>
#include <unordered_map>
#include <stdint.h>

class DLLNETWORK IcoSphere {
  private:
	IcoSphere() = delete;
  public:
	static void Create(const Vector3 &origin, float radius, std::vector<Vector3> &verts, std::vector<uint16_t> &indices, uint32_t recursionLevel = 1);
	static void Create(const Vector3 &origin, float radius, std::vector<Vector3> &verts, uint32_t recursionLevel = 1);
};

#endif
