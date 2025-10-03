// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "sharedutils/util.h"

#include "mathutil/uvec.h"

module pragma.shared;

import :math;

Vector3 util::angular_velocity_to_linear(const Vector3 &refPos, const Vector3 &angVel, const Vector3 &tgtPos) { return uvec::cross(angVel, tgtPos - refPos); }
