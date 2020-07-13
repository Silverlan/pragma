/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/renderers/c_particle_mod_blob.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_blob.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_blob_shadow.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/widebugdepthtexture.h"
#include <wgui/types/wirect.h>

REGISTER_PARTICLE_RENDERER(blob,CParticleRendererBlob);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

decltype(CParticleRendererBlob::s_bShowNeighborLinks) CParticleRendererBlob::s_bShowNeighborLinks = false;
//decltype(CParticleRendererBlob::s_dsParticles) CParticleRendererBlob::s_dsParticles = nullptr; // prosper TODO
decltype(CParticleRendererBlob::s_activeBlobRendererCount) CParticleRendererBlob::s_activeBlobRendererCount = 0;
//decltype(CParticleRendererBlob::s_shader) CParticleRendererBlob::s_shader = nullptr; // prosper TODO
decltype(CParticleRendererBlob::s_shadowShader) CParticleRendererBlob::s_shadowShader = nullptr;

void CParticleRendererBlob::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleRenderer::Initialize(pSystem,values);
	auto bHasShininess = false;
	auto shininess = 0.f;
	for(auto it=values.begin();it!=values.end();it++)
	{
		std::string key = it->first;
		ustring::to_lower(key);
		if(key == "specular_color")
			m_specularColor = Color(it->second).ToVector4();
		else if(key == "specular_shininess")
		{
			shininess = util::to_float(it->second);
			bHasShininess = true;
		}
		else if(key == "reflection_intensity")
			m_reflectionIntensity = util::to_float(it->second);
		else if(key == "refraction_index_ratio")
			m_refractionIndexRatio = util::to_float(it->second);
		//else if(key == "debug_mode") // prosper TODO
		//	m_debugMode = static_cast<Shader::ParticleBlob::DebugMode>(util::to_int(it->second)); // prosper TODO
	}
	if(bHasShininess == true)
		m_specularColor.w = shininess;
}

CParticleRendererBlob::~CParticleRendererBlob()
{
	ShowDebugNeighborLinks(false);
}

pragma::ShaderParticleBase *CParticleRendererBlob::GetShader() const
{
	return nullptr; // TODO
}

void CParticleRendererBlob::ShowDebugNeighborLinks(bool b)
{
	if(b == false)
	{
		m_dbgNeighborLinks.clear();
		return;
	}
	m_dbgNeighborLinks.resize(GetParticleSystem().GetMaxParticleCount());
	for(auto &info : m_dbgNeighborLinks)
	{
		//for(auto &o : info.renderObjects) // prosper TODO
		//	o = DebugRenderer::DrawLine({},{},Color::Lime); // prosper TODO
	}
}

