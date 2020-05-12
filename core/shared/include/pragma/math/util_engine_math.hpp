/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_ENGINE_MATH_HPP__
#define __UTIL_ENGINE_MATH_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/uvec.h>

namespace util
{
	DLLNETWORK Vector3 angular_velocity_to_linear(const Vector3 &refPos,const Vector3 &angVel,const Vector3 &tgtPos);
	DLLNETWORK float calc_fov_from_lens(umath::Millimeter sensorSize,umath::Millimeter focalLength,float aspectRatio);
	DLLNETWORK float calc_aperture_size_from_fstop(float fstop,umath::Millimeter focalLength,bool orthographicCamera=false);
	DLLNETWORK float calc_focal_length_from_fov(umath::Degree hfov,umath::Millimeter sensorSize);
	DLLNETWORK float calc_fov_from_focal_length(umath::Millimeter focalLength,umath::Millimeter sensorSize);
};

#endif
