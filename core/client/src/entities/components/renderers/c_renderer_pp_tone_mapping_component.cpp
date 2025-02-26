/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/renderers/c_renderer_pp_tone_mapping_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
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

static auto cvToneMapping = GetClientConVar("cl_render_tone_mapping");
CRendererPpToneMappingComponent::CRendererPpToneMappingComponent(BaseEntity &ent) : CRendererPpBaseComponent(ent) {}
PostProcessingEffectData::Flags CRendererPpToneMappingComponent::GetFlags() const { return m_applyToHdrImage ? PostProcessingEffectData::Flags::None : PostProcessingEffectData::Flags::ToneMapped; }
void CRendererPpToneMappingComponent::SetApplyToHdrImage(bool applyToHdrImage) { m_applyToHdrImage = applyToHdrImage; }
void CRendererPpToneMappingComponent::DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::PostProcessingGpuToneMapping, *drawSceneInfo.commandBuffer);
	c_game->StartGPUProfilingStage("PostProcessingHDR");

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		c_game->StopGPUProfilingStage(); // PostProcessingHDR
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::PostProcessingGpuToneMapping, *drawSceneInfo.commandBuffer);
	}};

	auto hShaderTonemapping = c_game->GetGameShader(CGame::GameShader::PPTonemapping);
	if(hShaderTonemapping.expired() || m_renderer.expired())
		return;
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto &dsgBloomTonemapping = hdrInfo.dsgBloomTonemapping;
	auto &descSetHdrResolve = *dsgBloomTonemapping->GetDescriptorSet();
	auto *srcImg = descSetHdrResolve.GetBoundImage(umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
	auto *srcImgBloomBlur = descSetHdrResolve.GetBoundImage(umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Bloom));
	// auto *srcImgGlow = descSetHdrResolve.GetBoundImage(umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));
	auto &drawCmd = drawSceneInfo.commandBuffer;
	if(m_renderer->IsMultiSampled() == false) // The resolved images already have the correct layout
	{
		drawCmd->RecordImageBarrier(*srcImg, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		// drawCmd->RecordImageBarrier(*srcImgBloomBlur, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		//.RecordImageBarrier(*(*drawCmd),**srcImgBloom,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
		// drawCmd->RecordImageBarrier(*srcImgGlow,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	}
	auto &dstTexPostHdr = hdrInfo.toneMappedRenderTarget->GetTexture();
	auto &dstImgPostHdr = dstTexPostHdr.GetImage();
	auto rt = m_applyToHdrImage ? hdrInfo.hdrPostProcessingRenderTarget : hdrInfo.toneMappedRenderTarget;
	auto pipeline = m_applyToHdrImage ? pragma::ShaderPPHDR::Pipeline::HDR : pragma::ShaderPPHDR::Pipeline::LDR;
	if(drawCmd->RecordBeginRenderPass(*rt) == true) {
		auto &shaderPPHdr = static_cast<pragma::ShaderPPHDR &>(*hShaderTonemapping);

		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderPPHdr.RecordBeginDraw(bindState, umath::to_integral(pipeline)) == true) {
			const float bloomAdditiveScale = 0.5f;

			rendering::ToneMapping toneMapping;
			if(drawSceneInfo.toneMapping.has_value())
				toneMapping = *drawSceneInfo.toneMapping;
			else {
				toneMapping = rendering::ToneMapping::Reinhard;
				auto toneMappingCvarVal = cvToneMapping->GetInt();
				switch(toneMappingCvarVal) {
				case -1:
					break;
				default:
					toneMapping = static_cast<rendering::ToneMapping>(toneMappingCvarVal + 1);
					break;
				}
			}

			shaderPPHdr.RecordDraw(bindState, descSetHdrResolve, toneMapping, m_renderer->GetHDRExposure(), bloomAdditiveScale, umath::is_flag_set(drawSceneInfo.flags, util::DrawSceneInfo::Flags::FlipVertically));
			shaderPPHdr.RecordEndDraw(bindState);
		}
		drawCmd->RecordEndRenderPass();

		drawCmd->RecordPostRenderPassImageBarrier(dstImgPostHdr, prosper::ImageLayout::ColorAttachmentOptimal, m_applyToHdrImage ? prosper::ImageLayout::ShaderReadOnlyOptimal : prosper::ImageLayout::TransferSrcOptimal);
	}
	if(m_renderer->IsMultiSampled() == false) {
		drawCmd->RecordImageBarrier(*srcImg, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		// drawCmd->RecordImageBarrier(*srcImgBloomBlur, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		//.RecordImageBarrier(*(*drawCmd),srcImgBloom,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
		// drawCmd->RecordImageBarrier(*srcImgGlow,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	}

	if(m_applyToHdrImage)
		hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo, prosper::ImageLayout::ColorAttachmentOptimal);
}
void CRendererPpToneMappingComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
