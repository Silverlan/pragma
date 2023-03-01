/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/physics/c_phys_water_surface_simulator.hpp"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_integrate.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_solve_edges.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water_surface_sum_edges.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/model/vk_mesh.h"
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp> // prosper TODO: Remove
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

CPhysWaterSurfaceSimulator::CPhysWaterSurfaceSimulator(Vector2 aabbMin, Vector2 aabbMax, float originY, uint32_t spacing, float stiffness, float propagation) : PhysWaterSurfaceSimulator(aabbMin, aabbMax, originY, spacing, stiffness, propagation)
{
	m_cmdBuffer = c_engine->GetRenderContext().AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType::Compute, m_universalQueueFamilyIndex);
	m_whShaderSurface = c_engine->GetShader("watersurface");
	m_whShaderSurfaceIntegrate = c_engine->GetShader("watersurfaceintegrate");
	m_whShaderSurfaceSolveEdges = c_engine->GetShader("watersurfacesolveedges");
	m_whShaderSurfaceSumEdges = c_engine->GetShader("watersurfacesumedges");
	m_whShaderWaterSplash = c_engine->GetShader("watersplash");
}

static auto cvGPUAcceleration = GetClientConVar("cl_water_surface_simulation_enable_gpu_acceleration");
void CPhysWaterSurfaceSimulator::InitializeSurface()
{
	PhysWaterSurfaceSimulator::InitializeSurface();
	if(m_particleField.empty() == true)
		return;
	m_bUseComputeShaders = cvGPUAcceleration->GetBool();
	m_bUseThread = !m_bUseComputeShaders;
	auto width = GetWidth();
	auto length = GetLength();
	m_triangleIndices.reserve((width - 1) * (length - 1) * 6);
	for(auto i = decltype(width) {0}; i < (width - 1); ++i) {
		for(auto j = decltype(length) {0}; j < (length - 1); ++j) {
			auto ptIdx0 = GetParticleIndex(m_surfaceInfo, i, j);
			auto ptIdx1 = GetParticleIndex(m_surfaceInfo, i + 1, j);
			auto ptIdx2 = GetParticleIndex(m_surfaceInfo, i, j + 1);
			m_triangleIndices.push_back(ptIdx0);
			m_triangleIndices.push_back(ptIdx1);
			m_triangleIndices.push_back(ptIdx2);

			auto ptIdx3 = GetParticleIndex(m_surfaceInfo, i + 1, j + 1);
			m_triangleIndices.push_back(ptIdx3);
			m_triangleIndices.push_back(ptIdx2);
			m_triangleIndices.push_back(ptIdx1);
		}
	}
	m_particlePositions.resize(m_particleField.size());

	if(m_bUseComputeShaders == false || m_whShaderSurface.expired() || m_whShaderSurfaceIntegrate.expired() || m_whShaderSurfaceSolveEdges.expired() || m_whShaderSurfaceSumEdges.expired() || pragma::ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT.IsValid() == false
	  || pragma::ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT.IsValid() == false || pragma::ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES.IsValid() == false || pragma::ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO.IsValid() == false
	  || pragma::ShaderWaterSurfaceSolveEdges::DESCRIPTOR_SET_WATER.IsValid() == false)
		return;
	auto &shaderWaterSurface = static_cast<pragma::ShaderWaterSurface &>(*m_whShaderSurface.get());
	m_descSetGroupParticles = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT);
	m_descSetGroupSplash = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT);

	auto &shaderWaterSurfaceIntegrate = static_cast<pragma::ShaderWaterSurfaceIntegrate &>(*m_whShaderSurfaceIntegrate.get());
	m_descSetGroupIntegrate = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES);

	auto size = sizeof(Particle) * m_particleField.size();
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit;
	createInfo.size = size;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	m_particleBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo, m_particleField.data());

	// TODO
	///size = sizeof(Vector3) *m_particlePositions.size();
	///m_positionBuffer = Vulkan::Buffer::Create(context,prosper::BufferUsageFlags::StorageBufferBit | prosper::BufferUsageFlags::VertexBufferBit,size,size,m_particlePositions.data(),true,nullptr);
	//size = sizeof(Vertex) *m_particlePositions.size();
	//std::vector<Vertex> vertices(m_particlePositions.size());
	//for(auto i=decltype(vertices.size()){0};i<vertices.size();++i)
	//	vertices.at(i).position = m_particlePositions.at(i);
	//m_positionBuffer = Vulkan::Buffer::Create(context,prosper::BufferUsageFlags::StorageBufferBit | prosper::BufferUsageFlags::VertexBufferBit,size,size,vertices.data(),true,nullptr);

	size = sizeof(Vector4) * m_particleField.size();
	std::vector<Vector4> verts;
	verts.resize(m_particleField.size());

	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit | prosper::BufferUsageFlags::VertexBufferBit;
	createInfo.size = size;
	m_positionBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo, verts.data());

	auto &descSetParticles = *m_descSetGroupParticles->GetDescriptorSet();
	descSetParticles.SetBindingStorageBuffer(*m_particleBuffer, umath::to_integral(pragma::ShaderWaterSurface::WaterEffectBinding::WaterParticles));
	descSetParticles.SetBindingStorageBuffer(*m_positionBuffer, umath::to_integral(pragma::ShaderWaterSurface::WaterEffectBinding::WaterPositions));

	auto &descSetSplash = *m_descSetGroupSplash->GetDescriptorSet();
	descSetSplash.SetBindingStorageBuffer(*m_particleBuffer, umath::to_integral(pragma::ShaderWaterSplash::WaterEffectBinding::WaterParticles));
	descSetSplash.SetBindingStorageBuffer(*m_positionBuffer, umath::to_integral(pragma::ShaderWaterSplash::WaterEffectBinding::WaterPositions));

	auto &descSetIntegrate = *m_descSetGroupIntegrate->GetDescriptorSet();
	descSetIntegrate.SetBindingStorageBuffer(*m_particleBuffer, umath::to_integral(pragma::ShaderWaterSurfaceIntegrate::WaterParticlesBinding::WaterParticles));

	// Initialize surface info buffer
	size = sizeof(m_surfaceInfo);
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	createInfo.size = size;
	m_surfaceInfoBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo, &m_surfaceInfo);
	m_descSetGroupSurfaceInfo = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO);
	auto &descSetSurfaceInfo = *m_descSetGroupSurfaceInfo->GetDescriptorSet();
	descSetSurfaceInfo.SetBindingUniformBuffer(*m_surfaceInfoBuffer, umath::to_integral(pragma::ShaderWaterSurface::SurfaceInfoBinding::SurfaceInfo));

	// Initialize edge buffer
	std::vector<ParticleEdgeInfo> particleEdgeInfo(m_particleField.size());
	size = sizeof(particleEdgeInfo.front()) * particleEdgeInfo.size();
	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit;
	createInfo.size = size;
	m_edgeBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo, particleEdgeInfo.data());
	auto &shaderWaterSurfaceSolveEdges = static_cast<pragma::ShaderWaterSurfaceSolveEdges &>(*m_whShaderSurfaceSolveEdges.get());
	m_edgeDescSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderWaterSurfaceSolveEdges::DESCRIPTOR_SET_WATER);
	auto &descSetEdge = *m_edgeDescSetGroup->GetDescriptorSet();
	descSetEdge.SetBindingStorageBuffer(*m_particleBuffer, umath::to_integral(pragma::ShaderWaterSurfaceSolveEdges::WaterBinding::WaterParticles));
	descSetEdge.SetBindingStorageBuffer(*m_edgeBuffer, umath::to_integral(pragma::ShaderWaterSurfaceSolveEdges::WaterBinding::WaterEdgeData));
}

