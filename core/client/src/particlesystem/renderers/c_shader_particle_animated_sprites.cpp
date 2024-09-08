/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_animated_sprites.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

static Mat3x4 calc_rotation_matrix_around_axis(const Vector3 &axis, umath::Degree ang)
{
	ang = umath::deg_to_rad(ang);
	auto sin = umath::sin(ang);
	auto cos = umath::cos(ang);
	auto xSq = axis.x * axis.x;
	auto ySq = axis.y * axis.y;
	auto zSq = axis.z * axis.z;
	auto m = Mat3x4();

	// Column 0:
	m[0][0] = xSq + (1 - xSq) * cos;
	m[1][0] = axis.x * axis.y * (1 - cos) + axis.z * sin;
	m[2][0] = axis.z * axis.x * (1 - cos) - axis.y * sin;

	// Column 1:
	m[0][1] = axis.x * axis.y * (1 - cos) - axis.z * sin;
	m[1][1] = ySq + (1 - ySq) * cos;
	m[2][1] = axis.y * axis.z * (1 - cos) + axis.x * sin;

	// Column 2:
	m[0][2] = axis.z * axis.x * (1 - cos) + axis.y * sin;
	m[1][2] = axis.y * axis.z * (1 - cos) - axis.x * sin;
	m[2][2] = zSq + (1 - zSq) * cos;

	// Column 3:
	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = 0;
	return m;
}

static Vector3 rotate_vector(const Vector3 &v, const Mat4 &m) { return Vector3 {uvec::dot(v, Vector3(m[0][0], m[0][1], m[0][2])), uvec::dot(v, Vector3(m[1][0], m[1][1], m[1][2])), uvec::dot(v, Vector3(m[2][0], m[2][1], m[2][2]))}; }

ShaderParticleAnimatedSprites::ShaderParticleAnimatedSprites(prosper::IPrContext &context, const std::string &identifier) : ShaderParticle2DBase(context, identifier, "programs/pfm/particles/particle_animated_sprites", "programs/particles/particle") {}

Vector3 ShaderParticleAnimatedSprites::DoCalcVertexPosition(const pragma::CParticleSystemComponent &ptc, uint32_t ptIdx, uint32_t localVertIdx, const Vector3 &camPos, const Vector3 &camUpWs, const Vector3 &camRightWs, float nearZ, float farZ) const
{
	// Note: This has to match the calculations performed in the vertex shader
	auto *pt = const_cast<pragma::CParticleSystemComponent &>(ptc).GetParticle(ptIdx);
	auto ptWorldPos = pt->GetPosition();
	// bool useCamBias = (u_instance.cameraBias != 0.0);
	// TODO
	/*if(useCamBias)
	{
		vec3 vEyeDir = normalize(u_renderSettings.posCam -vecWorldPos);
		vEyeDir *= camBias;
		vecWorldPos += vEyeDir;
	}*/

	auto rot = pt->GetRotation();
	auto yaw = pt->GetRotationYaw();
	auto radius = pt->GetRadius();
	auto viewToPos = ptWorldPos - camPos;
	auto l = uvec::length(viewToPos);
	if(l < radius / 2.0)
		return camPos; // TODO: How to handle this?

	auto camRight = -camRightWs;
	auto camForward = uvec::cross(camUpWs, camRight);
	uvec::normalize(&camForward);
	if(yaw != 0.f) {
		auto matRot = calc_rotation_matrix_around_axis(camUpWs, yaw);
		camRight = rotate_vector(camRight, matRot);
	}

	camRight = camRight * radius;
	auto camUp = camUpWs * radius;

	auto ca = umath::cos(-rot);
	auto sa = umath::sin(-rot);

	if(localVertIdx == 0) {
		float x = ca - sa;
		float y = -ca - sa;
		auto vecCorner = ptWorldPos + x * camRight;
		vecCorner = vecCorner + y * camUp;
		return vecCorner;
	}
	else if(localVertIdx == 1) {
		float x = ca + sa;
		float y = ca - sa;
		auto vecCorner = ptWorldPos + x * camRight;
		vecCorner = vecCorner + y * camUp;
		return vecCorner;
	}
	else if(localVertIdx == 2) {
		float x = -ca + sa;
		float y = ca + sa;
		auto vecCorner = ptWorldPos + x * camRight;
		vecCorner = vecCorner + y * camUp;
		return vecCorner;
	}
	float x = -ca - sa;
	float y = -ca + sa;
	auto vecCorner = ptWorldPos + x * camRight;
	vecCorner = vecCorner + y * camUp;
	return vecCorner;
}

void ShaderParticleAnimatedSprites::InitializeShaderResources()
{
	ShaderSceneLit::InitializeShaderResources();

	RegisterDefaultGfxPipelineVertexAttributes();
	AttachPushConstantRange(0u, sizeof(ShaderParticle2DBase::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
	RegisterDefaultGfxPipelineDescriptorSetGroups();
}
void ShaderParticleAnimatedSprites::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	auto basePipelineIdx = GetBasePipelineIndex(pipelineIdx);
	ShaderSceneLit::InitializeGfxPipeline(pipelineInfo, basePipelineIdx);

	pipelineInfo.ToggleDepthWrites(pipelineIdx == GetDepthPipelineIndex()); // Last pipeline is depth pipeline

	ShaderParticleBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
}

bool ShaderParticleAnimatedSprites::RecordDraw(prosper::ShaderBindState &bindState, pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const CParticleSystemComponent &ps, CParticleSystemComponent::OrientationType orientationType,
  ParticleRenderFlags renderFlags)
{
	PushConstants pushConstants {};
	pushConstants.camBias = 0.f;
	return RecordPushConstants(bindState, pushConstants) && ShaderParticle2DBase::RecordDraw(bindState, scene, renderer, ps, orientationType, renderFlags);
}
