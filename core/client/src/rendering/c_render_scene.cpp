/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/rendering/c_settings.hpp"
#include <shader/prosper_shader_blur.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <sharedutils/scope_guard.h>

extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
void CGame::RenderScenePresent(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,prosper::Texture &texPostHdr,prosper::IImage *optOutImage,uint32_t layerId)
{
	if(optOutImage)
	{
		drawCmd->RecordImageBarrier(*optOutImage,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferDstOptimal);
		prosper::util::BlitInfo blitInfo {};
		blitInfo.dstSubresourceLayer.baseArrayLayer = layerId;
		drawCmd->RecordBlitImage(blitInfo,texPostHdr.GetImage(),*optOutImage);
		drawCmd->RecordImageBarrier(*optOutImage,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	}
	drawCmd->RecordImageBarrier(texPostHdr.GetImage(),prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
}

std::shared_ptr<prosper::IPrimaryCommandBuffer> CGame::GetCurrentDrawCommandBuffer() const {return m_currentDrawCmd.lock();}
	
void CGame::RenderScene(const util::DrawSceneInfo &drawSceneInfo)
{
	m_currentDrawCmd = drawSceneInfo.commandBuffer;
	ScopeGuard sgCurrentDrawCmd {[this]() {
		m_currentDrawCmd = {};
	}};

	auto &scene = GetRenderScene();
	auto *renderer = scene->GetRenderer();
	if(renderer)
	{
		renderer->RenderScene(drawSceneInfo);
		StartProfilingStage(CGame::GPUProfilingPhase::Present);
		StartProfilingStage(CGame::CPUProfilingPhase::Present);

		prosper::Texture *presentationTexture = nullptr;
		if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::HDR))
			presentationTexture = drawSceneInfo.renderTarget ? &drawSceneInfo.renderTarget->GetTexture() : renderer->GetHDRPresentationTexture();
		else
			presentationTexture = renderer->GetPresentationTexture();

		RenderScenePresent(drawSceneInfo.commandBuffer,*presentationTexture,drawSceneInfo.outputImage.get(),drawSceneInfo.outputLayerId);
		StopProfilingStage(CGame::CPUProfilingPhase::Present);
		StopProfilingStage(CGame::GPUProfilingPhase::Present);
	}
	CallCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("PostRenderScene",drawSceneInfo);
	CallLuaCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("PostRenderScene",drawSceneInfo);
}
#pragma optimize("",on)
