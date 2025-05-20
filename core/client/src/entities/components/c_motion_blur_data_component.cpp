/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/c_engine.h"
#include "pragma/entities/components/c_motion_blur_data_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/rendering/shaders/info/c_shader_velocity_buffer.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_manager_t.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

static pragma::ShaderVelocityBuffer *g_velocityBufferShader = nullptr;
pragma::ShaderVelocityBuffer *pragma::get_velocity_buffer_shader()
{
	if(!g_velocityBufferShader)
		g_velocityBufferShader = static_cast<pragma::ShaderVelocityBuffer *>(c_engine->GetShader("velocity_buffer").get());
	return g_velocityBufferShader;
}

void CMotionBlurDataComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CMotionBlurDataComponent::Initialize() { BaseEntityComponent::Initialize(); }

void CMotionBlurDataComponent::UpdateEntityPoses()
{
	++m_lastUpdateIndex;
	auto *velShader = get_velocity_buffer_shader();
	if(!velShader)
		return;
	auto &entCam = GetEntity();
	auto itCur = m_motionBlurData.curModelMatrices.find(&entCam);
	auto itPrev = m_motionBlurData.prevModelMatrices.find(&entCam);
	if(itCur != m_motionBlurData.curModelMatrices.end() && itPrev != m_motionBlurData.prevModelMatrices.end()) {
		auto &curCamPose = itCur->second.pose;
		auto &prevCamPose = itPrev->second.pose;
		auto camVel = curCamPose.GetOrigin() - prevCamPose.GetOrigin();
		camVel *= (1.f / 60.f);

		auto rot = curCamPose.GetRotation() * uquat::get_inverse(prevCamPose.GetRotation());
		Vector3 axis;
		float angle;
		uquat::to_axis_angle(rot, axis, angle);
		auto angVel = axis * angle;
		m_motionBlurData.cameraData.linearCameraVelocity = {camVel, 0.f};
		m_motionBlurData.cameraData.angularCameraVelocity = {angVel, 0.f};
	}

	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
	for(auto *ent : entIt) {
		auto &r = *static_cast<CBaseEntity *>(ent)->GetRenderComponent();
		auto curPose = r.GetTransformationMatrix();
		auto it = m_motionBlurData.curModelMatrices.find(ent);

		auto *animC = static_cast<CAnimatedComponent *>(ent->GetAnimatedComponent().get());
		auto curBoneBuffer = animC ? animC->GetBoneBuffer() : nullptr;
		if(it != m_motionBlurData.curModelMatrices.end())
			m_motionBlurData.prevModelMatrices[ent] = it->second;
		else
			m_motionBlurData.prevModelMatrices[ent] = {curPose, ent->GetPose()};

		if(curBoneBuffer) {
			if(!m_motionBlurData.prevModelMatrices[ent].boneBuffer) {
				auto boneBuffer = pragma::get_instance_bone_buffer()->AllocateBuffer();
				m_motionBlurData.prevModelMatrices[ent].boneBuffer = boneBuffer;

				auto dsg = velShader->CreateDescriptorSetGroup(pragma::ShaderVelocityBuffer::DESCRIPTOR_SET_BONE_BUFFER.setIndex);
				dsg->GetDescriptorSet()->SetBindingUniformBuffer(*boneBuffer, 0);
				m_motionBlurData.prevModelMatrices[ent].boneDsg = dsg;
			}
		}

		m_motionBlurData.curModelMatrices[ent] = {curPose, ent->GetPose()};
	}
}
