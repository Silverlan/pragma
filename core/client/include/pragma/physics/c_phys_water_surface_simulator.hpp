/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
	void Draw(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,CModelSubMesh &mesh);
	const std::vector<uint16_t> &GetTriangleIndices() const;
	virtual void Simulate(double dt) override;
	const std::shared_ptr<prosper::IBuffer> &GetParticleBuffer() const;
	const std::shared_ptr<prosper::IBuffer> &GetPositionBuffer() const;

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
	
	std::shared_ptr<prosper::IPrimaryCommandBuffer> m_cmdBuffer = nullptr;
	uint32_t m_universalQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
	::util::WeakHandle<prosper::Shader> m_whShaderSurface = {};
	::util::WeakHandle<prosper::Shader> m_whShaderSurfaceIntegrate = {};
	::util::WeakHandle<prosper::Shader> m_whShaderSurfaceSolveEdges = {};
	::util::WeakHandle<prosper::Shader> m_whShaderSurfaceSumEdges = {};
	::util::WeakHandle<prosper::Shader> m_whShaderWaterSplash = {};

	std::shared_ptr<prosper::IBuffer> m_particleBuffer = nullptr;
	std::shared_ptr<prosper::IBuffer> m_positionBuffer = nullptr;
	
	std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupParticles = nullptr;
	std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupSplash = nullptr;
	std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupIntegrate = nullptr;
	std::vector<Vector3> m_particlePositions;
	bool m_bUseComputeShaders = false;

	// Edges
	std::shared_ptr<prosper::IBuffer> m_edgeBuffer = nullptr;
	std::shared_ptr<prosper::IDescriptorSetGroup> m_edgeDescSetGroup = nullptr;

	std::shared_ptr<prosper::IBuffer> m_surfaceInfoBuffer = nullptr;
	std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupSurfaceInfo = nullptr;
};

#endif
