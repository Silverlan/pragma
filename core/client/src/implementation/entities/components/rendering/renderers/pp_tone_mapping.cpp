// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :entities.components.pp_tone_mapping;
import :engine;
import :game;
import :rendering.shaders;

using namespace pragma;

static auto cvToneMapping = console::get_client_con_var("cl_render_tone_mapping");
CRendererPpToneMappingComponent::CRendererPpToneMappingComponent(ecs::BaseEntity &ent) : CRendererPpBaseComponent(ent) {}
PostProcessingEffectData::Flags CRendererPpToneMappingComponent::GetFlags() const { return m_applyToHdrImage ? PostProcessingEffectData::Flags::None : PostProcessingEffectData::Flags::ToneMapped; }
void CRendererPpToneMappingComponent::SetApplyToHdrImage(bool applyToHdrImage) { m_applyToHdrImage = applyToHdrImage; }
void CRendererPpToneMappingComponent::DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuToneMapping, *drawSceneInfo.commandBuffer);
	get_cgame()->StartGPUProfilingStage("PostProcessingHDR");

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		get_cgame()->StopGPUProfilingStage(); // PostProcessingHDR
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuToneMapping, *drawSceneInfo.commandBuffer);
	}};

	auto hShaderTonemapping = get_cgame()->GetGameShader(CGame::GameShader::PPTonemapping);
	if(hShaderTonemapping.expired() || m_renderer.expired())
		return;
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto &dsgBloomTonemapping = hdrInfo.dsgBloomTonemapping;
	auto &descSetHdrResolve = *dsgBloomTonemapping->GetDescriptorSet();
	auto *srcImg = descSetHdrResolve.GetBoundImage(math::to_integral(ShaderPPHDR::TextureBinding::Texture));
	auto *srcImgBloomBlur = descSetHdrResolve.GetBoundImage(math::to_integral(ShaderPPHDR::TextureBinding::Bloom));
	// auto *srcImgGlow = descSetHdrResolve.GetBoundImage(pragma::math::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));
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
	auto pipeline = m_applyToHdrImage ? ShaderPPHDR::Pipeline::HDR : ShaderPPHDR::Pipeline::LDR;
	if(drawCmd->RecordBeginRenderPass(*rt) == true) {
		auto &shaderPPHdr = static_cast<ShaderPPHDR &>(*hShaderTonemapping);

		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderPPHdr.RecordBeginDraw(bindState, math::to_integral(pipeline)) == true) {
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

			shaderPPHdr.RecordDraw(bindState, descSetHdrResolve, toneMapping, m_renderer->GetHDRExposure(), bloomAdditiveScale, math::is_flag_set(drawSceneInfo.flags, rendering::DrawSceneInfo::Flags::FlipVertically));
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
void CRendererPpToneMappingComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