const std::shared_ptr<prosper::IBuffer> &CPhysWaterSurfaceSimulator::GetParticleBuffer() const { return m_particleBuffer; }
const std::shared_ptr<prosper::IBuffer> &CPhysWaterSurfaceSimulator::GetPositionBuffer() const { return m_positionBuffer; }

static auto cvEdgeIterationCount = GetClientConVar("cl_water_surface_simulation_edge_iteration_count");
uint8_t CPhysWaterSurfaceSimulator::GetEdgeIterationCount() const { return cvEdgeIterationCount->GetInt(); }
#include "pragma/entities/components/c_player_component.hpp"
void CPhysWaterSurfaceSimulator::Simulate(double dt)
{
	if(m_bUseComputeShaders == false) {
		PhysWaterSurfaceSimulator::Simulate(dt);
		return;
	}
	if(m_cmdBuffer == nullptr || m_descSetGroupParticles == nullptr || m_descSetGroupSplash == nullptr)
		return;
	if(m_whShaderSurface.expired() || m_whShaderSurfaceIntegrate.expired() || m_whShaderSurfaceSolveEdges.expired() || m_whShaderSurfaceSumEdges.expired() || m_whShaderWaterSplash.expired())
		return;
	/*static auto bSplash = false;
	if(bSplash == true)
	{
		bSplash = false;
		static auto radius = 80.f;
		static auto force = 200.f;
		CreateSplash(c_game->GetLocalPlayer()->GetPosition(),radius,force);
	}*/
	auto &computeCmd = m_cmdBuffer;
	if(computeCmd->StartRecording(false, false) == false)
		return;
	// Apply splashes
	prosper::ShaderBindState bindState {*computeCmd};
	if(m_splashQueue.empty() == false) {
		auto &shaderWaterSplash = static_cast<pragma::ShaderWaterSplash &>(*m_whShaderWaterSplash.get());
		if(shaderWaterSplash.RecordBeginCompute(bindState) == true) {
			while(m_splashQueue.empty() == false) {
				shaderWaterSplash.RecordCompute(bindState, *m_descSetGroupSplash->GetDescriptorSet(), m_splashQueue.front());
				m_splashQueue.pop();
			}
			shaderWaterSplash.RecordEndCompute(bindState);
			computeCmd->RecordBufferBarrier(*m_particleBuffer, prosper::PipelineStageFlags::ComputeShaderBit, prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::ShaderReadBit | prosper::AccessFlags::ShaderWriteBit,
			  prosper::AccessFlags::ShaderReadBit | prosper::AccessFlags::ShaderWriteBit);
		}
	}
	//

	auto width = GetWidth();
	auto length = GetLength();

	// Integrate
	auto &shaderWaterSurfaceIntegrate = static_cast<pragma::ShaderWaterSurfaceIntegrate &>(*m_whShaderSurfaceIntegrate.get());
	if(shaderWaterSurfaceIntegrate.RecordBeginCompute(bindState) == true) {
		shaderWaterSurfaceIntegrate.RecordCompute(bindState, *m_descSetGroupSurfaceInfo->GetDescriptorSet(), *m_descSetGroupIntegrate->GetDescriptorSet(), width, length);
		shaderWaterSurfaceIntegrate.RecordEndCompute(bindState);

		computeCmd->RecordBufferBarrier(*m_particleBuffer, prosper::PipelineStageFlags::ComputeShaderBit, prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::ShaderWriteBit, prosper::AccessFlags::ShaderReadBit | prosper::AccessFlags::ShaderWriteBit);
	}
	//

	// Solve edges
	auto &shaderWaterSolveEdges = static_cast<pragma::ShaderWaterSurfaceSolveEdges &>(*m_whShaderSurfaceSolveEdges.get());
	auto &shaderWaterSumEdges = static_cast<pragma::ShaderWaterSurfaceSumEdges &>(*m_whShaderSurfaceSumEdges.get());
	auto sovleEdgeCount = GetEdgeIterationCount();
	for(auto i = decltype(sovleEdgeCount) {0}; i < sovleEdgeCount; ++i) {
		if(shaderWaterSolveEdges.RecordBeginCompute(bindState) == true) {
			shaderWaterSolveEdges.RecordCompute(bindState, *m_descSetGroupSurfaceInfo->GetDescriptorSet(), *m_edgeDescSetGroup->GetDescriptorSet(), width, length);
			shaderWaterSolveEdges.RecordEndCompute(bindState);

			computeCmd->RecordBufferBarrier(*m_edgeBuffer, prosper::PipelineStageFlags::ComputeShaderBit, prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::ShaderWriteBit, prosper::AccessFlags::ShaderReadBit | prosper::AccessFlags::ShaderWriteBit);

			if(shaderWaterSumEdges.RecordBeginCompute(bindState) == true) {
				shaderWaterSumEdges.RecordCompute(bindState, *m_descSetGroupSurfaceInfo->GetDescriptorSet(), *m_edgeDescSetGroup->GetDescriptorSet(), width, length);
				shaderWaterSumEdges.RecordEndCompute(bindState);

				computeCmd->RecordBufferBarrier(*m_particleBuffer, prosper::PipelineStageFlags::ComputeShaderBit, prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::ShaderWriteBit, prosper::AccessFlags::ShaderReadBit | prosper::AccessFlags::ShaderWriteBit);
				computeCmd->RecordBufferBarrier(*m_edgeBuffer, prosper::PipelineStageFlags::ComputeShaderBit, prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::ShaderReadBit, prosper::AccessFlags::ShaderReadBit | prosper::AccessFlags::ShaderWriteBit);
			}
		}
	}
	//

	//c_engine->StartGPUTimer(GPUTimerEvent::WaterSurface); // prosper TODO
	auto &shaderWaterSurface = static_cast<pragma::ShaderWaterSurface &>(*m_whShaderSurface.get());
	if(shaderWaterSurface.RecordBeginCompute(bindState) == true) {
		shaderWaterSurface.RecordCompute(bindState, *m_descSetGroupSurfaceInfo->GetDescriptorSet(), *m_descSetGroupParticles->GetDescriptorSet(), width, length);
		shaderWaterSurface.RecordEndCompute(bindState);
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::WaterSurface); // prosper TODO
	computeCmd->StopRecording();
	c_engine->GetRenderContext().SubmitCommandBuffer(*computeCmd);
}

