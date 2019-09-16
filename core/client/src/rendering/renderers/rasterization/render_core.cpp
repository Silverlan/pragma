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

void RasterizationRenderer::RenderParticleSystems(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::vector<pragma::CParticleSystemComponent*> &particles,RenderMode renderMode,Bool bloom,std::vector<pragma::CParticleSystemComponent*> *bloomParticles)
{
	auto bFirst = true;
	for(auto *particle : particles)
	{
		if(particle != nullptr && particle->IsActive() == true && particle->GetRenderMode() == renderMode)
		{
			if(bFirst == true)
			{
				bFirst = false;

				// We need to end the current render pass, because we need the depth buffer with everything
				// that has been rendered thus far.
				EndRenderPass(drawCmd);

				auto &hdrInfo = GetHDRInfo();
				auto &prepass = GetPrepass();
				if(prepass.textureDepth->IsMSAATexture())
				{
					auto &msaaTex = static_cast<prosper::MSAATexture&>(*prepass.textureDepth);
					msaaTex.Resolve(
						**drawCmd,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
						Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
					); // Particles aren't multisampled, but requires scene depth buffer
					msaaTex.Reset(); // Depth buffer isn't complete yet; We need to reset, otherwise the next resolve will not update it properly
				}
				//else
				//	prosper::util::record_image_barrier(**drawCmd,**prepass.textureDepth->GetImage(),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

				// Restart render pass
				BeginRenderPass(drawCmd,hdrInfo.rpPostParticle.get());
			}
			//scene->ResolveDepthTexture(drawCmd); // Particles aren't multisampled, but requires scene depth buffer
			particle->Render(drawCmd,*this,bloom);
			if(bloomParticles != nullptr)
			{
				if(particle->GetBloomScale() != 0.f)
					bloomParticles->push_back(particle);
				auto &children = particle->GetChildren();
				bloomParticles->reserve(bloomParticles->size() +children.size());
				for(auto &hChild : children)
				{
					if(hChild.expired())
						continue;
					bloomParticles->push_back(hChild.get());
				}
			}
		}
	}
}

// TODO: Remove this
#if 0
void RasterizationRenderer::RenderScenePostProcessing(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags)
{


	auto &hdrInfo = GetHDRInfo();
	// TODO: Obsolete?
	/*if(static_cast<AntiAliasing>(cvAntiAliasing->GetInt()) == AntiAliasing::FXAA)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingFXAA);
		// HDR needs to be resolved before FXAA is applied
		// Note: This will be undone by FXAA shader! (because HDR is required for post-processing)
		auto &dsgBloomTonemapping = hdrInfo.dsgBloomTonemapping;
		RenderSceneResolveHDR(drawCmd,*(*dsgBloomTonemapping)->get_descriptor_set(0u),true);

		auto &hdrInfo = GetHDRInfo();
		auto whShaderPPFXAA = c_game->GetGameShader(CGame::GameShader::PPFXAA);
		TODO: Remove this fxaa invocation!!
		if(whShaderPPFXAA.valid() == true)
		{
			auto &shaderFXAA = static_cast<pragma::ShaderPPFXAA&>(*whShaderPPFXAA.get());
			auto &prepass = hdrInfo.prepass;

			auto &hdrTex = hdrInfo.toneMappedRenderTarget->GetTexture();
			prosper::util::record_image_barrier(*(*drawCmd),*(*hdrTex->GetImage()),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			if(prosper::util::record_begin_render_pass(*(*drawCmd),*hdrInfo.hdrPostProcessingRenderTarget) == true)
			{
				if(shaderFXAA.BeginDraw(drawCmd) == true)
				{
					pragma::ShaderPPFXAA::PushConstants pushConstants {};
					pushConstants.subPixelAliasingRemoval = cvFxaaSubPixelAliasingRemoval->GetFloat();
					pushConstants.edgeThreshold = cvFxaaEdgeThreshold->GetFloat();
					pushConstants.minEdgeThreshold = cvFxaaMinEdgeThreshold->GetFloat();

					shaderFXAA.Draw(
						*(*hdrInfo.dsgTonemappedPostProcessing)->get_descriptor_set(0u),
						pushConstants
					);
					shaderFXAA.EndDraw();
				}
				prosper::util::record_end_render_pass(*(*drawCmd));

				prosper::util::record_post_render_pass_image_barrier(
					**drawCmd,**hdrInfo.hdrPostProcessingRenderTarget->GetTexture()->GetImage(),
					Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
				);
			}
			prosper::util::record_image_barrier(*(*drawCmd),*(*hdrInfo.sceneRenderTarget->GetTexture()->GetImage()),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			prosper::util::record_image_barrier(*(*drawCmd),*(*hdrTex->GetImage()),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
			hdrInfo.BlitStagingRenderTargetToMainRenderTarget(*drawCmd);
		}
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingFXAA);
	}*/

	// Glow Effects
	//c_engine->StartGPUTimer(GPUTimerEvent::Glow); // prosper TODO
	//c_engine->StopGPUTimer(GPUTimerEvent::Glow); // prosper TODO
	//
}
#endif

bool RasterizationRenderer::RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags)
{
	if(BaseRenderer::RenderScene(drawCmd,renderFlags) == false)
		return false;
	m_stage = Stage::Initial;
	AdvanceRenderStage(drawCmd,renderFlags);
	return true;
}
