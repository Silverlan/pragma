/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/rendering/c_settings.hpp"
#include <shader/prosper_shader_blur.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <sharedutils/scope_guard.h>

extern DLLCLIENT CEngine *c_engine;

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
	auto *renderer = const_cast<pragma::CSceneComponent *>(scene.get())->GetRenderer();
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
