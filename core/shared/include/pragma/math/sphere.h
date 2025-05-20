/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

struct DLLNETWORK Sphere {
	Sphere() : pos(0, 0, 0), radius(0.f) {}
	Sphere(Vector3 &position, float r) : pos(position), radius(r) {}
	Vector3 pos = {};
	float radius = 0.f;
};

#endif
