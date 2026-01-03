// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :entities.components.pp_fxaa;
import :engine;
import :game;
import :rendering.shaders;

using namespace pragma;

static auto cvAntiAliasing = console::get_client_con_var("cl_render_anti_aliasing");
static auto cvFxaaSubPixelAliasingRemoval = console::get_client_con_var("cl_render_fxaa_sub_pixel_aliasing_removal_amount");
static auto cvFxaaEdgeThreshold = console::get_client_con_var("cl_render_fxaa_edge_threshold");
static auto cvFxaaMinEdgeThreshold = console::get_client_con_var("cl_render_fxaa_min_edge_threshold");
CRendererPpFxaaComponent::CRendererPpFxaaComponent(ecs::BaseEntity &ent) : CRendererPpBaseComponent(ent) {}
void CRendererPpFxaaComponent::DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuFxaa, *drawSceneInfo.commandBuffer);

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuFxaa, *drawSceneInfo.commandBuffer);
	}};

	if(static_cast<rendering::AntiAliasing>(cvAntiAliasing->GetInt()) != rendering::AntiAliasing::FXAA || m_renderer.expired())
		return;
	get_cgame()->StartGPUProfilingStage("PostProcessingFXAA");

	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto whShaderPPFXAA = get_cgame()->GetGameShader(CGame::GameShader::PPFXAA);
	if(whShaderPPFXAA.valid() == true) {
		auto &shaderFXAA = static_cast<ShaderPPFXAA &>(*whShaderPPFXAA.get());
		auto &prepass = hdrInfo.prepass;

		auto &toneMappedImg = hdrInfo.toneMappedRenderTarget->GetTexture().GetImage();
		drawCmd->RecordImageBarrier(toneMappedImg, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

		auto *srcImg = hdrInfo.dsgTonemappedPostProcessing->GetDescriptorSet()->GetBoundImage(math::to_integral(ShaderPPFXAA::TextureBinding::SceneTextureHdr));
		if(srcImg)
			drawCmd->RecordImageBarrier(*srcImg, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

		if(drawCmd->RecordBeginRenderPass(*hdrInfo.toneMappedPostProcessingRenderTarget) == true) {
			prosper::ShaderBindState bindState {*drawCmd};
			if(shaderFXAA.RecordBeginDraw(bindState) == true) {
				ShaderPPFXAA::PushConstants pushConstants {};
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
	get_cgame()->StopGPUProfilingStage(); // PostProcessingFXAA
}
void CRendererPpFxaaComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
