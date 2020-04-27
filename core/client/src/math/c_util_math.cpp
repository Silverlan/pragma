/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/math/c_util_math.hpp"
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

float pragma::math::light_intensity_to_watts(float candelaOrLux,LightType lightType)
{
	if(lightType == LightType::Directional)
	{
		static auto intensity = 7.5f;
		auto watt = candelaOrLux /intensity; // Lux
		return watt;
	}
	// Candela
	auto watt = ulighting::lumens_to_watts(candelaOrLux,ulighting::get_luminous_efficacy(ulighting::LightSourceType::LEDLamp));
	static auto mulFactor = 30.f;
	watt *= mulFactor; // Arbitrary, but results in a closer match
	return watt;
}
