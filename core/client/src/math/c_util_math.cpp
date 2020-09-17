/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/math/c_util_math.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include <pragma/entities/environment/lights/env_light.h>

const std::array<Mat4,6> &pragma::math::get_cubemap_view_matrices()
{
	static const std::array<Mat4,6> viewMatrices = {
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(-1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec3(0.0f,-1.0f,0.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,-1.0f,0.0f))
	};
	return viewMatrices;
}

const Mat4 &pragma::math::get_cubemap_projection_matrix(float aspectRatio,float nearZ,float farZ)
{
	static const auto projMatrix = glm::perspectiveRH<float>(glm::radians(90.0f),aspectRatio,nearZ,farZ);
	return projMatrix;
}

Watt pragma::math::cycles::get_light_power(const CLightComponent &light)
{
	auto lumen = light.GetLightIntensityLumen();
	auto &ent = light.GetEntity();
	auto colC = ent.GetComponent<CColorComponent>();
	auto col = colC.valid() ? colC->GetColor() : Color::White;
	auto pointC = ent.GetComponent<CLightPointComponent>();
	if(pointC.valid())
		return ulighting::cycles::lumen_to_watt_point(lumen,col.ToVector3());
	auto spotC = ent.GetComponent<CLightSpotComponent>();
	if(spotC.valid())
		return ulighting::cycles::lumen_to_watt_spot(lumen,col.ToVector3(),spotC->GetOuterCutoffAngle());
	auto envC = ent.GetComponent<CLightDirectionalComponent>();
	if(envC.expired())
		return 0.f;
	return ulighting::cycles::lumen_to_watt_area(lumen,col.ToVector3());
}
