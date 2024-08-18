/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/c_renderer_pp_fxaa_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/console/c_cvar.h"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <prosper_command_buffer.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

static auto cvAntiAliasing = GetClientConVar("cl_render_anti_aliasing");
static auto cvFxaaSubPixelAliasingRemoval = GetClientConVar("cl_render_fxaa_sub_pixel_aliasing_removal_amount");
static auto cvFxaaEdgeThreshold = GetClientConVar("cl_render_fxaa_edge_threshold");
static auto cvFxaaMinEdgeThreshold = GetClientConVar("cl_render_fxaa_min_edge_threshold");
CRendererPpFxaaComponent::CRendererPpFxaaComponent(BaseEntity &ent) : CRendererPpBaseComponent(ent) {}
void CRendererPpFxaaComponent::DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::PostProcessingGpuFxaa, *drawSceneInfo.commandBuffer);

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::PostProcessingGpuFxaa, *drawSceneInfo.commandBuffer);
	}};

	if(static_cast<pragma::rendering::AntiAliasing>(cvAntiAliasing->GetInt()) != pragma::rendering::AntiAliasing::FXAA || m_renderer.expired())
		return;
	c_game->StartGPUProfilingStage("PostProcessingFXAA");

	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto whShaderPPFXAA = c_game->GetGameShader(CGame::GameShader::PPFXAA);
	if(whShaderPPFXAA.valid() == true) {
		auto &shaderFXAA = static_cast<pragma::ShaderPPFXAA &>(*whShaderPPFXAA.get());
		auto &prepass = hdrInfo.prepass;

		auto &toneMappedImg = hdrInfo.toneMappedRenderTarget->GetTexture().GetImage();
		drawCmd->RecordImageBarrier(toneMappedImg, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

		auto *srcImg = hdrInfo.dsgTonemappedPostProcessing->GetDescriptorSet()->GetBoundImage(umath::to_integral(pragma::ShaderPPFXAA::TextureBinding::SceneTextureHdr));
		if(srcImg)
			drawCmd->RecordImageBarrier(*srcImg, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

		if(drawCmd->RecordBeginRenderPass(*hdrInfo.toneMappedPostProcessingRenderTarget) == true) {
			prosper::ShaderBindState bindState {*drawCmd};
			if(shaderFXAA.RecordBeginDraw(bindState) == true) {
				pragma::ShaderPPFXAA::PushConstants pushConstants {};
				pushConstants.subPixelAliasingRemoval = cvFxaaSubPixelAliasingRemoval->GetFloat();
				pushConstants.edgeThreshold = cvFxaaEdgeThreshold->GetFloat();
				pushConstants.minEdgeThreshold = cvFxaaMinEdgeThreshold->GetFloat();

				shaderFXAA.RecordDraw(bindState, *hdrInfo.dsgTonemappedPostProcessing->GetDescriptorSet(), pushConstants);
				shaderFXAA.RecordEndDraw(bindState);
			}
			drawCmd->RecordEndRenderPass();

			drawCmd->RecordPostRenderPassImageBarrier(hdrInfo.toneMappedPostProcessingRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		}

		// Blit FXAA output back to tonemapped image, which will be used for the presentation
		// TODO: This blit operation isn't actually necessary, it would be more performant to just
		// use the FXAA output image directly for presentation!
		auto &fxaaOutputImg = hdrInfo.toneMappedPostProcessingRenderTarget->GetTexture().GetImage();
		drawCmd->RecordImageBarrier(fxaaOutputImg, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
		drawCmd->RecordImageBarrier(toneMappedImg, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
		drawCmd->RecordBlitImage({}, hdrInfo.toneMappedPostProcessingRenderTarget->GetTexture().GetImage(), hdrInfo.toneMappedRenderTarget->GetTexture().GetImage());
		drawCmd->RecordImageBarrier(toneMappedImg, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::TransferSrcOptimal);
		// TODO: This would be better placed BEFORE the FXAA render pass
		drawCmd->RecordImageBarrier(fxaaOutputImg, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);

		if(srcImg)
			drawCmd->RecordImageBarrier(*srcImg, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	}
	c_game->StopGPUProfilingStage(); // PostProcessingFXAA
}
void CRendererPpFxaaComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
