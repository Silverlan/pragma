// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.rasterization_renderer;
import :engine;
import :game;
import :rendering.shaders;

using namespace pragma::rendering;

static auto cvDrawParticles = pragma::console::get_client_con_var("render_draw_particles");
void pragma::CRasterizationRendererComponent::RenderParticles(prosper::ICommandBuffer &cmd, const DrawSceneInfo &drawSceneInfo, bool depthPass, prosper::IPrimaryCommandBuffer *primCmdBuffer)
{
	assert(!depthPass || primCmdBuffer != nullptr);
	// TODO: Only render particles if they're visible
	auto &culledParticles = m_culledParticles;
	culledParticles.clear();
	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<ecs::CParticleSystemComponent>>();
	culledParticles.reserve(entIt.GetCount());
	for(auto *ent : entIt) {
		auto *ptrC = ent->GetComponent<ecs::CParticleSystemComponent>().get();
		auto &renderers = ptrC->GetRenderers();
		auto *renderer = !renderers.empty() ? renderers.front().get() : nullptr;
		if(!renderer)
			continue;
		if(depthPass && !renderer->RequiresDepthPass())
			continue;
		culledParticles.push_back(ent->GetComponent<ecs::CParticleSystemComponent>().get());
	}
	auto bShouldDrawParticles = (drawSceneInfo.renderFlags & RenderFlags::Particles) == RenderFlags::Particles && cvDrawParticles->GetBool() == true && culledParticles.empty() == false;
	if(!bShouldDrawParticles)
		return;
	get_cgame()->StartGPUProfilingStage("Particles");
	///InvokeEventCallbacks(EVENT_MT_BEGIN_RECORD_PARTICLES,evDataLightingStage);

	auto &hdrInfo = GetHDRInfo();
	auto &prepass = hdrInfo.prepass;
	auto texDepth = prepass.textureDepth;

	auto &rt = hdrInfo.GetRenderTarget(drawSceneInfo);
	auto &hdrTex = rt.GetTexture();

	if(!depthPass) {
		if(drawSceneInfo.renderTarget) {
			// Custom render target; Blit to staging target
			auto &imgColorRender = drawSceneInfo.renderTarget->GetTexture().GetImage();
			auto &imgColorSampled = prepass.textureDepthSampled->GetImage();
			cmd.RecordImageBarrier(imgColorSampled, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
			cmd.RecordImageBarrier(imgColorRender, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
			cmd.RecordBlitImage({}, imgColorRender, imgColorSampled);
			cmd.RecordImageBarrier(imgColorSampled, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		}

		// We need to sample depth buffer, so we need to blit
		auto &imgDepthRender = prepass.textureDepth->GetImage();
		auto &imgDepthSampled = prepass.textureDepthSampled->GetImage();
		cmd.RecordImageBarrier(imgDepthSampled, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
		cmd.RecordImageBarrier(imgDepthRender, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
		cmd.RecordBlitImage({}, imgDepthRender, imgDepthSampled);
		cmd.RecordImageBarrier(imgDepthSampled, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		cmd.RecordImageBarrier(imgDepthRender, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal);

		// cmd.RecordImageBarrier(texDepth->GetImage(),prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
		if(primCmdBuffer->RecordBeginRenderPass(*hdrInfo.rtParticle) == true) {
			RecordRenderParticleSystems(cmd, drawSceneInfo, culledParticles, SceneRenderPass::World, depthPass, false);
			primCmdBuffer->RecordEndRenderPass();
		}
		// cmd.RecordImageBarrier(texDepth->GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal);

		if(drawSceneInfo.renderTarget) {
			// Custom render target; Blit from staging target
			auto &imgColorRender = drawSceneInfo.renderTarget->GetTexture().GetImage();
			auto &imgColorSampled = prepass.textureDepthSampled->GetImage();
			cmd.RecordImageBarrier(imgColorSampled, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferSrcOptimal);
			cmd.RecordImageBarrier(imgColorRender, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::TransferDstOptimal);
			cmd.RecordBlitImage({}, imgColorRender, imgColorSampled);
			cmd.RecordImageBarrier(imgColorSampled, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
			cmd.RecordImageBarrier(imgColorRender, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		}
	}
	else {
		RecordRenderParticleSystems(cmd, drawSceneInfo, culledParticles, SceneRenderPass::World, depthPass, false);
	}

	//InvokeEventCallbacks(EVENT_MT_END_RECORD_PARTICLES,evDataLightingStage);
	get_cgame()->StopGPUProfilingStage(); // Particles
}
