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

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

CPhysWaterSurfaceSimulator::CPhysWaterSurfaceSimulator(Vector2 aabbMin,Vector2 aabbMax,float originY,uint32_t spacing,float stiffness,float propagation)
	: PhysWaterSurfaceSimulator(aabbMin,aabbMax,originY,spacing,stiffness,propagation)
{
	m_cmdBuffer = c_engine->AllocatePrimaryLevelCommandBuffer(Anvil::QueueFamilyType::COMPUTE,m_universalQueueFamilyIndex);
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
	m_triangleIndices.reserve((width -1) *(length -1) *6);
	for(auto i=decltype(width){0};i<(width -1);++i)
	{
		for(auto j=decltype(length){0};j<(length -1);++j)
		{
			auto ptIdx0 = GetParticleIndex(m_surfaceInfo,i,j);
			auto ptIdx1 = GetParticleIndex(m_surfaceInfo,i +1,j);
			auto ptIdx2 = GetParticleIndex(m_surfaceInfo,i,j +1);
			m_triangleIndices.push_back(ptIdx0);
			m_triangleIndices.push_back(ptIdx1);
			m_triangleIndices.push_back(ptIdx2);

			auto ptIdx3 = GetParticleIndex(m_surfaceInfo,i +1,j +1);
			m_triangleIndices.push_back(ptIdx3);
			m_triangleIndices.push_back(ptIdx2);
			m_triangleIndices.push_back(ptIdx1);
		}
	}
	m_particlePositions.resize(m_particleField.size());

	if(
		m_bUseComputeShaders == false || m_whShaderSurface.expired() || 
		m_whShaderSurfaceIntegrate.expired() || m_whShaderSurfaceSolveEdges.expired() || 
		m_whShaderSurfaceSumEdges.expired() || pragma::ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT.IsValid() == false ||
		pragma::ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT.IsValid() == false || pragma::ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES.IsValid() == false ||
		pragma::ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO.IsValid() == false || pragma::ShaderWaterSurfaceSolveEdges::DESCRIPTOR_SET_WATER.IsValid() == false
	)
		return;
	auto &dev = c_engine->GetDevice();
	auto &shaderWaterSurface = static_cast<pragma::ShaderWaterSurface&>(*m_whShaderSurface.get());
	m_descSetGroupParticles = prosper::util::create_descriptor_set_group(dev,pragma::ShaderWaterSurface::DESCRIPTOR_SET_WATER_EFFECT);
	m_descSetGroupSplash = prosper::util::create_descriptor_set_group(dev,pragma::ShaderWaterSplash::DESCRIPTOR_SET_WATER_EFFECT);

	auto &shaderWaterSurfaceIntegrate = static_cast<pragma::ShaderWaterSurfaceIntegrate&>(*m_whShaderSurfaceIntegrate.get());
	m_descSetGroupIntegrate = prosper::util::create_descriptor_set_group(dev,pragma::ShaderWaterSurfaceIntegrate::DESCRIPTOR_SET_WATER_PARTICLES);

	auto size = sizeof(Particle) *m_particleField.size();
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT;
	createInfo.size = size;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	m_particleBuffer = prosper::util::create_buffer(dev,createInfo,m_particleField.data());

	// TODO
	///size = sizeof(Vector3) *m_particlePositions.size();
	///m_positionBuffer = Vulkan::Buffer::Create(context,Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT | Anvil::BufferUsageFlagBits::VERTEX_BUFFER_BIT,size,size,m_particlePositions.data(),true,nullptr);
	//size = sizeof(Vertex) *m_particlePositions.size();
	//std::vector<Vertex> vertices(m_particlePositions.size());
	//for(auto i=decltype(vertices.size()){0};i<vertices.size();++i)
	//	vertices.at(i).position = m_particlePositions.at(i);
	//m_positionBuffer = Vulkan::Buffer::Create(context,Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT | Anvil::BufferUsageFlagBits::VERTEX_BUFFER_BIT,size,size,vertices.data(),true,nullptr);

	size = sizeof(Vector4) *m_particleField.size();
	std::vector<Vector4> verts;
	verts.resize(m_particleField.size());

	createInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT | Anvil::BufferUsageFlagBits::VERTEX_BUFFER_BIT;
	createInfo.size = size;
	m_positionBuffer = prosper::util::create_buffer(dev,createInfo,verts.data());

	auto &descSetParticles = *m_descSetGroupParticles->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_storage_buffer(descSetParticles,*m_particleBuffer,umath::to_integral(pragma::ShaderWaterSurface::WaterEffectBinding::WaterParticles));
	prosper::util::set_descriptor_set_binding_storage_buffer(descSetParticles,*m_positionBuffer,umath::to_integral(pragma::ShaderWaterSurface::WaterEffectBinding::WaterPositions));

	auto &descSetSplash = *m_descSetGroupSplash->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_storage_buffer(descSetSplash,*m_particleBuffer,umath::to_integral(pragma::ShaderWaterSplash::WaterEffectBinding::WaterParticles));
	prosper::util::set_descriptor_set_binding_storage_buffer(descSetSplash,*m_positionBuffer,umath::to_integral(pragma::ShaderWaterSplash::WaterEffectBinding::WaterPositions));

	auto &descSetIntegrate = *m_descSetGroupIntegrate->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_storage_buffer(descSetIntegrate,*m_particleBuffer,umath::to_integral(pragma::ShaderWaterSurfaceIntegrate::WaterParticlesBinding::WaterParticles));

	// Initialize surface info buffer
	size = sizeof(m_surfaceInfo);
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT;
	createInfo.size = size;
	m_surfaceInfoBuffer = prosper::util::create_buffer(dev,createInfo,&m_surfaceInfo);
	m_descSetGroupSurfaceInfo = prosper::util::create_descriptor_set_group(dev,pragma::ShaderWaterSurface::DESCRIPTOR_SET_SURFACE_INFO);
	auto &descSetSurfaceInfo = *m_descSetGroupSurfaceInfo->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_uniform_buffer(descSetSurfaceInfo,*m_surfaceInfoBuffer,umath::to_integral(pragma::ShaderWaterSurface::SurfaceInfoBinding::SurfaceInfo));

	// Initialize edge buffer
	std::vector<ParticleEdgeInfo> particleEdgeInfo(m_particleField.size());
	size = sizeof(particleEdgeInfo.front()) *particleEdgeInfo.size();
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT;
	createInfo.size = size;
	m_edgeBuffer = prosper::util::create_buffer(dev,createInfo,particleEdgeInfo.data());
	auto &shaderWaterSurfaceSolveEdges = static_cast<pragma::ShaderWaterSurfaceSolveEdges&>(*m_whShaderSurfaceSolveEdges.get());
	m_edgeDescSetGroup = prosper::util::create_descriptor_set_group(dev,pragma::ShaderWaterSurfaceSolveEdges::DESCRIPTOR_SET_WATER);
	auto &descSetEdge = *m_edgeDescSetGroup->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_storage_buffer(descSetEdge,*m_particleBuffer,umath::to_integral(pragma::ShaderWaterSurfaceSolveEdges::WaterBinding::WaterParticles));
	prosper::util::set_descriptor_set_binding_storage_buffer(descSetEdge,*m_edgeBuffer,umath::to_integral(pragma::ShaderWaterSurfaceSolveEdges::WaterBinding::WaterEdgeData));
}

