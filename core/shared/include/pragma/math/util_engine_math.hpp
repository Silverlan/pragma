// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_ENGINE_MATH_HPP__
#define __UTIL_ENGINE_MATH_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/uvec.h>

namespace util {
	DLLNETWORK Vector3 angular_velocity_to_linear(const Vector3 &refPos, const Vector3 &angVel, const Vector3 &tgtPos);
};

#endif
