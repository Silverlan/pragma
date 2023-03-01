/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/c_renderer_pp_bloom_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
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

static auto cvBloomEnabled = GetClientConVar("render_bloom_enabled");
static auto cvBloomAmount = GetClientConVar("render_bloom_amount");
CRendererPpBloomComponent::CRendererPpBloomComponent(BaseEntity &ent) : CRendererPpBaseComponent(ent) {}
void CRendererPpBloomComponent::DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::PostProcessingGpuBloom, *drawSceneInfo.commandBuffer);

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::PostProcessingGpuBloom, *drawSceneInfo.commandBuffer);
	}};

	if(cvBloomEnabled->GetBool() == false)
		return;
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingBloom);
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto bloomTexMsaa = hdrInfo.sceneRenderTarget->GetTexture(1u);
	auto &drawCmd = drawSceneInfo.commandBuffer;
	// Blit high-res bloom image into low-res image, which is cheaper to blur
	drawCmd->RecordImageBarrier(hdrInfo.bloomTexture->GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
	drawCmd->RecordImageBarrier(hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
	drawCmd->RecordBlitTexture(*hdrInfo.bloomTexture, hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage());

	static auto blurSize = 5.f;
	static int32_t kernelSize = 9u;
	uint32_t blurAmount = umath::clamp(cvBloomAmount->GetInt(), 0, 20);

	drawCmd->RecordImageBarrier(hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	for(auto i = decltype(blurAmount) {0}; i < blurAmount; ++i) {
		prosper::util::record_blur_image(c_engine->GetRenderContext(), drawCmd, *hdrInfo.bloomBlurSet, {Vector4(1.f, 1.f, 1.f, 1.f), blurSize, kernelSize});
	}
	drawCmd->RecordImageBarrier(hdrInfo.bloomTexture->GetImage(), prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingBloom);
}
void CRendererPpBloomComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
