// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.shaders.pp_bloom_blur;
import :client_state;

using namespace pragma;

rendering::BloomPipelineInfo::BloomPipelineInfo(uint32_t radius, double sigma) : radius(radius), sigma(sigma) {}

ShaderPPBloomBlurBase::ShaderPPBloomBlurBase(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader) : ShaderBlurBase(context, identifier, fsShader), m_defaultPipeline {AddPipeline(DEFAULT_RADIUS, DEFAULT_SIGMA)} {}

std::shared_ptr<rendering::BloomPipelineInfo> ShaderPPBloomBlurBase::AddPipeline(uint32_t radius, double sigma)
{
	for(auto &info : m_pipelineInfos) {
		if(info.expired())
			continue;
		auto pInfo = info.lock();
		if(pInfo->radius == radius && pInfo->sigma == sigma)
			return pInfo;
	}
	auto info = pragma::util::make_shared<rendering::BloomPipelineInfo>(radius, sigma);
	m_pipelineInfos.push_back(info);
	return info;
}

void ShaderPPBloomBlurBase::OnInitializePipelines()
{
	ShaderBlurBase::OnInitializePipelines();
	for(auto it = m_pipelineInfos.begin(); it != m_pipelineInfos.end();) {
		auto &info = *it;
		if(info.expired()) {
			it = m_pipelineInfos.erase(it);
			continue;
		}
		++it;
	}
	SetPipelineCount(m_pipelineInfos.size(), false);
}

void ShaderPPBloomBlurBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { CreateCachedRenderPass<ShaderPPBloomBlurBase>({{{ShaderGameWorldLightingPass::RENDER_PASS_FORMAT}}}, outRenderPass, pipelineIdx); }

void ShaderPPBloomBlurBase::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderBlurBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	assert(pipelineIdx < m_pipelineInfos.size());
	auto &info = m_pipelineInfos[pipelineIdx];
	assert(!info.expired());
	if(info.expired())
		return;
	auto pInfo = info.lock();
	pInfo->pipelineIdx = pipelineIdx;
	auto coefficients = util::generate_two_pass_gaussian_blur_coefficients(pInfo->radius, pInfo->sigma);

	auto numSamples = coefficients.first.size();
	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, 0u /* constantId */, static_cast<int32_t>(numSamples));

	std::vector<float> offsets;
	std::vector<float> weights;
	offsets.resize(coefficients.first.size());
	weights.resize(coefficients.second.size());
	for(auto i = decltype(offsets.size()) {0u}; i < offsets.size(); ++i)
		offsets[i] = static_cast<float>(coefficients.first[i]);
	for(auto i = decltype(weights.size()) {0u}; i < weights.size(); ++i)
		weights[i] = static_cast<float>(coefficients.second[i]);

	for(auto i = decltype(numSamples) {0}; i < numSamples; ++i) {
		AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, 1u + i, offsets[i]);
		AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, 1u + MAX_SAMPLE_COUNT + i, weights[i]);
	}
}

ShaderPPBloomBlurH::ShaderPPBloomBlurH(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBloomBlurBase(context, identifier, "programs/effects/gaussianblur_horizontal") {}
ShaderPPBloomBlurH::~ShaderPPBloomBlurH() {}

ShaderPPBloomBlurV::ShaderPPBloomBlurV(prosper::IPrContext &context, const std::string &identifier) : ShaderPPBloomBlurBase(context, identifier, "programs/effects/gaussianblur_vertical") { SetBaseShader<ShaderPPBloomBlurH>(); }
ShaderPPBloomBlurV::~ShaderPPBloomBlurV() {}