void CParticleRendererBlob::UpdateDebugNeighborLinks()
{
	 // prosper TODO
#if 0
	if(s_bShowNeighborLinks == false)
	{
		if(!m_dbgNeighborLinks.empty())
			ShowDebugNeighborLinks(false);
		return;
	}
	else if(m_dbgNeighborLinks.empty())
		ShowDebugNeighborLinks(true);

	for(auto &info : m_dbgNeighborLinks)
		info.hide = true;

	auto &particles = m_particleSystem->GetRenderParticleData();
	for(auto it=particles.begin();it!=particles.end();++it)
	{
		auto renderIdx0 = it -particles.begin();
		auto particleIdx0 = m_particleSystem->TranslateBufferIndex(renderIdx0);
		auto &cLinks0 = m_particleLinks[particleIdx0];
		auto &p0 = *it;
		auto &dbgObjs0 = m_dbgNeighborLinks[particleIdx0];
		dbgObjs0.hide = false;
		for(auto i=decltype(cLinks0.links.size()){1};i<cLinks0.links.size();++i)
		{
			auto particleIdx1 = cLinks0.links[i].targetParticleIdx;
			auto &o = static_cast<DebugRenderer::WorldObject&>(*dbgObjs0.renderObjects[i -1].get());
			auto &verts = o.GetVertices();
			assert(verts.size() == 2);
			std::pair<Vector3,Vector3> newVerts;
			if(particleIdx1 == INVALID_BLOB_INDEX)
			{
				newVerts.first = {};
				newVerts.second = {};
			}
			else
			{
				auto &p1 = particles[m_particleSystem->TranslateParticleIndex(particleIdx1)];
				newVerts.first = *reinterpret_cast<const Vector3*>(&p0.position);
				newVerts.second = *reinterpret_cast<const Vector3*>(&p1.position);
				auto col = Color::Lime;
				auto &cLinks1 = m_particleLinks[particleIdx1];
				auto it = std::find_if(cLinks1.links.begin(),cLinks1.links.end(),[particleIdx0](const Link &link) {
					return (link.targetParticleIdx == particleIdx0) ? true : false;
				});
				if(it == cLinks1.links.end())
					col = Color::Red; // Link is one-way only (Which shouldn't happen), color it red

				// Update color
				auto &colOld = o.GetColor();
				auto colNew = col.ToVector4();
				if(uvec::cmp(colOld,colNew) == false) // Only update instance buffer if color has changed
					o.SetColor(colNew);
				//
			}
			auto bUpdateBuffer = (uvec::cmp(verts.front(),newVerts.first) == false || uvec::cmp(verts.back(),newVerts.second) == false) ? true : false;
			verts.front() = newVerts.first;
			verts.back() = newVerts.second;
			if(bUpdateBuffer == true)
				o.UpdateVertexBuffer();
		}
	}
	for(auto &info : m_dbgNeighborLinks)
	{
		if(info.hide == false)
			continue;
		for(auto &ptrO : info.renderObjects)
		{
			auto &o = static_cast<DebugRenderer::WorldObject&>(*ptrO.get());
			auto &verts = o.GetVertices();
			assert(verts.size() == 2);
			if(uvec::cmp(verts.front(),uvec::ORIGIN) == false || uvec::cmp(verts.back(),uvec::ORIGIN) == false)
			{
				verts.front() = {};
				verts.back() = {};
				o.UpdateVertexBuffer();
			}
		}
	}
#endif
}

void CParticleRendererBlob::SetShowNeighborLinks(bool b) {s_bShowNeighborLinks = b;}

