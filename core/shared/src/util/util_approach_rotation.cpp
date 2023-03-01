/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/util_approach_rotation.hpp"

Quat uquat::approach_direction(const Quat &rot, const Vector3 &up, const Vector3 &targetDir, const Vector2 &rotAmount, Vector2 *resRotAm, const Vector2 *pitchLimit, const Vector2 *yawLimit, const Quat *rotPivot, const EulerAngles *angOffset)
{
	auto nup = up - uvec::project(up, targetDir);
	uvec::normalize(&nup);

	auto *rotPiv = (rotPivot != nullptr) ? rotPivot : &rot;
	auto rotBaseInv = uquat::get_inverse(*rotPiv);

	auto rotTgt = rotBaseInv * uquat::create_look_rotation(targetDir, nup); // Target rotation
	auto rotSrc = rotBaseInv * rot;                                         // Current rotation

	auto angTgt = EulerAngles(rotTgt);
	auto angSrc = EulerAngles(rotSrc);
	if(angOffset != nullptr)
		angTgt = angTgt + *angOffset;

	if(pitchLimit != nullptr) {
		if(angTgt.p < pitchLimit->x)
			angTgt.p = pitchLimit->x; // Min Pitch
		if(angTgt.p > pitchLimit->y)
			angTgt.p = pitchLimit->y; // Max Pitch
	}

	if(yawLimit != nullptr) {
		if(angTgt.y < yawLimit->x)
			angTgt.y = yawLimit->x; // Min Yaw
		if(angTgt.y > yawLimit->y)
			angTgt.y = yawLimit->y; // Max Yaw
	}

	angSrc.p = umath::normalize_angle(umath::approach_angle(angSrc.p, angTgt.p, rotAmount.x));
	angSrc.y = umath::normalize_angle(umath::approach_angle(angSrc.y, angTgt.y, rotAmount.y));
	angSrc.r = 0.0;

	if(resRotAm != nullptr) {
		resRotAm->x = umath::get_angle_difference(angTgt.p, angSrc.p);
		resRotAm->y = umath::get_angle_difference(angTgt.y, angSrc.y);
	}

	rotTgt = *rotPiv * uquat::create(angSrc);
	return rotTgt;
}
