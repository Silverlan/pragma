/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ICOSPHERE_H__
#define __ICOSPHERE_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <vector>
#include <unordered_map>
#include <stdint.h>

class DLLNETWORK IcoSphere
{
private:
	IcoSphere()=delete;
public:
	static void Create(const Vector3 &origin,float radius,std::vector<Vector3> &verts,std::vector<uint16_t> &indices,uint32_t recursionLevel=1);
	static void Create(const Vector3 &origin,float radius,std::vector<Vector3> &verts,uint32_t recursionLevel=1);
};

#endif