// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.renderer_rotational_buffer;

import :client_state;

void pragma::pts::CParticleRendererRotationalBuffer::Initialize(BaseEnvParticleSystemComponent &pSystem)
{
	m_hParticleSystem = pSystem.GetHandle<ecs::CParticleSystemComponent>();

	auto maxParticles = static_cast<ecs::CParticleSystemComponent &>(pSystem).GetMaxParticleCount();
	m_rotations.resize(maxParticles);

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = m_rotations.size() * sizeof(m_rotations.front());
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	m_rotBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo, m_rotations.data());
}

void pragma::pts::CParticleRendererRotationalBuffer::SetRotationAlignVelocity(bool b) { m_bAlignVelocity = b; }
bool pragma::pts::CParticleRendererRotationalBuffer::ShouldRotationAlignVelocity() const { return m_bAlignVelocity; }
const std::shared_ptr<prosper::IBuffer> &pragma::pts::CParticleRendererRotationalBuffer::GetBuffer() const { return m_rotBuffer; }

bool pragma::pts::CParticleRendererRotationalBuffer::Update()
{
	//auto frameId = pragma::get_cengine()->GetRenderContext().GetLastFrameId();
	if(/*frameId == m_lastFrameUpdate || */ m_hParticleSystem.expired())
		return false;
	//m_lastFrameUpdate = frameId;

	// Update rotation buffer
	auto *ps = m_hParticleSystem.get();
	auto pTrComponent = ps->GetEntity().GetTransformComponent();
	auto psRot = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
	if(ps->IsRendererBufferUpdateRequired() == true) {
		auto numParticles = ps->GetRenderParticleCount();
		for(auto i = decltype(numParticles) {0}; i < numParticles; ++i) {
			auto particleIdx = ps->TranslateBufferIndex(i);
			auto *pt = ps->GetParticle(particleIdx);
			if(pt == nullptr)
				continue;
			if(m_bAlignVelocity == false) {
				auto &rot = pt->GetWorldRotation();
				m_rotations.at(i) = rot;
				if(ps->ShouldParticlesRotateWithEmitter())
					m_rotations.at(i) = psRot * m_rotations.at(i);
			}
			else {
				auto vel = pt->GetVelocity();
				auto l = uvec::length(vel);
				if(l <= 0.0001f)
					vel = Vector3(0.f, 1.f, 0.f);
				else
					uvec::normalize(&vel);
				m_rotations.at(i) = Quat {0.f, vel.x, vel.y, vel.z};
			}
		}
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_rotBuffer, 0ull, numParticles * sizeof(m_rotations.front()), m_rotations.data());
	}
	return true;
}
