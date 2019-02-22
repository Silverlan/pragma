#ifndef __C_PHYS_WATER_SURFACE_SIMULATOR_HPP__
#define __C_PHYS_WATER_SURFACE_SIMULATOR_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/water/c_shader_water_splash.hpp"
#include <pragma/physics/phys_water_surface_simulator.hpp>

class DLLCLIENT CPhysWaterSurfaceSimulator
	: public PhysWaterSurfaceSimulator
{
public:
	CPhysWaterSurfaceSimulator(Vector2 aabbMin,Vector2 aabbMax,float originY,uint32_t spacing,float stiffness=0.1f,float propagation=100.f);
	void Draw(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,CModelSubMesh &mesh);
	const std::vector<uint16_t> &GetTriangleIndices() const;
	virtual void Simulate(double dt) override;
	const std::shared_ptr<prosper::Buffer> &GetParticleBuffer() const;
	const std::shared_ptr<prosper::Buffer> &GetPositionBuffer() const;

#pragma pack(push,1)
	struct DLLCLIENT ParticleEdgeInfo
	{
		Vector4 thisToNb = {};
		Vector4 nbToThis = {};
	};
#pragma pack(pop)
protected:
	virtual uint8_t GetEdgeIterationCount() const override;
	virtual void InitializeSurface() override;
	std::vector<uint16_t> m_triangleIndices;
	
	std::shared_ptr<prosper::PrimaryCommandBuffer> m_cmdBuffer = nullptr;
	uint32_t m_universalQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
	::util::WeakHandle<prosper::Shader> m_whShaderSurface = {};
	::util::WeakHandle<prosper::Shader> m_whShaderSurfaceIntegrate = {};
	::util::WeakHandle<prosper::Shader> m_whShaderSurfaceSolveEdges = {};
	::util::WeakHandle<prosper::Shader> m_whShaderSurfaceSumEdges = {};
	::util::WeakHandle<prosper::Shader> m_whShaderWaterSplash = {};

	std::shared_ptr<prosper::Buffer> m_particleBuffer = nullptr;
	std::shared_ptr<prosper::Buffer> m_positionBuffer = nullptr;
	
	std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupParticles = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupSplash = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupIntegrate = nullptr;
	std::vector<Vector3> m_particlePositions;
	bool m_bUseComputeShaders = false;

	// Edges
	std::shared_ptr<prosper::Buffer> m_edgeBuffer = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> m_edgeDescSetGroup = nullptr;

	std::shared_ptr<prosper::Buffer> m_surfaceInfoBuffer = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupSurfaceInfo = nullptr;
};

#endif
