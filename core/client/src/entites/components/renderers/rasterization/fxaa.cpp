/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/c_settings.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
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

void pragma::CRasterizationRendererComponent::RenderFXAA(const util::DrawSceneInfo &drawSceneInfo)
{
	if(static_cast<AntiAliasing>(cvAntiAliasing->GetInt()) != AntiAliasing::FXAA)
		return;
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingFXAA);

	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &hdrInfo = GetHDRInfo();
	auto whShaderPPFXAA = c_game->GetGameShader(CGame::GameShader::PPFXAA);
	if(whShaderPPFXAA.valid() == true)
	{
		auto &shaderFXAA = static_cast<pragma::ShaderPPFXAA&>(*whShaderPPFXAA.get());
		auto &prepass = hdrInfo.prepass;

		auto &toneMappedImg = hdrInfo.toneMappedRenderTarget->GetTexture().GetImage();
		drawCmd->RecordImageBarrier(toneMappedImg,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
		if(drawCmd->RecordBeginRenderPass(*hdrInfo.toneMappedPostProcessingRenderTarget) == true)
		{
			if(shaderFXAA.BeginDraw(drawCmd) == true)
			{
				pragma::ShaderPPFXAA::PushConstants pushConstants {};
				pushConstants.subPixelAliasingRemoval = cvFxaaSubPixelAliasingRemoval->GetFloat();
				pushConstants.edgeThreshold = cvFxaaEdgeThreshold->GetFloat();
				pushConstants.minEdgeThreshold = cvFxaaMinEdgeThreshold->GetFloat();

				shaderFXAA.Draw(
					*hdrInfo.dsgTonemappedPostProcessing->GetDescriptorSet(),
					pushConstants
				);
				shaderFXAA.EndDraw();
			}
			drawCmd->RecordEndRenderPass();

			drawCmd->RecordPostRenderPassImageBarrier(
				hdrInfo.toneMappedPostProcessingRenderTarget->GetTexture().GetImage(),
				prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ColorAttachmentOptimal
			);
		}

		// Blit FXAA output back to tonemapped image, which will be used for the presentation
		// TODO: This blit operation isn't actually necessary, it would be more performant to just
		// use the FXAA output image directly for presentation!
		auto &fxaaOutputImg = hdrInfo.toneMappedPostProcessingRenderTarget->GetTexture().GetImage();
		drawCmd->RecordImageBarrier(fxaaOutputImg,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal);
		drawCmd->RecordImageBarrier(toneMappedImg,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);
		drawCmd->RecordBlitImage({},hdrInfo.toneMappedPostProcessingRenderTarget->GetTexture().GetImage(),hdrInfo.toneMappedRenderTarget->GetTexture().GetImage());
		drawCmd->RecordImageBarrier(toneMappedImg,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::TransferSrcOptimal);
		// TODO: This would be better placed BEFORE the FXAA render pass
		drawCmd->RecordImageBarrier(fxaaOutputImg,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	}
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingFXAA);
}