void CParticleRendererBlob::OnParticleSystemStarted()
{
	 // prosper TODO
#if 0
	auto &context = c_engine->GetRenderContext();
	if(s_activeBlobRendererCount++ == 0)
	{
		auto hShader = c_engine->GetShader("particleblob");
		if(hShader.IsValid())
		{
			// Generate a descriptor set for our particle storage buffer
			auto *shader = s_shader = static_cast<Shader::ParticleBlob*>(hShader.get());
			if(shader->GenerateDescriptorSet(shader->GetDescSet(Shader::ParticleBlob::DescSet::ParticleData),s_dsParticles) == true)
				s_dsParticles->Update(CParticleSystem::GetManagedParticleBuffer());
		}

		hShader = c_engine->GetShader("particleblobshadow");
		if(hShader.IsValid())
			s_shadowShader = static_cast<Shader::ParticleBlobShadow*>(hShader.get());
	}
	auto maxParticleCount = m_particleSystem->GetMaxParticleCount();
	assert(maxParticleCount < (INVALID_BLOB_INDEX +1));
	m_adjacentParticleIds.resize(maxParticleCount);
	m_adjacentBlobBuffer = Vulkan::SwapBuffer::Create(context,prosper::BufferUsageFlags::StorageBufferBit,m_adjacentParticleIds.size() *sizeof(decltype(m_adjacentParticleIds.front().front())) *m_adjacentParticleIds.front().size(),m_adjacentParticleIds.data());

	m_particleLinks.resize(maxParticleCount);
	for(auto it=m_particleLinks.begin();it!=m_particleLinks.end();++it)
	{
		auto &cLinks = *it;
		auto &firstLink = cLinks.links.front();
		firstLink.targetParticleIdx = it -m_particleLinks.begin();
		firstLink.distSqr = 0.f;
	}
#if ENABLE_BLOB_DEPTH_TEST == 1
	// Incomplete; For testing purposes only!
	auto &scene = c_game->GetRenderScene();
	auto &sceneDepth = scene->GetDepthTexture();
	auto &sceneTex = scene->GetRenderTexture();
	auto samples = sceneDepth->GetSampleCount();

	auto depthImage = Vulkan::Image::Create(context,sceneDepth->GetWidth(),sceneDepth->GetHeight(),sceneDepth->GetFormat(),std::function<void(vk::ImageCreateInfo&,vk::MemoryPropertyFlags&)>([samples](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
		info.usage = prosper::ImageUsageFlags::DepthStencilAttachmentBit | prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferSrcBit;
		//info.samples = samples;
	}));
	auto depthTex = Vulkan::Texture::Create(context,depthImage);//,Vulkan::Sampler(nullptr));
	auto renderTex = Vulkan::Texture::Create(context,sceneTex->GetWidth(),sceneTex->GetHeight(),prosper::Format::R8G8B8A8_UNorm/*sceneTex->GetFormat()*/,false,false,[](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags &flags) {
		info.setUsage(prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit);
	}); // TODO: Use sceneTex instead; TODO: Sample Count
	auto rp = context->GenerateRenderPass({
		{renderTex->GetFormat(),renderTex->GetSampleCount(),true}, // TODO: Bloom
		{depthTex->GetFormat(),depthTex->GetSampleCount(),true}
	});
	auto fb = Vulkan::Framebuffer::Create(context,rp,depthTex->GetWidth(),depthTex->GetHeight(),{renderTex->GetImageView(),depthTex->GetImageView()});
	m_rtTransparent = Vulkan::RenderTarget::Create(renderTex,depthTex,fb);
	//m_rtTransparent = Vulkan::RenderTarget::Create(sceneTex,depthTex,fb);
	c_game->AddCallback("DrawScene",FunctionCallback<bool,std::reference_wrapper<const Vulkan::RenderPass>,std::reference_wrapper<const Vulkan::Framebuffer>,std::reference_wrapper<const Vulkan::CommandBuffer>>::Create([this,depthImage,renderTex](std::reference_wrapper<const Vulkan::RenderPass> &rpScene,std::reference_wrapper<const Vulkan::Framebuffer> &fbScene,std::reference_wrapper<const Vulkan::CommandBuffer> &drawCmd) {
		renderTex->GetImage()->SetDrawLayout(prosper::ImageLayout::ColorAttachmentOptimal);
		depthImage->SetDrawLayout(prosper::ImageLayout::DepthStencilAttachmentOptimal); // TODO: Only once?
		//drawCmd.get()->BeginRenderPass(m_rtTransparent,Color::Red,1.f);
		drawCmd.get()->BeginRenderPass(m_rtTransparent->GetRenderPass(),m_rtTransparent->GetFramebuffer(),renderTex->GetWidth(),renderTex->GetHeight(),{
			vk::ClearValue{vk::ClearColorValue{std::array<float,4>{1.f,0.f,0.f,0.f}}}, // Color Attachment
			vk::ClearValue{vk::ClearDepthStencilValue{1.f,0}} // Depth Attachment
		});
			m_particleSystem->Render(false); // TODO Bloom?
		drawCmd.get()->EndRenderPass();
		
		depthImage->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal);
		renderTex->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal);
		/*tex->GetImage()->SetDrawLayout(prosper::ImageLayout::ColorAttachmentOptimal);
		drawCmd->BeginRenderPass(glowInfo.renderPass,glowInfo.framebuffer,tex->GetWidth(),tex->GetHeight(),{
			vk::ClearValue{vk::ClearColorValue{std::array<float,4>{1.f,0.f,0.f,1.f}}},
			vk::ClearValue{vk::ClearColorValue{}}
		});*/
		return false;
	}));
	static auto dbg = std::make_unique<DebugGameGUI>([renderTex]() {
		auto *b = WGUI::Create<WIRect>();
		b->SetColor(Color::White);
		b->SetSize(512,512);

		auto *r = WGUI::Create<WITexturedRect>(b);
		r->SetSize(512,512);
		r->SetTexture(renderTex);
		return b->GetHandle();//r->GetHandle();
	});
	static auto dbg2 = std::make_unique<DebugGameGUI>([depthTex]() {
		auto r = WGUI::Create<WIDebugDepthTexture>();
		r->SetSize(512,512);
		r->SetX(512);
		r->SetTexture(depthTex);
		auto &scene = c_game->GetRenderScene();
		r->Update(scene->GetZNear(),scene->GetZFar());
		return r->GetHandle();
	});
	auto *d = dbg2.get();
	dbg2->AddCallback("PostRenderScene",FunctionCallback<>::Create([d]() {
		auto *el = d->GetGUIElement();
		if(el == nullptr)
			return;
		auto &scene = c_game->GetRenderScene();
		static_cast<WIDebugDepthTexture*>(el)->Update(scene->GetZNear(),scene->GetZFar());
	}));