const std::shared_ptr<prosper::Buffer> &CPhysWaterSurfaceSimulator::GetParticleBuffer() const {return m_particleBuffer;}
const std::shared_ptr<prosper::Buffer> &CPhysWaterSurfaceSimulator::GetPositionBuffer() const {return m_positionBuffer;}

static auto cvEdgeIterationCount = GetClientConVar("cl_water_surface_simulation_edge_iteration_count");
uint8_t CPhysWaterSurfaceSimulator::GetEdgeIterationCount() const {return cvEdgeIterationCount->GetInt();}
#include "pragma/entities/components/c_player_component.hpp"
void CPhysWaterSurfaceSimulator::Simulate(double dt)
{
	if(m_bUseComputeShaders == false)
	{
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
	if((*computeCmd)->start_recording(false,false) == false)
		return;
	// Apply splashes
	if(m_splashQueue.empty() == false)
	{
		auto &shaderWaterSplash = static_cast<pragma::ShaderWaterSplash&>(*m_whShaderWaterSplash.get());
		if(shaderWaterSplash.BeginCompute(computeCmd) == true)
		{
			while(m_splashQueue.empty() == false)
			{
				shaderWaterSplash.Compute(*(*m_descSetGroupSplash)->get_descriptor_set(0u),m_splashQueue.front());
				m_splashQueue.pop();
			}
			shaderWaterSplash.EndCompute();
			prosper::util::record_buffer_barrier(
				*(*computeCmd),*m_particleBuffer,
				Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
				Anvil::AccessFlagBits::SHADER_READ_BIT | Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT | Anvil::AccessFlagBits::SHADER_WRITE_BIT
			);
		}
	}
	//

	auto width = GetWidth();
	auto length = GetLength();

	// Integrate
	auto &shaderWaterSurfaceIntegrate = static_cast<pragma::ShaderWaterSurfaceIntegrate&>(*m_whShaderSurfaceIntegrate.get());
	if(shaderWaterSurfaceIntegrate.BeginCompute(computeCmd) == true)
	{
		shaderWaterSurfaceIntegrate.Compute(*(*m_descSetGroupSurfaceInfo)->get_descriptor_set(0u),*(*m_descSetGroupIntegrate)->get_descriptor_set(0u),width,length);
		shaderWaterSurfaceIntegrate.EndCompute();

		prosper::util::record_buffer_barrier(
			*(*computeCmd),*m_particleBuffer,
			Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
			Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT | Anvil::AccessFlagBits::SHADER_WRITE_BIT
		);
	}
	//

	// Solve edges
	auto &shaderWaterSolveEdges = static_cast<pragma::ShaderWaterSurfaceSolveEdges&>(*m_whShaderSurfaceSolveEdges.get());
	auto &shaderWaterSumEdges = static_cast<pragma::ShaderWaterSurfaceSumEdges&>(*m_whShaderSurfaceSumEdges.get());
	auto sovleEdgeCount = GetEdgeIterationCount();
	for(auto i=decltype(sovleEdgeCount){0};i<sovleEdgeCount;++i)
	{
		if(shaderWaterSolveEdges.BeginCompute(computeCmd) == true)
		{
			shaderWaterSolveEdges.Compute(*(*m_descSetGroupSurfaceInfo)->get_descriptor_set(0u),*(*m_edgeDescSetGroup)->get_descriptor_set(0u),width,length);
			shaderWaterSolveEdges.EndCompute();

			prosper::util::record_buffer_barrier(
				*(*computeCmd),*m_edgeBuffer,
				Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
				Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT | Anvil::AccessFlagBits::SHADER_WRITE_BIT
			);

			if(shaderWaterSumEdges.BeginCompute(computeCmd) == true)
			{
				shaderWaterSumEdges.Compute(*(*m_descSetGroupSurfaceInfo)->get_descriptor_set(0u),*(*m_edgeDescSetGroup)->get_descriptor_set(0u),width,length);
				shaderWaterSumEdges.EndCompute();

				prosper::util::record_buffer_barrier(
					*(*computeCmd),*m_particleBuffer,
					Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
					Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT | Anvil::AccessFlagBits::SHADER_WRITE_BIT
				);
				prosper::util::record_buffer_barrier(
					*(*computeCmd),*m_edgeBuffer,
					Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
					Anvil::AccessFlagBits::SHADER_READ_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT | Anvil::AccessFlagBits::SHADER_WRITE_BIT
				);
			}
		}
	}
	//

	//c_engine->StartGPUTimer(GPUTimerEvent::WaterSurface); // prosper TODO
	auto &shaderWaterSurface = static_cast<pragma::ShaderWaterSurface&>(*m_whShaderSurface.get());
	if(shaderWaterSurface.BeginCompute(computeCmd) == true)
	{
		shaderWaterSurface.Compute(*(*m_descSetGroupSurfaceInfo)->get_descriptor_set(0u),*(*m_descSetGroupParticles)->get_descriptor_set(0u),width,length);
		shaderWaterSurface.EndCompute();
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::WaterSurface); // prosper TODO
	(*computeCmd)->stop_recording();
	c_engine->SubmitCommandBuffer(*computeCmd);
}

const std::vector<uint16_t> &CPhysWaterSurfaceSimulator::GetTriangleIndices() const {return m_triangleIndices;}

void CPhysWaterSurfaceSimulator::Draw(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,CModelSubMesh &mesh)
{
	// TODO
	auto &verts = mesh.GetVertices();
	//std::vector<Vector3> lines;
	//lines.reserve(verts.size() *2);
	//auto prevPos = Vector3{};

	std::vector<Vector4> particlePositions(m_particleField.size());
	m_positionBuffer->Read(0ull,particlePositions.size() *sizeof(particlePositions.front()),particlePositions.data());

	auto numVerts = umath::min(verts.size(),GetParticleCount());
	for(auto i=decltype(numVerts){0};i<numVerts;++i)
	{
		auto pos = CalcParticlePosition(i);//m_particlePositions.at(i); // TODO: Remove m_particlePositions?

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
	auto &vkMesh = mesh.GetVKMesh();
	prosper::util::record_update_buffer(
		*(*drawCmd),*vkMesh->GetVertexBuffer(),
		0ull,verts.size() *sizeof(verts.front()),verts.data()
	);
	//




	//static auto tNextDraw = 0.0;
	//auto &t = c_game->CurTime();
	//if(tNextDraw > t)
	//	return;
	//auto duration = 0.1f;
	//tNextDraw = t +duration;

	// Test Normals
	/*static std::vector<Vector3> faceNormals = {};
	faceNormals.clear();
	faceNormals.reserve(m_triangleIndices.size() /3);
	for(auto i=decltype(m_triangleIndices.size()){0};i<m_triangleIndices.size();i+=3)
	{
		auto idx0 = m_triangleIndices.at(i);
		auto idx1 = m_triangleIndices.at(i +1);
		auto idx2 = m_triangleIndices.at(i +2);
		faceNormals.push_back(Geometry::CalcFaceNormal(particlePositions.at(idx0),particlePositions.at(idx1),particlePositions.at(idx2)));
	}
	static std::vector<std::array<std::size_t,4>> vertTriangles;
	if(vertTriangles.empty() == true)
	{
		vertTriangles.resize(m_particlePositions.size());
		for(auto &a : vertTriangles)
		{
			for(auto &v : a)
				v = std::numeric_limits<std::size_t>::max();
		}
		const auto fAddIndex = [](std::size_t particleIdx,std::size_t triangleIdx) {
			auto &a = vertTriangles.at(particleIdx);
			for(auto &v : a)
			{
				if(v != std::numeric_limits<std::size_t>::max())
					continue;
				v = triangleIdx;
			}
		};
		for(auto i=decltype(m_triangleIndices.size()){0};i<m_triangleIndices.size();i+=3)
		{
			auto idx0 = m_triangleIndices.at(i);
			auto idx1 = m_triangleIndices.at(i +1);
			auto idx2 = m_triangleIndices.at(i +2);
			fAddIndex(idx0,i /3);
			fAddIndex(idx1,i /3);
			fAddIndex(idx2,i /3);
		}
	}
	for(auto i=decltype(particlePositions.size()){0};i<particlePositions.size();++i)
	{
		auto &triangles = vertTriangles.at(i);
		Vector3 ptNormal {};
		std::size_t count = 0;
		for(auto &triIdx : triangles)
		{
			if(triIdx == std::numeric_limits<std::size_t>::max())
				break;
			auto &n = faceNormals.at(triIdx);
			ptNormal += n;
			++count;
		}
		if(count > 0)
			ptNormal /= static_cast<float>(count);
		uvec::normalize(&ptNormal);
		verts.at(i).normal = ptNormal;
	}*/
	//
	//DebugRenderer::DrawLines(lines,Color::Red,0.1f);

	//std::vector<Vector3> verts;
	//verts.reserve(m_triangleIndices.size());
	//for(auto idx : m_triangleIndices)
	//	verts.push_back(GetParticlePosition(idx));
	//DebugRenderer::DrawMesh(verts,Color::Red,duration);


}
