// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :entities.components.pp_fog;
import :engine;
import :game;
import :rendering.shaders;

using namespace pragma;

CRendererPpFogComponent::CRendererPpFogComponent(ecs::BaseEntity &ent) : CRendererPpBaseComponent(ent) {}
void CRendererPpFogComponent::DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuFog, *drawSceneInfo.commandBuffer);
	get_cgame()->StartGPUProfilingStage("PostProcessingFog");

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		get_cgame()->StopGPUProfilingStage(); // PostProcessingFog
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuFog, *drawSceneInfo.commandBuffer);
	}};

	if(drawSceneInfo.scene.expired() || m_renderer.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto descSetGroupFog = m_renderer->GetFogOverride();
	if(descSetGroupFog == nullptr) {
		auto *worldEnv = scene.GetWorldEnvironment();
		if(worldEnv != nullptr) {
			auto &fog = worldEnv->GetFogSettings();
			if(fog.IsEnabled() == true)
				descSetGroupFog = scene.GetFogDescriptorSetGroup();
		}
	}
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto hShaderFog = get_cgame()->GetGameShader(CGame::GameShader::PPFog);
	if(descSetGroupFog == nullptr || hShaderFog.expired())
		return;
	auto &shaderFog = static_cast<ShaderPPFog &>(*hShaderFog.get());
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
		if(shaderFog.RecordBeginDraw(bindState) == true) {
			shaderFog.RecordDraw(bindState, *hdrInfo.dsgHDRPostProcessing->GetDescriptorSet(), *hdrInfo.dsgDepthPostProcessing->GetDescriptorSet(), *scene.GetCameraDescriptorSetGraphics(), *scene.GetFogDescriptorSetGroup()->GetDescriptorSet());
			shaderFog.RecordEndDraw(bindState);
		}
		drawCmd->RecordEndRenderPass();
	}
	drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal);

	hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
}
void CRendererPpFogComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
