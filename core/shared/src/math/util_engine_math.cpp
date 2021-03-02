/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/math/util_engine_math.hpp"

Vector3 util::angular_velocity_to_linear(const Vector3 &refPos,const Vector3 &angVel,const Vector3 &tgtPos) {return uvec::cross(angVel,tgtPos -refPos);}
