// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :game;
import :rendering.shaders;

using namespace pragma::rendering;

void pragma::CRasterizationRendererComponent::RenderSSAO(const DrawSceneInfo &drawSceneInfo)
{
	auto &ssaoInfo = GetSSAOInfo();
	auto *shaderSSAO = static_cast<ShaderSSAO *>(ssaoInfo.GetSSAOShader());
	auto *shaderSSAOBlur = static_cast<ShaderSSAOBlur *>(ssaoInfo.GetSSAOBlurShader());
	if(IsSSAOEnabled() == false || shaderSSAO == nullptr || shaderSSAOBlur == nullptr || drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	get_cgame()->StartProfilingStage("SSAO");
	get_cgame()->StartGPUProfilingStage("SSAO");
	// Pre-render depths, positions and normals (Required for SSAO)
	//auto *renderInfo  = scene.GetSceneRenderDesc().GetRenderInfo(RenderMode::World);
	auto &drawCmd = drawSceneInfo.commandBuffer;
	//if(renderInfo != nullptr)
	{
		// SSAO
		auto &prepass = GetPrepass();
		auto &ssaoImg = ssaoInfo.renderTarget->GetTexture().GetImage();

		auto texNormals = prepass.textureNormals;
		auto bNormalsMultiSampled = texNormals->IsMSAATexture();
		if(bNormalsMultiSampled) {
			texNormals = static_cast<prosper::MSAATexture &>(*texNormals).Resolve(*drawCmd, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		}
		else
			drawCmd->RecordImageBarrier(texNormals->GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		auto texDepth = prepass.textureDepth;
		if(texDepth->IsMSAATexture()) {
			texDepth = static_cast<prosper::MSAATexture &>(*texDepth).Resolve(*drawCmd, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		}
		else
			drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

		drawCmd->RecordBeginRenderPass(*ssaoInfo.renderTarget);
		auto &renderImage = ssaoInfo.renderTarget->GetTexture().GetImage();
		auto extents = renderImage.GetExtents();

		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderSSAO->RecordBeginDraw(bindState) == true) {
			shaderSSAO->RecordDraw(bindState, scene, *ssaoInfo.descSetGroupPrepass->GetDescriptorSet(), {extents.width, extents.height});
			shaderSSAO->RecordEndDraw(bindState);
		}

		drawCmd->RecordEndRenderPass();

		drawCmd->RecordImageBarrier(texNormals->GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal);

		// Blur SSAO
		drawCmd->RecordImageBarrier(ssaoInfo.renderTargetBlur->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		drawCmd->RecordBeginRenderPass(*ssaoInfo.renderTargetBlur);

		if(shaderSSAOBlur->RecordBeginDraw(bindState) == true) {
			shaderSSAOBlur->RecordDraw(bindState, *ssaoInfo.descSetGroupOcclusion->GetDescriptorSet());
			shaderSSAOBlur->RecordEndDraw(bindState);
		}

		drawCmd->RecordEndRenderPass();
		//

		if(bNormalsMultiSampled) {
			drawCmd->RecordImageBarrier(texNormals->GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
			drawCmd->RecordImageBarrier(texDepth->GetImage(), prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		}

		drawCmd->RecordImageBarrier(ssaoInfo.renderTarget->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	}
	get_cgame()->StopGPUProfilingStage(); // SSAO
	get_cgame()->StopProfilingStage();    // SSAO
}
