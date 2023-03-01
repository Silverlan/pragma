/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __UTIL_BALLISTIC_H__
#define __UTIL_BALLISTIC_H__

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <mathutil/uvec.h>

namespace umath {
	DLLNETWORK Vector3 calc_ballistic_position(const Vector3 &start, const Vector3 &vel, float gravity, float t);
	DLLNETWORK float calc_ballistic_range(float speed, float gravity, float initialHeight);
	DLLNETWORK bool calc_ballistic_velocity(const Vector3 &start, const Vector3 &end, float angle, float gravity, Vector3 &vel);
	DLLNETWORK float calc_ballistic_time_of_flight(const Vector3 &start, float launchAngle, float velocity, float gravity);
	DLLNETWORK float calc_ballistic_time_of_flight(const Vector3 &start, const Vector3 &vel, float gravity);
	DLLNETWORK float calc_ballistic_time_of_flight(const Vector3 &start, const Vector3 &end, float launchAngle, float velocity, float gravity);
	DLLNETWORK float calc_ballistic_time_of_flight(const Vector3 &start, const Vector3 &end, const Vector3 &vel, float gravity);
	DLLNETWORK float calc_ballistic_angle_of_reach(const Vector3 &start, float distance, float initialVelocity, float gravity);
	DLLNETWORK int32_t solve_ballistic_arc(const Vector3 &projPos, float projSpeed, const Vector3 &target, float gravity, std::array<Vector3, 2> &s);
	DLLNETWORK int32_t solve_ballistic_arc(const Vector3 &projPos, float projSpeed, const Vector3 &targetPos, const Vector3 &targetVelocity, float gravity, std::array<Vector3, 2> &s);
	DLLNETWORK bool solve_ballistic_arc_lateral(const Vector3 &projPos, float lateralSpeed, const Vector3 &targetPos, float maxHeight, Vector3 &fireVelocity, float &gravity);
	DLLNETWORK bool solve_ballistic_arc_lateral(const Vector3 &projPos, float lateralSpeed, const Vector3 &target, const Vector3 &targetVelocity, float maxHeightOffset, Vector3 &fireVelocity, float &gravity, Vector3 &impactPoint);
};

#endif
