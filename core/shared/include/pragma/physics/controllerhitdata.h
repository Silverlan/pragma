// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __CONTROLLERHITDATA_H__
#define __CONTROLLERHITDATA_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

struct DLLNETWORK ControllerHitData {
	ControllerHitData();
	~ControllerHitData();
	PhysObjHandle *physObj = nullptr;
	Vector3 hitNormal = {};
	void Clear();
};

#endif
