// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <shader/prosper_shader_blur.hpp>

module pragma.client;

import :rendering.controlled_blur_settings;

import :engine;

static util::WeakHandle<prosper::Shader> g_bloomBlurH {};
static util::WeakHandle<prosper::Shader> g_bloomBlurV {};
static void init_shaders()
{
	if(g_bloomBlurH.expired())
		g_bloomBlurH = pragma::get_cengine()->GetShader("pp_bloom_blur_h");
	if(g_bloomBlurV.expired())
		g_bloomBlurV = pragma::get_cengine()->GetShader("pp_bloom_blur_v");
}

pragma::ControlledBlurSettings::ControlledBlurSettings() { init_shaders(); }
void pragma::ControlledBlurSettings::SetRadius(uint32_t radius)
{
	radius = umath::clamp(radius, 0u, MAX_BLUR_RADIUS);
	m_radius = radius;
	SetShaderPipelineDirty();
}
void pragma::ControlledBlurSettings::SetSigma(double sigma)
{
	sigma = umath::clamp(sigma, 0.0, MAX_BLUR_SIGMA);
	m_sigma = sigma;
	SetShaderPipelineDirty();
}
uint32_t pragma::ControlledBlurSettings::GetRadius() const { return m_radius; }
double pragma::ControlledBlurSettings::GetSigma() const { return m_sigma; }

void pragma::ControlledBlurSettings::SetBlurAmount(int32_t blurAmount) { m_blurAmount = blurAmount; }
int32_t pragma::ControlledBlurSettings::GetBlurAmount() const { return m_blurAmount; }

bool pragma::ControlledBlurSettings::IsValid() const
{
	if(!m_bloomPipelineInfoH || !m_bloomPipelineInfoH->pipelineIdx || !m_bloomPipelineInfoV || !m_bloomPipelineInfoV->pipelineIdx)
		return false;
	return true;
}

void pragma::ControlledBlurSettings::SetShaderPipelineDirty()
{
	m_pipelineDirty = true;
	m_bloomPipelineInfoH = nullptr;
	m_bloomPipelineInfoV = nullptr;
}

void pragma::ControlledBlurSettings::UpdateShaderPipelines()
{
	pragma::get_cengine()->GetRenderContext().WaitIdle(true);
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

static auto cvBloomAmount = GetClientConVar("render_bloom_amount");
void pragma::ControlledBlurSettings::RecordBlur(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmd, prosper::BlurSet &blurSet)
{
	static auto blurSize = 5.f;
	static int32_t kernelSize = 9u;
	uint32_t blurAmount = umath::clamp(m_blurAmount >= 0 ? m_blurAmount : cvBloomAmount->GetInt(), 0, 20);

	prosper::util::ShaderInfo shaderInfo {};
	shaderInfo.shaderH = static_cast<prosper::ShaderBlurBase *>(g_bloomBlurH.get());
	shaderInfo.shaderHPipeline = *m_bloomPipelineInfoH->pipelineIdx;
	shaderInfo.shaderV = static_cast<prosper::ShaderBlurBase *>(g_bloomBlurV.get());
	shaderInfo.shaderVPipeline = *m_bloomPipelineInfoV->pipelineIdx;

	for(auto i = decltype(blurAmount) {0}; i < blurAmount; ++i) {
		prosper::util::record_blur_image(pragma::get_cengine()->GetRenderContext(), cmd, blurSet, {Vector4(1.f, 1.f, 1.f, 1.f), blurSize, kernelSize}, 1u, &shaderInfo);
	}
}
