/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/c_optical_camera_component.hpp"
#include "pragma/entities/components/renderers/rasterization/hdr_data.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_dof.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/game/c_game.h"
#include <pragma/console/convars.h>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

#include "pragma/console/c_cvar.h"
#include <pragma/entities/entity_iterator.hpp>
#include "pragma/entities/environment/effects/c_env_particle_system.h"

static auto cvDrawParticles = GetClientConVar("render_draw_particles");
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include <prosper_descriptor_set_group.hpp>
void pragma::CRasterizationRendererComponent::RenderParticles(prosper::ICommandBuffer &cmd, const util::DrawSceneInfo &drawSceneInfo, bool depthPass, prosper::IPrimaryCommandBuffer *primCmdBuffer)
{
	assert(!depthPass || primCmdBuffer != nullptr);
	// TODO: Only render particles if they're visible
	auto &culledParticles = m_culledParticles;
	culledParticles.clear();
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	culledParticles.reserve(entIt.GetCount());
	for(auto *ent : entIt) {
		auto *ptrC = ent->GetComponent<pragma::CParticleSystemComponent>().get();
		auto &renderers = ptrC->GetRenderers();
		auto *renderer = !renderers.empty() ? renderers.front().get() : nullptr;
		if(!renderer)
			continue;
		if(depthPass && !renderer->RequiresDepthPass())
			continue;
		culledParticles.push_back(ent->GetComponent<pragma::CParticleSystemComponent>().get());
	}
	auto bShouldDrawParticles = (drawSceneInfo.renderFlags & RenderFlags::Particles) == RenderFlags::Particles && cvDrawParticles->GetBool() == true && culledParticles.empty() == false;
	if(!bShouldDrawParticles)
		return;
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::Particles);
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
			RecordRenderParticleSystems(cmd, drawSceneInfo, culledParticles, pragma::rendering::SceneRenderPass::World, depthPass, false);
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
		RecordRenderParticleSystems(cmd, drawSceneInfo, culledParticles, pragma::rendering::SceneRenderPass::World, depthPass, false);
	}

	//InvokeEventCallbacks(EVENT_MT_END_RECORD_PARTICLES,evDataLightingStage);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::Particles);
}
