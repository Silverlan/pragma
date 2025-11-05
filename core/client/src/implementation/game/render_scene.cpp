// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :game;
import :engine;
import :entities.components.render;
import :entities.components.renderer;
import :rendering.shaders;

void CGame::RenderScenePresent(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, prosper::Texture &texPostHdr, prosper::IImage *optOutImage, uint32_t layerId)
{
	if(optOutImage) {
		drawCmd->RecordImageBarrier(*optOutImage, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferDstOptimal);
		prosper::util::BlitInfo blitInfo {};
		blitInfo.dstSubresourceLayer.baseArrayLayer = layerId;
		drawCmd->RecordBlitImage(blitInfo, texPostHdr.GetImage(), *optOutImage);
		drawCmd->RecordImageBarrier(*optOutImage, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	}
	drawCmd->RecordImageBarrier(texPostHdr.GetImage(), prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
}

std::shared_ptr<prosper::IPrimaryCommandBuffer> CGame::GetCurrentDrawCommandBuffer() const { return m_currentDrawCmd.lock(); }

void CGame::RenderScene(const util::DrawSceneInfo &drawSceneInfo)
{
	m_currentDrawCmd = drawSceneInfo.commandBuffer;
	util::ScopeGuard sgCurrentDrawCmd {[this]() { m_currentDrawCmd = {}; }};

	std::chrono::steady_clock::time_point t;
	if(drawSceneInfo.renderStats) {
		t = std::chrono::steady_clock::now();
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::RenderSceneGpu, *drawSceneInfo.commandBuffer);
	}

	CallCallbacks<void, std::reference_wrapper<const util::DrawSceneInfo>>("PreRenderScene", drawSceneInfo);
	CallLuaCallbacks<void, const util::DrawSceneInfo *>("PreRenderScene", &drawSceneInfo);

	auto &scene = drawSceneInfo.scene;
	auto *renderer = const_cast<pragma::CSceneComponent *>(scene.get())->GetRenderer<pragma::CRendererComponent>();
	if(renderer) {
		prosper::Texture *presentationTexture = nullptr;
		if(umath::is_flag_set(drawSceneInfo.renderFlags, RenderFlags::HDR))
			presentationTexture = drawSceneInfo.renderTarget ? &drawSceneInfo.renderTarget->GetTexture() : renderer->GetHDRPresentationTexture();
		else {
			presentationTexture = renderer->GetPresentationTexture();
			drawSceneInfo.commandBuffer->RecordImageBarrier(presentationTexture->GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		}

		StartProfilingStage("Render");
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::RendererGpu, *drawSceneInfo.commandBuffer);
		renderer->Render(drawSceneInfo);
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::RendererGpu, *drawSceneInfo.commandBuffer);
		StopProfilingStage(); // Render

		StartGPUProfilingStage("Present");
		StartProfilingStage("Present");

		RenderScenePresent(drawSceneInfo.commandBuffer, *presentationTexture, drawSceneInfo.outputImage.get(), drawSceneInfo.outputLayerId);
		StopProfilingStage();    // Present
		StopGPUProfilingStage(); // Present
	}
	CallCallbacks<void, std::reference_wrapper<const util::DrawSceneInfo>>("PostRenderScene", drawSceneInfo);
	CallLuaCallbacks<void, const util::DrawSceneInfo *>("PostRenderScene", &drawSceneInfo);

	if(drawSceneInfo.renderStats) {
		(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::RenderSceneGpu, *drawSceneInfo.commandBuffer);
		(*drawSceneInfo.renderStats)->SetTime(RenderStats::RenderStage::RenderSceneCpu, std::chrono::steady_clock::now() - t);
	}
}
