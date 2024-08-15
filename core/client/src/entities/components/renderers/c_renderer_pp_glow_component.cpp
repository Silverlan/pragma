/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/c_renderer_pp_glow_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_bloom_blur.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/world/c_shader_glow.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/console/c_cvar.h"
#include <pragma/entities/entity_component_manager_t.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

static auto cvBloomEnabled = GetClientConVar("render_bloom_enabled");
static auto cvBloomAmount = GetClientConVar("render_bloom_amount");

static util::WeakHandle<prosper::Shader> g_bloomBlurH {};
static util::WeakHandle<prosper::Shader> g_bloomBlurV {};
static void init_shaders()
{
	if(g_bloomBlurH.expired())
		g_bloomBlurH = c_engine->GetShader("pp_bloom_blur_h");
	if(g_bloomBlurV.expired())
		g_bloomBlurV = c_engine->GetShader("pp_bloom_blur_v");
}

void CRendererPpGlowComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CRendererPpGlowComponent;

	using TBlurRadius = uint32_t;
	{
		auto memberInfo = create_component_member_info<T, TBlurRadius, static_cast<void (T::*)(TBlurRadius)>(&T::SetBlurRadius), static_cast<TBlurRadius (T::*)() const>(&T::GetBlurRadius)>("blurRadius", ShaderPPBloomBlurBase::DEFAULT_RADIUS);
		memberInfo.SetMin(0);
		memberInfo.SetMax(ControlledBlurSettings::MAX_BLUR_RADIUS);
		registerMember(std::move(memberInfo));
	}

	using TBlurSigma = double;
	{
		auto memberInfo = create_component_member_info<T, TBlurSigma, static_cast<void (T::*)(TBlurSigma)>(&T::SetBlurSigma), static_cast<TBlurSigma (T::*)() const>(&T::GetBlurSigma)>("blurSigma", ShaderPPBloomBlurBase::DEFAULT_SIGMA);
		memberInfo.SetMin(0);
		memberInfo.SetMax(ControlledBlurSettings::MAX_BLUR_SIGMA);
		registerMember(std::move(memberInfo));
	}

	using TBlurAmount = int32_t;
	{
		auto memberInfo = create_component_member_info<T, TBlurAmount, static_cast<void (T::*)(TBlurAmount)>(&T::SetBlurAmount), static_cast<TBlurAmount (T::*)() const>(&T::GetBlurAmount)>("blurAmount", -1);
		memberInfo.SetMin(-1);
		memberInfo.SetMax(20);
		registerMember(std::move(memberInfo));
	}
}