#endif
#endif
}

REGISTER_CONVAR_CALLBACK_CL(debug_particle_blob_show_neighbor_links,[](NetworkState *state,ConVar*,bool,bool val) {
	CHECK_CHEATS("debug_particle_blob_show_neighbor_links",state,);
	if(c_game == nullptr)
		return;
	CParticleRendererBlob::SetShowNeighborLinks(val);
});


void CParticleRendererBlob::OnParticleSystemStopped()
{
	/*if(--s_activeBlobRendererCount == 0) // Release the descriptor set when it's not needed anymore
	{
		auto &context = c_engine->GetRenderContext();
		const_cast<Vulkan::Context&>(context).KeepAlive(s_dsParticles); // Keep descriptor set alive until it's definitely not in use anymore
		s_dsParticles = nullptr;
	}
	m_adjacentParticleIds.clear();
	m_adjacentBlobBuffer = nullptr;*/ // prosper TODO
}

void CParticleRendererBlob::SortParticleLinks()
{
	 // prosper TODO
	/*
	for(auto &cLinks : m_particleLinks)
	{
		auto &links = cLinks.links;
		auto itBegin = links.begin() +1;

		//std::sort(links.begin() +1,links.begin() +cLinks.nextLinkId -1,[](const Link &l0,const Link &l1) {
		std::sort(links.begin() +1,links.end(),[](const Link &l0,const Link &l1) {
			return (l0.distSqr < l1.distSqr) ? true : false;
		});

#ifdef _DEBUG
		// Make sure sorting has worked correctly
		auto bReachedInvalid = false;
		for(auto &link : links)
		{
			if(bReachedInvalid == false)
			{
				if(link.targetParticleIdx == INVALID_BLOB_INDEX)
					bReachedInvalid = true;
			}
			else
				assert(link.targetParticleIdx == INVALID_BLOB_INDEX);
		}
#endif
	}*/
}

