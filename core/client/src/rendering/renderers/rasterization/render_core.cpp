/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/renderers/rasterization/glow_data.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/c_settings.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <sharedutils/util_shaderinfo.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma::rendering;

void RasterizationRenderer::RenderParticleSystems(const util::DrawSceneInfo &drawSceneInfo,std::vector<pragma::CParticleSystemComponent*> &particles,RenderMode renderMode,Bool bloom,std::vector<pragma::CParticleSystemComponent*> *bloomParticles)
{
	auto depthOnly = umath::is_flag_set(drawSceneInfo.renderFlags,FRender::ParticleDepth);
	if(depthOnly && bloom)
		return;
	auto renderFlags = ParticleRenderFlags::None;
	umath::set_flag(renderFlags,ParticleRenderFlags::DepthOnly,depthOnly);
	umath::set_flag(renderFlags,ParticleRenderFlags::Bloom,bloom);
	auto bFirst = true;
	auto &drawCmd = drawSceneInfo.commandBuffer;
	for(auto *particle : particles)
	{
		if(particle != nullptr && particle->IsActive() == true && particle->GetRenderMode() == renderMode && particle->GetParent() == nullptr)
		{
			if(bFirst == true)
			{
				bFirst = false;

				// We need to end the current render pass, because we need the depth buffer with everything
				// that has been rendered thus far.
				EndRenderPass(drawSceneInfo);

				auto &hdrInfo = GetHDRInfo();
				auto &prepass = GetPrepass();
				if(prepass.textureDepth->IsMSAATexture())
				{
					auto &msaaTex = static_cast<prosper::MSAATexture&>(*prepass.textureDepth);
					msaaTex.Resolve(
						*drawCmd,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal,
						prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal
					); // Particles aren't multisampled, but requires scene depth buffer
					msaaTex.Reset(); // Depth buffer isn't complete yet; We need to reset, otherwise the next resolve will not update it properly
				}
				//else
				//	.RecordImageBarrier(**drawCmd,**prepass.textureDepth->GetImage(),prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);

				// Restart render pass
				BeginRenderPass(drawSceneInfo,hdrInfo.rpPostParticle.get());
			}
			//scene->ResolveDepthTexture(drawCmd); // Particles aren't multisampled, but requires scene depth buffer
			particle->Render(drawCmd,*this,renderFlags);
			if(bloomParticles != nullptr)
			{
				if(particle->IsBloomEnabled())
					bloomParticles->push_back(particle);
				auto &children = particle->GetChildren();
				bloomParticles->reserve(bloomParticles->size() +children.size());
				for(auto &hChild : children)
				{
					if(hChild.child.expired())
						continue;
					bloomParticles->push_back(hChild.child.get());
				}
			}
		}
	}
}

bool RasterizationRenderer::RenderScene(const util::DrawSceneInfo &drawSceneInfo)
{
	if(BaseRenderer::RenderScene(drawSceneInfo) == false)
		return false;
	RenderGameScene(drawSceneInfo);
	return true;
}