CRendererPpGlowComponent::CRendererPpGlowComponent(BaseEntity &ent) : CRendererPpBaseComponent(ent) { SetPipelineDirty(); }
prosper::Texture &CRendererPpGlowComponent::GetGlowTexture() { return m_glowRt->GetTexture(); }
#include <cmaterial.h>
void CRendererPpGlowComponent::DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &texture = m_glowRt->GetTexture();
	// Blit high-res bloom image into low-res image, which is cheaper to blur
	drawCmd->RecordImageBarrier(texture.GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
	drawCmd->RecordImageBarrier(m_blurRt->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
	drawCmd->RecordBlitTexture(texture, m_blurRt->GetTexture().GetImage());

	if(!m_controlledBlurSettings.IsValid())
		return;

	drawCmd->RecordImageBarrier(texture.GetImage(), prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	drawCmd->RecordImageBarrier(m_blurRt->GetTexture().GetImage(), prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	m_controlledBlurSettings.RecordBlur(drawCmd, *m_blurSet);
	//drawCmd->RecordImageBarrier(bloomTexture->GetImage(), prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
}
void CRendererPpGlowComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CRendererPpGlowComponent::ExecuteGlowPass(const util::DrawSceneInfo &drawSceneInfo)
{
#if 0
	static auto matInitialized = false;
	if(!matInitialized) {
		auto mat = pragma::get_client_state()->LoadMaterial("models/antlion_guard/antlionguard2");
		if(mat) {
			auto *shader = static_cast<pragma::ShaderGameWorldLightingPass *>(pragma::get_cengine()->GetShader("glow").get());
			shader->InitializeMaterialDescriptorSet(static_cast<CMaterial &>(*mat));
			matInitialized = true;
		}
	}
#endif

	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::PostProcessingGpuGlow, *drawSceneInfo.commandBuffer);

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::PostProcessingGpuGlow, *drawSceneInfo.commandBuffer);
	}};

	c_game->StartGPUProfilingStage("PostProcessingBloom");

	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &texGlow = m_glowRt->GetTexture();
	drawCmd->RecordImageBarrier(texGlow.GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
	drawCmd->RecordClearImage(texGlow.GetImage(), prosper::ImageLayout::TransferDstOptimal, std::array<float, 4> {0.f, 0.f, 0.f, 0.f});
	drawCmd->RecordImageBarrier(texGlow.GetImage(), prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ColorAttachmentOptimal);

	//

	// TODO: This shouldn't be here
	if(drawSceneInfo.commandBuffer->RecordBeginRenderPass(*m_glowRt, {prosper::ClearColorValue {std::array<float, 4> {0.f, 0.f, 0.f, 1.f}}, prosper::ClearColorValue {std::array<float, 4> {0.f, 0.f, 0.f, 1.f}}}, prosper::IPrimaryCommandBuffer::RenderPassFlags::SecondaryCommandBuffers)) {
		m_glowCommandBufferGroup->ExecuteCommands(*drawCmd);

		// TODO
		auto cRenderer = GetEntity().GetComponent<CRasterizationRendererComponent>();
		if(cRenderer.valid())
			m_renderer->RecordRenderParticleSystems(*drawCmd, drawSceneInfo, cRenderer->GetCulledParticles(), pragma::rendering::SceneRenderPass::World, false /* depthPass */, true /* glow */);
	}

	drawSceneInfo.commandBuffer->RecordEndRenderPass();
	//
	c_game->StopGPUProfilingStage(); // PostProcessingBloom
}
void CRendererPpGlowComponent::RecordGlowPass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &rt = m_glowRt;
	m_glowCommandBufferGroup->StartRecording(rt->GetRenderPass(), rt->GetFramebuffer());

	if(!umath::is_flag_set(drawSceneInfo.flags, util::DrawSceneInfo::Flags::DisableLightingPass)) {
		if((drawSceneInfo.renderFlags & RenderFlags::Glow) != RenderFlags::None) {
			m_glowCommandBufferGroup->Record([this, &drawSceneInfo](prosper::ISecondaryCommandBuffer &cmd) mutable {
				util::RenderPassDrawInfo rpDrawInfo {drawSceneInfo, cmd};
				pragma::rendering::LightingStageRenderProcessor rsys {rpDrawInfo, {} /* drawOrigin */};
				auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
				auto *glowStageStats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::GlowPass) : nullptr;
				rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::Glow, false /* translucent */), glowStageStats);
			});
		}
	}

	m_glowCommandBufferGroup->EndRecording();
}
void CRendererPpGlowComponent::SetBlurRadius(uint32_t radius)
{
	m_controlledBlurSettings.SetRadius(radius);
	SetPipelineDirty();
}
void CRendererPpGlowComponent::SetBlurSigma(double sigma)
{
	m_controlledBlurSettings.SetSigma(sigma);
	SetPipelineDirty();
}
uint32_t CRendererPpGlowComponent::GetBlurRadius() const { return m_controlledBlurSettings.GetRadius(); }
double CRendererPpGlowComponent::GetBlurSigma() const { return m_controlledBlurSettings.GetSigma(); }