void CParticleRendererBlob::OnParticleDestroyed(CParticle &particle)
{
	/*CParticleRenderer::Destroy(particle);
	auto particleIdx = particle.GetIndex();
	for(auto it=m_particleLinks.begin();it!=m_particleLinks.end();++it)
	{
		auto idx = it -m_particleLinks.begin();
		auto &cLinks = *it;
		for(auto it=cLinks.links.begin() +1;it<cLinks.links.end();++it)
		{
			auto &link = *it;
			if(idx == particleIdx || link.targetParticleIdx == particleIdx) // Remove all links of the destroyed particle, as well as all links referencing the destroyed particle
			{
				if(link.targetParticleIdx != INVALID_BLOB_INDEX)
					--cLinks.nextLinkId;
				link = {};
			}
		}
	}
	SortParticleLinks();*/ // prosper TODO
}
/*
void CParticleRendererBlob::UpdateAdjacentParticles(const Vulkan::Buffer &blobIndexBuffer)
{
	auto frameId = c_engine->GetRenderContext().GetLastFrameId();
	if(m_lastFrame == frameId)
		return;
	m_lastFrame = frameId;

	auto &particles = m_particleSystem->GetRenderParticleData();
	auto numParticles = m_particleSystem->GetRenderParticleCount();

	auto itBegin = particles.begin();
	auto itEnd = itBegin +numParticles;

	// Update existing links (Remove invalid links)
	for(auto it=itBegin;it!=itEnd;++it)
	{
		auto renderIdx0 = it -itBegin;
		auto particleIdx0 = m_particleSystem->TranslateBufferIndex(renderIdx0);
		assert(particleIdx0 != std::numeric_limits<decltype(particleIdx0)>::max());
		if(particleIdx0 == std::numeric_limits<decltype(particleIdx0)>::max())
			continue;
		auto &cLinks0 = m_particleLinks[particleIdx0];
		auto &links0 = cLinks0.links;

		auto &p0 = *it;
		auto &pos0 = *reinterpret_cast<const Vector3*>(&p0.position);
		auto radius0 = p0.position.w;
		for(auto itLink0=links0.begin() +1;itLink0!=links0.end();++itLink0)
		{
			auto &link = *itLink0;
			if(link.targetParticleIdx == INVALID_BLOB_INDEX)
				continue;
			auto particleIdx1 = link.targetParticleIdx;
			auto bClearLink = false;
			auto renderIdx1 = m_particleSystem->TranslateParticleIndex(particleIdx1);
			if(renderIdx1 == std::numeric_limits<decltype(renderIdx1)>::max())
				bClearLink = true;
			else
			{
				auto &cParticle1 = *m_particleSystem->GetParticle(particleIdx1);
				bClearLink = !cParticle1.IsAlive();
				if(bClearLink == false)
				{
					auto &p1 = particles[renderIdx1];
					auto &pos1 = *reinterpret_cast<const Vector3*>(&p1.position);
					auto radius1 = p1.position.w;
					auto maxDistSqr = umath::pow2(radius0 +radius1);
					auto distSqr = link.distSqr = uvec::length_sqr(pos1 -pos0);
					bClearLink = (distSqr >= maxDistSqr) ? true : false;
				}
			}
			if(bClearLink == true)
			{
				// Clear neighbor link
				auto &cLinks1 = m_particleLinks[particleIdx1];
				auto &links1 = cLinks1.links;
				auto itLink1 = std::find_if(links1.begin(),links1.end(),[particleIdx0](const Link &link) {
					return (link.targetParticleIdx == particleIdx0) ? true : false;
				});
				if(itLink1 != links1.end())
				{
					--cLinks1.nextLinkId;
					*itLink1 = {};
				}
				//
				--cLinks0.nextLinkId;
				link = {}; // Clear own link
			}
		}
	}

	//

	// Sort links by distance
	SortParticleLinks();
	
	const auto fAddLink = [](LinkContainer &cLinks,uint32_t particleIdx,float distSqr) {
		assert(cLinks.nextLinkId < Shader::ParticleBlob::MAX_BLOB_NEIGHBORS +1);
		auto it = std::find_if(cLinks.links.begin(),cLinks.links.end(),[distSqr](const Link &link) {
			return (distSqr < link.distSqr) ? true : false;
		});
		assert(it != cLinks.links.end());
		for(auto i=cLinks.links.size() -1;i>(it -cLinks.links.begin());--i)
			cLinks.links[i] = cLinks.links[i -1];
		it->distSqr = distSqr;
		it->targetParticleIdx = particleIdx;
		++cLinks.nextLinkId;
	};

	// Find new links
	for(auto it0=itBegin;it0!=itEnd;++it0)
	{
		auto renderIdx0 = it0 -itBegin;
		auto particleIdx0 = m_particleSystem->TranslateBufferIndex(renderIdx0);
		auto &cLinks0 = m_particleLinks[particleIdx0];
		auto &links0 = cLinks0.links;
		if(links0.back().targetParticleIdx == INVALID_BLOB_INDEX) // Only check for neighbors if link slots are available
		{
			auto &p0 = *it0;
			auto &pos0 = *reinterpret_cast<const Vector3*>(&p0.position);
			auto radius0 = p0.position.w;
			for(auto it1=it0 +1;it1!=itEnd;++it1)
			{
				auto renderIdx1 = it1 -itBegin;
				auto particleIdx1 = m_particleSystem->TranslateBufferIndex(renderIdx1);
				auto &cLinks1 = m_particleLinks[particleIdx1];
				auto &links1 = cLinks1.links;
				if(links1.back().targetParticleIdx != INVALID_BLOB_INDEX)
					continue; // No free slots available, skip this particle
				auto &p1 = *it1;
				auto &pos1 = *reinterpret_cast<const Vector3*>(&p1.position);
				auto radius1 = p1.position.w;
				auto maxDistSqr = umath::pow2(radius0 +radius1);
				auto distSqr = uvec::length_sqr(pos1 -pos0);
				if(distSqr < maxDistSqr)
				{
					// Check if link already exists
					auto itEnd = cLinks0.links.begin() +cLinks0.nextLinkId;
					auto it = std::find_if(cLinks0.links.begin(),itEnd,[particleIdx1](const Link &link) {
						return (link.targetParticleIdx == particleIdx1) ? true : false;
					});
					if(it != itEnd)
						continue;
					//

					fAddLink(cLinks0,particleIdx1,distSqr);
					fAddLink(cLinks1,particleIdx0,distSqr);
					if(links0.back().targetParticleIdx != INVALID_BLOB_INDEX)
						break;
				}
			}
		}
		auto &particleIds = m_adjacentParticleIds[renderIdx0];
		std::fill(particleIds.begin() +cLinks0.nextLinkId,particleIds.end(),INVALID_BLOB_INDEX);
		for(auto i=decltype(cLinks0.nextLinkId){0};i<=cLinks0.nextLinkId;++i)
			particleIds[i] = m_particleSystem->TranslateParticleIndex(cLinks0.links[i].targetParticleIdx);
	}
	//

	// Update buffer
	for(auto it=particles.begin();it!=itEnd;++it)
	{
		auto renderIdx = it -particles.begin();
		auto &particleIds = m_adjacentParticleIds[renderIdx];
		auto offset = renderIdx *Shader::ParticleBlob::MAX_BLOB_NEIGHBORS *sizeof(particleIds.front());
		blobIndexBuffer->MapMemory(offset,particleIds.size() *sizeof(particleIds.front()),particleIds.data(),true); // Map and flush the memory range
	}
	//
	UpdateDebugNeighborLinks();
}
*/ // prosper TODO
void CParticleRendererBlob::Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::ParticleRenderFlags renderFlags)
{
	/*if(s_dsParticles == nullptr || m_adjacentBlobBuffer == nullptr)
		return;
	auto *buf = m_particleSystem->GetParticleBuffer();
	if(buf == nullptr)
		return;
	auto &context = c_engine->GetRenderContext();
	auto swapIdx = context.GetFrameSwapIndex();
	if(s_shader->BeginDraw(context.GetDrawCmd(),s_dsParticles,(*buf)->GetBaseOffset(),m_specularColor,m_reflectionIntensity,m_refractionIndexRatio,m_debugMode) == false)
		return;
	auto &blobIndexBuffer = *m_adjacentBlobBuffer->GetBuffer(swapIdx);
	UpdateAdjacentParticles(blobIndexBuffer);
	auto &cam = *c_game->GetRenderCamera();
	auto &scene = c_game->GetRenderScene();
	auto &descSetLightSources = scene->GetForwardPlusInstance().GetDescriptorSetGraphics();
	auto &descSetShadows = scene->GetLightShadowMapDescriptorSet();
	s_shader->BindLights(descSetShadows,descSetLightSources);
	s_shader->BindScene(*c_game->GetRenderScene(),(m_particleSystem->GetRenderMode() == RenderMode::View) ? true : false);
	s_shader->Draw(cam,m_particleSystem,bloom,blobIndexBuffer);
	s_shader->EndDraw();*/ // prosper TODO
}

void CParticleRendererBlob::RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId)
{
	/*auto &shader = *s_shadowShader;
	auto &context = c_engine->GetRenderContext();
	auto swapIdx = context.GetFrameSwapIndex();
	auto &blobIndexBuffer = *m_adjacentBlobBuffer->GetBuffer(swapIdx);
	UpdateAdjacentParticles(blobIndexBuffer);
	auto *buf = m_particleSystem->GetParticleBuffer();
	if(buf == nullptr)
		return;
	if(shader.BeginDraw(context.GetDrawCmd(),s_dsParticles,(*buf)->GetBaseOffset()) == false)
		return;
	shader.Draw(m_particleSystem,blobIndexBuffer,light,layerId);
	shader.EndDraw();*/ // prosper TODO
}
