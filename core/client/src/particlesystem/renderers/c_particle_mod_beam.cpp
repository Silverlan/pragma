/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/renderers/c_particle_mod_beam.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_polyboard.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_polyboard_shadow.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

REGISTER_PARTICLE_RENDERER(beam,CParticleRendererBeam);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

CParticleRendererBeam::Node::Node(const Vector3 &o,const Color&)
	: origin(o),color(1.f,0.f,0.f,1.f)//{c.r /255.f,c.g /255.f,c.b /255.f,c.a /255.f})
{}
void CParticleRendererBeam::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleRenderer::Initialize(pSystem,values);
	m_shader = c_engine->GetShader("particlepolyboard");
	for(auto &it : values)
	{
		auto key = it.first;
		ustring::to_lower(key);
		if(key == "node_start")
			m_startNode = util::to_int(it.second);
		else if(key == "node_end")
			m_endNode = util::to_int(it.second);
		else if(key == "curvature")
			m_curvature = util::to_float(it.second);
	}
	m_startNode = umath::max(m_startNode,static_cast<uint32_t>(1));
	m_endNode = umath::max(m_endNode,static_cast<uint32_t>(1));
	m_nodeCount = (m_endNode -m_startNode) +1;
	// Generate Indices
	std::vector<uint16_t> indices;
	if(m_nodeCount > 0)
	{
		auto numVerts = m_nodeCount;
		indices.reserve(numVerts *4);
		for(auto i=decltype(numVerts){0};i<(numVerts -1);++i)
		{
			indices.push_back(static_cast<uint16_t>((i > 0) ? (i -1) : i)); // Previous
			indices.push_back(static_cast<uint16_t>(i)); // Current
			indices.push_back(static_cast<uint16_t>(i +1)); // Next
			indices.push_back(static_cast<uint16_t>((i < numVerts -2) ? (i +2) : (i +1))); // Second Next
		}
	}
	m_indexCount = static_cast<uint32_t>(indices.size());
	m_nodeOrigins.resize(m_nodeCount,{{},{0,0,0,255}});

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = m_nodeOrigins.size() *sizeof(Node);
	m_vertexBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,m_nodeOrigins.data());

	createInfo.usageFlags = prosper::BufferUsageFlags::IndexBufferBit;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(uint16_t) *indices.size();
	m_indexBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,indices.data());
}
pragma::ShaderParticleBase *CParticleRendererBeam::GetShader() const
{
	return static_cast<pragma::ShaderParticlePolyboard*>(m_shader.get());
}
void CParticleRendererBeam::PostSimulate(double tDelta)
{
	CParticleRenderer::PostSimulate(tDelta);
	UpdateNodes();
}

void CParticleRendererBeam::UpdateNodes()
{
	auto *p = GetParticleSystem().GetParticle(0);
	auto *color = (p != nullptr) ? &p->GetColor() : nullptr;
	for(auto i=(m_startNode -1);i!=m_endNode;++i)
	{
		auto pos = GetParticleSystem().GetNodePosition(i +1);
		m_nodeOrigins[i].origin = pos;
		if(color != nullptr)
			m_nodeOrigins[i].color = {color->r /255.f,color->g /255.f,color->b /255.f,color->a /255.f};
	}
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_vertexBuffer,0ull,m_vertexBuffer->GetSize(),m_nodeOrigins.data());
}

std::pair<Vector3,Vector3> CParticleRendererBeam::GetRenderBounds() const
{
	if((m_startNode -1) == m_endNode)
		return {{},{}};
	auto radius = GetParticleSystem().GetRadius();
	const auto minSize = Vector3{-radius,-radius,-radius};
	const auto maxSize = Vector3{radius,radius,radius};
	std::pair<Vector3,Vector3> bounds {};
	for(auto i=(m_startNode -1);i!=m_endNode;++i)
	{
		auto pos = GetParticleSystem().GetNodePosition(i +1);
		uvec::to_min_max(bounds.first,bounds.second,pos +minSize,pos +maxSize);
	}
	return bounds;
}

void CParticleRendererBeam::Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::ParticleRenderFlags renderFlags)
{
	if(m_shader.expired())
		return;
	auto *shader = static_cast<pragma::ShaderParticlePolyboard*>(m_shader.get());
	if(shader == nullptr || shader->BeginDraw(drawCmd,GetParticleSystem(),renderFlags) == false)
		return;
	auto &descSetLightSources = *renderer.GetForwardPlusInstance().GetDescriptorSetGraphics();
	auto &descSetShadows = *renderer.GetCSMDescriptorSet();
	shader->BindLights(descSetShadows,descSetLightSources);
	shader->BindSceneCamera(renderer,(GetParticleSystem().GetRenderMode() == RenderMode::View) ? true : false);
	shader->BindRenderSettings(c_game->GetGlobalRenderSettingsDescriptorSet());
	shader->Draw(renderer,*m_particleSystem,*m_vertexBuffer,*m_indexBuffer,m_indexCount,GetParticleSystem().GetRadius(),m_curvature); // TODO: bloom
	shader->EndDraw();
}


void CParticleRendererBeam::RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId)
{
	/*static auto hShader = c_engine->GetShader("particlepolyboardshadow");
	if(!hShader.IsValid())
		return;
	auto &shader = static_cast<Shader::ParticlePolyboardShadow&>(*hShader.get());
	if(shader.BeginDraw() == false)
		return;
	//auto &cam = *c_game->GetRenderCamera();
	shader.Draw(m_particleSystem,m_vertexBuffer,m_indexBuffer,m_indexCount,m_particleSystem->GetRadius(),m_curvature,light,layerId);
	shader.EndDraw();*/ // prosper TODO
}

void CParticleRendererBeam::OnParticleSystemStopped()
{
	CParticleRenderer::OnParticleSystemStopped();

}