void CRendererPpGlowComponent::SetBlurAmount(int32_t blurAmount) { m_controlledBlurSettings.SetBlurAmount(blurAmount); }
int32_t CRendererPpGlowComponent::GetBlurAmount() const { return m_controlledBlurSettings.GetBlurAmount(); }
void CRendererPpGlowComponent::Initialize()
{
	CRendererPpBaseComponent::Initialize();

	BindEventUnhandled(pragma::CRendererComponent::EVENT_ON_RENDER_TARGET_RELOADED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { InitializeRenderTarget(); });
	BindEventUnhandled(pragma::CRasterizationRendererComponent::EVENT_ON_RECORD_LIGHTING_PASS, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { RecordGlowPass(static_cast<pragma::CEDrawSceneInfo &>(evData.get()).drawSceneInfo); });
	BindEventUnhandled(pragma::CRasterizationRendererComponent::EVENT_POST_LIGHTING_PASS, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ExecuteGlowPass(static_cast<pragma::CEDrawSceneInfo &>(evData.get()).drawSceneInfo); });
}
void CRendererPpGlowComponent::InitializeRenderTarget()
{
	auto rendererC = GetEntity().GetComponent<pragma::CRendererComponent>();
	auto cRenderer = GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rendererC.expired() || cRenderer.expired())
		return;

	auto &context = c_engine->GetRenderContext();

	m_glowCommandBufferGroup = context.CreateSwapCommandBufferGroup(context.GetWindow());

	// TODO: Image view?
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.width = cRenderer->GetPrepass().textureDepth->GetImage().GetWidth();
	createInfo.height = cRenderer->GetPrepass().textureDepth->GetImage().GetHeight();
	createInfo.format = pragma::ShaderGlow::RENDER_PASS_FORMAT;
	createInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	createInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;

	auto img = context.CreateImage(createInfo);
	auto tex = context.CreateTexture({}, *img, prosper::util::ImageViewCreateInfo {}, prosper::util::SamplerCreateInfo {});

	auto imgBloom = context.CreateImage(createInfo);
	auto texBloom = context.CreateTexture({}, *imgBloom, prosper::util::ImageViewCreateInfo {}, prosper::util::SamplerCreateInfo {});
	//auto &descSetHdrResolve = *dsgBloomTonemapping->GetDescriptorSet();
	//descSetHdrResolve.SetBindingTexture(*bloomBlurTexture, umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Bloom));

	auto rp = static_cast<prosper::ShaderGraphics *>(pragma::get_cengine()->GetShader("glow").get())->GetRenderPass();

	auto rt = context.CreateRenderTarget({tex, texBloom, cRenderer->GetPrepass().textureDepth}, rp);
	rt->SetDebugName("scene_glow_rt");
	m_glowRt = rt;

	// Blur
	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;

	// TODO
	uint32_t width = 512;
	uint32_t height = 512;

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = pragma::ShaderGameWorldLightingPass::RENDER_PASS_FORMAT;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	// imgCreateInfo.samples = sampleCount;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;

	imgCreateInfo.usage
	  = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::TransferSrcBit; // Note: Transfer flag required for debugging purposes only (See debug_glow_bloom console command)
	//if(sampleCount != prosper::SampleCountFlags::e1Bit)
	//	imgCreateInfo.usage |= prosper::ImageUsageFlags::TransferSrcBit;
	imgCreateInfo.usage |= prosper::ImageUsageFlags::TransferSrcBit;

	// The bloom image has to be blurred multiple times, which is expensive for larger resolutions.
	// We don't really care about the quality of the blur image though, so we're using a smaller
	// version of the bloom image for post-processing.
	auto aspectRatio = width / static_cast<float>(height);
	imgCreateInfo.width = width;
	imgCreateInfo.height = static_cast<uint32_t>(imgCreateInfo.width * aspectRatio);

	prosper::util::ImageViewCreateInfo hdrImgViewCreateInfo {};
	// Note: We need the alpha channel for FXAA Luma
	// hdrImgViewCreateInfo.swizzleAlpha = prosper::ComponentSwizzle::One;
	prosper::util::SamplerCreateInfo hdrSamplerCreateInfo {};
	hdrSamplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	hdrSamplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	hdrSamplerCreateInfo.minFilter = prosper::Filter::Linear; // Note: These have to be linear for FXAA!
	hdrSamplerCreateInfo.magFilter = prosper::Filter::Linear;

	if((imgCreateInfo.height % 2) != 0)
		++imgCreateInfo.height;
	auto hdrBloomBlurImg = context.CreateImage(imgCreateInfo);
	auto bloomBlurTexture = context.CreateTexture(texCreateInfo, *hdrBloomBlurImg, hdrImgViewCreateInfo, hdrSamplerCreateInfo);
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	m_blurRt = context.CreateRenderTarget({bloomBlurTexture}, prosper::ShaderGraphics::GetRenderPass<prosper::ShaderBlurBase>(context, umath::to_integral(prosper::ShaderBlurBase::Pipeline::R16G16B16A16Sfloat)));
	m_blurRt->SetDebugName("scene_glow_blur_rt");
	m_blurSet = prosper::BlurSet::Create(context, m_blurRt);

	auto &descSet = *cRenderer->GetHDRInfo().dsgBloomTonemapping->GetDescriptorSet();
	descSet.SetBindingTexture(*bloomBlurTexture, umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));
}
void CRendererPpGlowComponent::SetPipelineDirty() { SetTickPolicy(pragma::TickPolicy::Always); }

void CRendererPpGlowComponent::OnTick(double dt)
{
	CRendererPpBaseComponent::OnTick(dt);
	SetTickPolicy(TickPolicy::Never);
	m_controlledBlurSettings.UpdateShaderPipelines();
}
