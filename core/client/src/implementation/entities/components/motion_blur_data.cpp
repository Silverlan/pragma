// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.motion_blur_data;
import :engine;
import :entities.components.render;
import :game;
import :rendering.shaders;

using namespace pragma;

static ShaderVelocityBuffer *g_velocityBufferShader = nullptr;
ShaderVelocityBuffer *pragma::get_velocity_buffer_shader()
{
	if(!g_velocityBufferShader)
		g_velocityBufferShader = static_cast<ShaderVelocityBuffer *>(get_cengine()->GetShader("velocity_buffer").get());
	return g_velocityBufferShader;
}

void CMotionBlurDataComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

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

	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CRenderComponent>>();
	for(auto *ent : entIt) {
		auto &r = *static_cast<ecs::CBaseEntity *>(ent)->GetRenderComponent();
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
				auto boneBuffer = get_instance_bone_buffer()->AllocateBuffer();
				m_motionBlurData.prevModelMatrices[ent].boneBuffer = boneBuffer;

				auto dsg = velShader->CreateDescriptorSetGroup(ShaderVelocityBuffer::DESCRIPTOR_SET_BONE_BUFFER.setIndex);
				dsg->GetDescriptorSet()->SetBindingUniformBuffer(*boneBuffer, 0);
				m_motionBlurData.prevModelMatrices[ent].boneDsg = dsg;
			}
		}

		m_motionBlurData.curModelMatrices[ent] = {curPose, ent->GetPose()};
	}
}
