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


void CGame::RenderScenePresent(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,prosper::Texture &texPostHdr,prosper::IImage &outImage,uint32_t layerId)
{
	drawCmd->RecordImageBarrier(outImage,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferDstOptimal);
	prosper::util::BlitInfo blitInfo {};
	blitInfo.dstSubresourceLayer.baseArrayLayer = layerId;
	drawCmd->RecordBlitImage(blitInfo,texPostHdr.GetImage(),outImage);
	drawCmd->RecordImageBarrier(texPostHdr.GetImage(),prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	drawCmd->RecordImageBarrier(outImage,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
}

std::shared_ptr<prosper::IPrimaryCommandBuffer> CGame::GetCurrentDrawCommandBuffer() const {return m_currentDrawCmd.lock();}
	
void CGame::RenderScene(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,prosper::IImage &outImage,FRender renderFlags,uint32_t outLayerId)
{
	m_currentDrawCmd = drawCmd;
	ScopeGuard sgCurrentDrawCmd {[this]() {
		m_currentDrawCmd = {};
	}};

	auto &scene = GetRenderScene();
	auto *renderer = scene->GetRenderer();
	if(renderer)
	{
		renderer->RenderScene(drawCmd,renderFlags);
		StartProfilingStage(CGame::GPUProfilingPhase::Present);
		StartProfilingStage(CGame::CPUProfilingPhase::Present);

		prosper::Texture *presentationTexture = nullptr;
		if(umath::is_flag_set(renderFlags,FRender::HDR))
			presentationTexture = renderer->GetHDRPresentationTexture();
		else
			presentationTexture = renderer->GetPresentationTexture();

		RenderScenePresent(drawCmd,*presentationTexture,outImage,outLayerId);
		StopProfilingStage(CGame::CPUProfilingPhase::Present);
		StopProfilingStage(CGame::GPUProfilingPhase::Present);
	}
}

