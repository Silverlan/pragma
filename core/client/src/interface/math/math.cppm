// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include <mathutil/umath_lighting.hpp>
#include <mathutil/umat.h>
#include <array>

export module pragma.client.math;

import pragma.client.entities.components.color;
import pragma.client.entities.components.lights.point;

export namespace pragma::math {
	DLLCLIENT const std::array<Mat4, 6> &get_cubemap_view_matrices();
	DLLCLIENT const Mat4 &get_cubemap_projection_matrix(float aspectRatio = 1.f, float nearZ = 0.1f, float farZ = 10.f);

	namespace cycles {
		Watt get_light_power(const CLightComponent &light);
	};
};

const std::array<Mat4, 6> &pragma::math::get_cubemap_view_matrices()
{
	static const std::array<Mat4, 6> viewMatrices = {glm::lookAtRH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::lookAtRH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
	  glm::lookAtRH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::lookAtRH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
	  glm::lookAtRH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::lookAtRH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};
	return viewMatrices;
}

const Mat4 &pragma::math::get_cubemap_projection_matrix(float aspectRatio, float nearZ, float farZ)
{
	static const auto projMatrix = glm::perspectiveRH<float>(glm::radians(90.0f), aspectRatio, nearZ, farZ);
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
		return ulighting::cycles::lumen_to_watt_point(lumen, col.ToVector3());
	auto spotC = ent.GetComponent<CLightSpotComponent>();
	if(spotC.valid())
		return ulighting::cycles::lumen_to_watt_spot(lumen, col.ToVector3(), spotC->GetOuterConeAngle() / 2.f);
	auto envC = ent.GetComponent<CLightDirectionalComponent>();
	if(envC.expired())
		return 0.f;
	return ulighting::cycles::lumen_to_watt_area(lumen, col.ToVector3());
}
