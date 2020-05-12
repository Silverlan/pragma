/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/math/util_engine_math.hpp"

Vector3 util::angular_velocity_to_linear(const Vector3 &refPos,const Vector3 &angVel,const Vector3 &tgtPos) {return uvec::cross(angVel,tgtPos -refPos);}

float util::calc_fov_from_lens(umath::Millimeter sensorSize,umath::Millimeter focalLength,float aspectRatio)
{
	// Source: https://github.com/blender/blender/blob/79f99becafa3e2a2d7e0b6779961a4e2b58b371e/intern/cycles/blender/blender_camera.cpp#L468
	return 2.f *atanf((0.5f *sensorSize) /focalLength /aspectRatio);
}
float util::calc_aperture_size_from_fstop(float fstop,umath::Millimeter focalLength,bool orthographicCamera)
{
	// Source: https://github.com/blender/blender/blob/79f99becafa3e2a2d7e0b6779961a4e2b58b371e/intern/cycles/blender/blender_camera.cpp#L213
	fstop = fmax(fstop,1e-5f);

	auto apertureSize = 0.f;
	if(orthographicCamera)
		apertureSize = 1.0f /(2.0f *fstop);
	else
		apertureSize = (focalLength *1e-3f) /(2.0f *fstop);
	return apertureSize;
}
float util::calc_focal_length_from_fov(umath::Degree hfov,umath::Millimeter sensorSize)
{
	return (sensorSize /2.0f) /tanf(umath::deg_to_rad(hfov) *0.5f);
}
float util::calc_fov_from_focal_length(umath::Millimeter focalLength,umath::Millimeter sensorSize)
{
	return 2.0f *atanf((sensorSize /2.0f) /focalLength);
}
