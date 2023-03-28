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
#include "pragma/rendering/shaders/post_processing/c_shader_pp_bloom_blur.hpp"
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

static util::WeakHandle<prosper::Shader> g_bloomBlurH {};
static util::WeakHandle<prosper::Shader> g_bloomBlurV {};
static void init_shaders()
{
	if(g_bloomBlurH.expired())
		g_bloomBlurH = c_engine->GetShader("pp_bloom_blur_h");
	if(g_bloomBlurV.expired())
		g_bloomBlurV = c_engine->GetShader("pp_bloom_blur_v");
}

CRendererPpBloomComponent::CRendererPpBloomComponent(BaseEntity &ent) : CRendererPpBaseComponent(ent)
{
	init_shaders();
	SetPipelineDirty();
}
void CRendererPpBloomComponent::DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo)
{
	if(g_bloomBlurH.expired() || g_bloomBlurV.expired())
		return;
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::PostProcessingGpuBloom, *drawSceneInfo.commandBuffer);

	util::ScopeGuard scopeGuard {[&drawSceneInfo]() {
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::PostProcessingGpuBloom, *drawSceneInfo.commandBuffer);
	}};

	if(cvBloomEnabled->GetBool() == false)
		return;

	if(!m_bloomPipelineInfoH || !m_bloomPipelineInfoH->pipelineIdx || !m_bloomPipelineInfoV || !m_bloomPipelineInfoV->pipelineIdx)
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

	prosper::util::ShaderInfo shaderInfo {};
	shaderInfo.shaderH = static_cast<prosper::ShaderBlurBase *>(g_bloomBlurH.get());
	shaderInfo.shaderHPipeline = *m_bloomPipelineInfoH->pipelineIdx;
	shaderInfo.shaderV = static_cast<prosper::ShaderBlurBase *>(g_bloomBlurV.get());
	shaderInfo.shaderVPipeline = *m_bloomPipelineInfoV->pipelineIdx;

	drawCmd->RecordImageBarrier(hdrInfo.bloomBlurRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	for(auto i = decltype(blurAmount) {0}; i < blurAmount; ++i) {
		prosper::util::record_blur_image(c_engine->GetRenderContext(), drawCmd, *hdrInfo.bloomBlurSet, {Vector4(1.f, 1.f, 1.f, 1.f), blurSize, kernelSize}, 1u, &shaderInfo);
	}
	drawCmd->RecordImageBarrier(hdrInfo.bloomTexture->GetImage(), prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingBloom);
}
void CRendererPpBloomComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CRendererPpBloomComponent::SetBlurRadius(uint32_t radius)
{
	radius = umath::clamp(radius, 0u, 15u);
	m_radius = radius;
	SetPipelineDirty();
}
void CRendererPpBloomComponent::SetBlurSigma(double sigma)
{
	sigma = umath::clamp(sigma, 0.0, 10.0);
	m_sigma = sigma;
	SetPipelineDirty();
}
uint32_t CRendererPpBloomComponent::GetBlurRadius() const { return m_radius; }
double CRendererPpBloomComponent::GetBlurSigma() const { return m_sigma; }

void CRendererPpBloomComponent::SetBloomThreshold(float threshold)
{
	m_bloomThreshold = threshold;
	auto rasterC = GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rasterC.valid())
		rasterC->SetBloomThreshold(threshold);
}
float CRendererPpBloomComponent::GetBloomThreshold() const { return m_bloomThreshold; }

void CRendererPpBloomComponent::SetPipelineDirty()
{
	m_pipelineDirty = true;
	m_bloomPipelineInfoH = nullptr;
	m_bloomPipelineInfoV = nullptr;
	SetTickPolicy(pragma::TickPolicy::Always);
}

void CRendererPpBloomComponent::OnTick(double dt)
{
	CRendererPpBaseComponent::OnTick(dt);
	SetTickPolicy(TickPolicy::Never);
	if(!m_pipelineDirty)
		return;
	m_pipelineDirty = false;
	c_engine->GetRenderContext().WaitIdle(true);
	init_shaders();
	if(g_bloomBlurH.valid()) {
		m_bloomPipelineInfoH = static_cast<ShaderPPBloomBlurBase *>(g_bloomBlurH.get())->AddPipeline(m_radius, m_sigma);
		g_bloomBlurH->ReloadPipelines();
	}
	if(g_bloomBlurV.valid()) {
		m_bloomPipelineInfoV = static_cast<ShaderPPBloomBlurBase *>(g_bloomBlurV.get())->AddPipeline(m_radius, m_sigma);
		g_bloomBlurV->ReloadPipelines();
	}
}
