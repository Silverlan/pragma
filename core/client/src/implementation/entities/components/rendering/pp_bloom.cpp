// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.pp_bloom;
import :entities.components.rasterization_renderer;
import :engine;
import :game;
import :rendering.shaders;

using namespace pragma;

static auto cvBloomEnabled = console::get_client_con_var("render_bloom_enabled");

void CRendererPpBloomComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CRendererPpBloomComponent;

	using TBlurRadius = uint32_t;
	{
		auto memberInfo = create_component_member_info<T, TBlurRadius, static_cast<void (T::*)(TBlurRadius)>(&T::SetBlurRadius), static_cast<TBlurRadius (T::*)() const>(&T::GetBlurRadius)>("blurRadius", ShaderPPBloomBlurBase::DEFAULT_RADIUS);
		memberInfo.SetMin(0);
		memberInfo.SetMax(rendering::ControlledBlurSettings::MAX_BLUR_RADIUS);
		registerMember(std::move(memberInfo));
	}

	using TBlurSigma = double;
	{
		auto memberInfo = create_component_member_info<T, TBlurSigma, static_cast<void (T::*)(TBlurSigma)>(&T::SetBlurSigma), static_cast<TBlurSigma (T::*)() const>(&T::GetBlurSigma)>("blurSigma", ShaderPPBloomBlurBase::DEFAULT_SIGMA);
		memberInfo.SetMin(0);
		memberInfo.SetMax(rendering::ControlledBlurSettings::MAX_BLUR_SIGMA);
		registerMember(std::move(memberInfo));
	}

	using TBloomThreshold = float;
	{
		auto memberInfo = create_component_member_info<T, TBloomThreshold, static_cast<void (T::*)(TBloomThreshold)>(&T::SetBloomThreshold), static_cast<TBloomThreshold (T::*)() const>(&T::GetBloomThreshold)>("bloomThreshold", 1.f);
		memberInfo.SetMin(0);
		memberInfo.SetMax(10.f);
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

CRendererPpBloomComponent::CRendererPpBloomComponent(ecs::BaseEntity &ent) : CRendererPpBaseComponent(ent) { SetPipelineDirty(); }
void CRendererPpBloomComponent::SetBloomThreshold(float threshold)
{
	m_bloomThreshold = threshold;
	auto rasterC = GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rasterC.valid())
		rasterC->SetBloomThreshold(threshold);
}
float CRendererPpBloomComponent::GetBloomThreshold() const { return m_bloomThreshold; }
void CRendererPpBloomComponent::DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuBloom, *drawSceneInfo.commandBuffer);

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(rendering::RenderStats::RenderStage::PostProcessingGpuBloom, *drawSceneInfo.commandBuffer);
	}};

	if(cvBloomEnabled->GetBool() == false)
		return;

	if(!m_controlledBlurSettings.IsValid())
		return;

	get_cgame()->StartGPUProfilingStage("PostProcessingBloom");
	auto &hdrInfo = m_renderer->GetHDRInfo();
	auto bloomTexMsaa = hdrInfo.sceneRenderTarget->GetTexture(1u);
	auto &drawCmd = drawSceneInfo.commandBuffer;
	// Blit high-res bloom image into low-res image, which is cheaper to blur
	drawCmd->RecordImageBarrier(hdrInfo.bloomTexture->GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
	drawCmd->RecordImageBarrier(hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferDstOptimal);
	drawCmd->RecordBlitTexture(*hdrInfo.bloomTexture, hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage());

	drawCmd->RecordImageBarrier(hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	m_controlledBlurSettings.RecordBlur(drawCmd, *hdrInfo.bloomBlurSet);
	drawCmd->RecordImageBarrier(hdrInfo.bloomTexture->GetImage(), prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	// drawCmd->RecordImageBarrier(hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	get_cgame()->StopGPUProfilingStage(); // PostProcessingBloom
}
void CRendererPpBloomComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CRendererPpBloomComponent::SetBlurRadius(uint32_t radius)
{
	m_controlledBlurSettings.SetRadius(radius);
	SetPipelineDirty();
}
void CRendererPpBloomComponent::SetBlurSigma(double sigma)
{
	m_controlledBlurSettings.SetSigma(sigma);
	SetPipelineDirty();
}
uint32_t CRendererPpBloomComponent::GetBlurRadius() const { return m_controlledBlurSettings.GetRadius(); }
double CRendererPpBloomComponent::GetBlurSigma() const { return m_controlledBlurSettings.GetSigma(); }

void CRendererPpBloomComponent::SetBlurAmount(int32_t blurAmount) { m_controlledBlurSettings.SetBlurAmount(blurAmount); }
int32_t CRendererPpBloomComponent::GetBlurAmount() const { return m_controlledBlurSettings.GetBlurAmount(); }

void CRendererPpBloomComponent::SetPipelineDirty() { SetTickPolicy(TickPolicy::Always); }

void CRendererPpBloomComponent::OnTick(double dt)
{
	CRendererPpBaseComponent::OnTick(dt);
	SetTickPolicy(TickPolicy::Never);
	m_controlledBlurSettings.UpdateShaderPipelines();
}
