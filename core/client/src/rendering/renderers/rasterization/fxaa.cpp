#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/c_settings.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/game/c_game.h"
#include "pragma/console/c_cvar.h"
#include <pragma/console/convars.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

static auto cvAntiAliasing = GetClientConVar("cl_render_anti_aliasing");
static auto cvFxaaSubPixelAliasingRemoval = GetClientConVar("cl_render_fxaa_sub_pixel_aliasing_removal_amount");
static auto cvFxaaEdgeThreshold = GetClientConVar("cl_render_fxaa_edge_threshold");
static auto cvFxaaMinEdgeThreshold = GetClientConVar("cl_render_fxaa_min_edge_threshold");
#pragma optimize("",off)
void RasterizationRenderer::RenderFXAA(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	if(static_cast<AntiAliasing>(cvAntiAliasing->GetInt()) != AntiAliasing::FXAA)
		return;
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingFXAA);

	auto &hdrInfo = GetHDRInfo();
	auto whShaderPPFXAA = c_game->GetGameShader(CGame::GameShader::PPFXAA);
	if(whShaderPPFXAA.valid() == true)
	{
		auto &shaderFXAA = static_cast<pragma::ShaderPPFXAA&>(*whShaderPPFXAA.get());
		auto &prepass = hdrInfo.prepass;

		auto &toneMappedImg = *hdrInfo.toneMappedRenderTarget->GetTexture()->GetImage();
		prosper::util::record_image_barrier(*(*drawCmd),*toneMappedImg,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		if(prosper::util::record_begin_render_pass(*(*drawCmd),*hdrInfo.toneMappedPostProcessingRenderTarget) == true)
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
				**drawCmd,**hdrInfo.toneMappedPostProcessingRenderTarget->GetTexture()->GetImage(),
				Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL
			);
		}

		// Blit FXAA output back to tonemapped image, which will be used for the presentation
		// TODO: This blit operation isn't actually necessary, it would be more performant to just
		// use the FXAA output image directly for presentation!
		auto &fxaaOutputImg = *hdrInfo.toneMappedPostProcessingRenderTarget->GetTexture()->GetImage();
		prosper::util::record_image_barrier(*(*drawCmd),*fxaaOutputImg,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),*toneMappedImg,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		prosper::util::record_blit_image(**drawCmd,{},**hdrInfo.toneMappedPostProcessingRenderTarget->GetTexture()->GetImage(),**hdrInfo.toneMappedRenderTarget->GetTexture()->GetImage());
		prosper::util::record_image_barrier(*(*drawCmd),*toneMappedImg,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		// TODO: This would be better placed BEFORE the FXAA render pass
		prosper::util::record_image_barrier(*(*drawCmd),*fxaaOutputImg,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	}
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingFXAA);
}
#pragma optimize("",on)