const std::vector<uint16_t> &CPhysWaterSurfaceSimulator::GetTriangleIndices() const { return m_triangleIndices; }

void CPhysWaterSurfaceSimulator::Draw(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, CModelSubMesh &mesh)
{
	// TODO
	auto &verts = mesh.GetVertices();
	//std::vector<Vector3> lines;
	//lines.reserve(verts.size() *2);
	//auto prevPos = Vector3{};

	std::vector<Vector4> particlePositions(m_particleField.size());
	m_positionBuffer->Read(0ull, particlePositions.size() * sizeof(particlePositions.front()), particlePositions.data());

	auto numVerts = umath::min(verts.size(), GetParticleCount());
	for(auto i = decltype(numVerts) {0}; i < numVerts; ++i) {
		auto pos = CalcParticlePosition(i); //m_particlePositions.at(i); // TODO: Remove m_particlePositions?

		pos.y = particlePositions.at(i).y; // TODO: Write data directly into mesh vertex buffer, so there's no need for a separate position buffer
		//lines.push_back(prevPos);
		//lines.push_back(pos);
		//lines.push_back(pos +Vector3(0.f,64.f,0.f));
		//prevPos = pos;

		//uvec::world_to_local(corigin,rorigin,pos);
		//pos -= Vector3{-664.f,-201.f,737.f}; // TODO: Transformation matrix
		//static Vector3 offset {-700.f,195.5f,-493.5f};
		//pos += offset;
		verts.at(i).position = pos;
	}
	auto &vkMesh = mesh.GetSceneMesh();
	drawCmd->RecordUpdateBuffer(*vkMesh->GetVertexBuffer(), 0ull, verts.size() * sizeof(verts.front()), verts.data());
	//
}
