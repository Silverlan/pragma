/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include "pragma/rendering/shaders/post_processing/c_shader_pp_bloom_blur.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <shader/prosper_shader_t.hpp>
#include <pragma/util/util_game.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

using namespace pragma;

pragma::BloomPipelineInfo::BloomPipelineInfo(uint32_t radius, double sigma) : radius(radius), sigma(sigma) {}

ShaderPPBloomBlurBase::ShaderPPBloomBlurBase(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader) : prosper::ShaderBlurBase(context, identifier, fsShader), m_defaultPipeline {AddPipeline(DEFAULT_RADIUS, DEFAULT_SIGMA)} {}

std::shared_ptr<BloomPipelineInfo> ShaderPPBloomBlurBase::AddPipeline(uint32_t radius, double sigma)
{
	for(auto &info : m_pipelineInfos) {
		if(info.expired())
			continue;
		auto pInfo = info.lock();
		if(pInfo->radius == radius && pInfo->sigma == sigma)
			return pInfo;
	}
	auto info = std::make_shared<BloomPipelineInfo>(radius, sigma);
	m_pipelineInfos.push_back(info);
	return info;
}

void ShaderPPBloomBlurBase::OnInitializePipelines()
{
	prosper::ShaderBlurBase::OnInitializePipelines();
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

void ShaderPPBloomBlurBase::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { CreateCachedRenderPass<ShaderPPBloomBlurBase>({{{pragma::ShaderGameWorldLightingPass::RENDER_PASS_FORMAT}}}, outRenderPass, pipelineIdx); }

void ShaderPPBloomBlurBase::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	prosper::ShaderBlurBase::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

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
