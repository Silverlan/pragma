/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __UTIL_APPROACH_ROTATION_HPP__
#define __UTIL_APPROACH_ROTATION_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>
#include <mathutil/uvec.h>

namespace uquat {
	DLLNETWORK Quat approach_direction(const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const Vector2 &rotAmount, Vector2 *resRotAm = nullptr, const Vector2 *pitchLimit = nullptr, const Vector2 *yawLimit = nullptr, const Quat *rotPivot = nullptr,
	  const EulerAngles *angOffset = nullptr);
};

#endif
