// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :entities.components.pp_dof;
import :engine;
import :entities.components.optical_camera;
import :game;
import :rendering.shaders;

using namespace pragma;

CRendererPpDoFComponent::CRendererPpDoFComponent(ecs::BaseEntity &ent) : CRendererPpBaseComponent(ent) {}
void CRendererPpDoFComponent::DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuDoF, *drawSceneInfo.commandBuffer);
	get_cgame()->StartGPUProfilingStage("PostProcessingDoF");

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		get_cgame()->StopGPUProfilingStage(); // PostProcessingDoF
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuDoF, *drawSceneInfo.commandBuffer);
	}};

	if(drawSceneInfo.scene.expired() || m_renderer.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &cam = scene.GetActiveCamera();
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &hShaderDof = get_cgame()->GetGameShader(CGame::GameShader::PPDoF);
	if(hShaderDof.expired() || cam.expired())
		return;
	auto opticalC = cam->GetEntity().GetComponent<COpticalCameraComponent>();
	if(opticalC.expired())
		return;
	auto &shaderDoF = static_cast<ShaderPPDoF &>(*hShaderDof.get());
	auto &prepass = hdrInfo.prepass;
	auto texDepth = prepass.textureDepth;
	if(texDepth->IsMSAATexture()) {
		texDepth = static_cast<prosper::MSAATexture &>(*texDepth).Resolve(*drawCmd, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	}
	else
		drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	//texDepth->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal);

	auto &hdrTex = hdrInfo.sceneRenderTarget->GetTexture();
	drawCmd->RecordImageBarrier(hdrTex.GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	drawCmd->RecordBufferBarrier(*scene.GetCameraBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	drawCmd->RecordBufferBarrier(*scene.GetRenderSettingsBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	drawCmd->RecordBufferBarrier(*scene.GetFogBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	if(drawCmd->RecordBeginRenderPass(*hdrInfo.hdrPostProcessingRenderTarget) == true) {
		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderDoF.RecordBeginDraw(bindState) == true) {
			ShaderPPDoF::PushConstants pushConstants {};
			pushConstants.mvp = cam->GetViewMatrix() * cam->GetProjectionMatrix();
			pushConstants.width = scene.GetWidth();
			pushConstants.height = scene.GetHeight();

			pushConstants.focalDepth = opticalC->GetFocalDistance();
			pushConstants.focalLength = opticalC->GetFocalLength();
			pushConstants.fstop = opticalC->GetFStop();

			pushConstants.zNear = cam->GetNearZ();
			pushConstants.zFar = cam->GetFarZ();

			pushConstants.flags = ShaderPPDoF::Flags::None;
			math::set_flag(pushConstants.flags, ShaderPPDoF::Flags::EnableVignette, opticalC->IsVignetteEnabled());
			math::set_flag(pushConstants.flags, ShaderPPDoF::Flags::PentagonBokehShape, opticalC->GetPentagonBokehShape());
			math::set_flag(pushConstants.flags, ShaderPPDoF::Flags::DebugShowDepth, opticalC->GetDebugShowDepth());
			math::set_flag(pushConstants.flags, ShaderPPDoF::Flags::DebugShowFocus, opticalC->GetDebugShowFocus());
			pushConstants.rings = opticalC->GetRingCount();
			pushConstants.ringSamples = opticalC->GetRingSamples();
			pushConstants.CoC = opticalC->GetCircleOfConfusionSize();
			pushConstants.maxBlur = opticalC->GetMaxBlur();
			pushConstants.dither = opticalC->GetDitherAmount();
			pushConstants.vignIn = opticalC->GetVignettingInnerBorder();
			pushConstants.vignOut = opticalC->GetVignettingOuterBorder();
			pushConstants.pentagonShapeFeather = opticalC->GetPentagonShapeFeather();

			shaderDoF.RecordDraw(bindState, *hdrInfo.dsgHDRPostProcessing->GetDescriptorSet(), *hdrInfo.dsgDepthPostProcessing->GetDescriptorSet(), pushConstants);
			shaderDoF.RecordEndDraw(bindState);
		}
		drawCmd->RecordEndRenderPass();
	}
	drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal);

	hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
}
void CRendererPpDoFComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
