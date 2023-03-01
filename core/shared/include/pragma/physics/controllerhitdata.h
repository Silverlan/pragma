/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
