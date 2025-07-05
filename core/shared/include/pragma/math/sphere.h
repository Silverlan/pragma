// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
