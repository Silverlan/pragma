/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/renderers/c_particle_renderer_rotational_buffer.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void CParticleRendererRotationalBuffer::Initialize(pragma::CParticleSystemComponent &pSystem)
{
	m_hParticleSystem = pSystem.GetHandle<pragma::CParticleSystemComponent>();

	auto maxParticles = pSystem.GetMaxParticleCount();
	m_rotations.resize(maxParticles);

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = m_rotations.size() *sizeof(m_rotations.front());
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	m_rotBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,m_rotations.data());
}

void CParticleRendererRotationalBuffer::SetRotationAlignVelocity(bool b) {m_bAlignVelocity = b;}
bool CParticleRendererRotationalBuffer::ShouldRotationAlignVelocity() const {return m_bAlignVelocity;}
const std::shared_ptr<prosper::IBuffer> &CParticleRendererRotationalBuffer::GetBuffer() const {return m_rotBuffer;}

bool CParticleRendererRotationalBuffer::Update()
{
	//auto frameId = c_engine->GetRenderContext().GetLastFrameId();
	if(/*frameId == m_lastFrameUpdate || */m_hParticleSystem.expired())
		return false;
	//m_lastFrameUpdate = frameId;

	// Update rotation buffer
	auto *ps = m_hParticleSystem.get();
	auto pTrComponent = ps->GetEntity().GetTransformComponent();
	auto psRot = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
	if(ps->IsRendererBufferUpdateRequired() == true)
	{
		auto numParticles = ps->GetRenderParticleCount();
		for(auto i=decltype(numParticles){0};i<numParticles;++i)
		{
			auto particleIdx = ps->TranslateBufferIndex(i);
			auto *pt = ps->GetParticle(particleIdx);
			if(pt == nullptr)
				continue;
			if(m_bAlignVelocity == false)
			{
				auto &rot = pt->GetWorldRotation();
				m_rotations.at(i) = rot;
				if(ps->ShouldParticlesRotateWithEmitter())
					m_rotations.at(i) = psRot *m_rotations.at(i);
			}
			else
			{
				auto vel = pt->GetVelocity();
				auto l = uvec::length(vel);
				if(l <= 0.0001f)
					vel = Vector3(0.f,1.f,0.f);
				else
					uvec::normalize(&vel);
				m_rotations.at(i) = Quat{0.f,vel.x,vel.y,vel.z};
			}
		}
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_rotBuffer,0ull,numParticles *sizeof(m_rotations.front()),m_rotations.data());
	}
	return true;
}

