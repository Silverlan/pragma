/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/shaders/image/c_shader_resize_image.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_copy_image.hpp>
#include <shader/prosper_shader_t.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_texture.hpp>
#include <random>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;
ShaderResizeImage::ShaderResizeImage(prosper::IPrContext &context, const std::string &identifier) : prosper::ShaderBaseImageProcessing(context, identifier, "programs/image/resize_image") { SetPipelineCount(umath::to_integral(Filter::Count) * umath::to_integral(RenderPass::Count)); }

ShaderResizeImage::~ShaderResizeImage() {}

ShaderResizeImage::Filter ShaderResizeImage::GetFilter(uint32_t pipelineIdx) const { return static_cast<Filter>(pipelineIdx % umath::to_integral(Filter::Count)); }
ShaderResizeImage::RenderPass ShaderResizeImage::GetRenderPassType(uint32_t pipelineIdx) const { return static_cast<RenderPass>(pipelineIdx / umath::to_integral(Filter::Count)); }
std::optional<ShaderResizeImage::RenderPass> ShaderResizeImage::GetRenderPassType(prosper::Format format) const
{
	switch(format) {
	case prosper::Format::R8G8B8A8_UNorm:
		return RenderPass::R8G8B8A8;
	case prosper::Format::R16G16B16A16_SFloat:
		return RenderPass::R16G16B16A16;
	default:
		return std::nullopt;
	}
}
prosper::Format ShaderResizeImage::GetFormat(uint32_t pipelineIdx) const
{
	switch(GetRenderPassType(pipelineIdx)) {
	case RenderPass::R8G8B8A8:
		return prosper::Format::R8G8B8A8_UNorm;
	case RenderPass::R16G16B16A16:
		return prosper::Format::R16G16B16A16_SFloat;
	default:
		return prosper::Format::Unknown;
	}
}
uint32_t ShaderResizeImage::GetPipelineIndex(Filter filter, RenderPass renderPass) const { return umath::to_integral(filter) + umath::to_integral(renderPass) * umath::to_integral(Filter::Count); }

void ShaderResizeImage::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) { CreateCachedRenderPass<ShaderResizeImage>({{prosper::util::RenderPassCreateInfo::AttachmentInfo {GetFormat(pipelineIdx)}}}, outRenderPass, pipelineIdx); }

void ShaderResizeImage::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderBaseImageProcessing::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	AddSpecializationConstant(pipelineInfo, prosper::ShaderStageFlags::FragmentBit, 0u /* constantId */, static_cast<uint32_t>(GetFilter(pipelineIdx)));
}

void ShaderResizeImage::InitializeShaderResources()
{
	ShaderBaseImageProcessing::InitializeShaderResources();

	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderResizeImage::RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTexture, const BicubicFilter &bicubicFilter, prosper::Format format) const
{
	prosper::ShaderBindState bindState {cmd};
	auto rpType = GetRenderPassType(format);
	if(!rpType)
		return false;
	if(RecordBeginDraw(bindState, GetPipelineIndex(Filter::Bicubic, *rpType)) == false)
		return false;
	PushConstants pushConstants {};
	auto res = RecordDraw(bindState, descSetTexture, pushConstants);
	RecordEndDraw(bindState);
	return res;
}
bool ShaderResizeImage::RecordDraw(prosper::ICommandBuffer &cmd, prosper::IDescriptorSet &descSetTexture, const LanczosFilter &lanczosFilter, prosper::Format format) const
{
	prosper::ShaderBindState bindState {cmd};
	auto rpType = GetRenderPassType(format);
	if(!rpType)
		return false;
	if(RecordBeginDraw(bindState, GetPipelineIndex(Filter::Lanczos, *rpType)) == false)
		return false;

	constexpr std::array<Vector4, 2> aaKernel {
	  Vector4 {0.44031130485056913, 0.29880437751590694, 0.04535643028360444, -0.06431646022479595}, // x2
	  Vector4 {0.2797564513818748, 0.2310717037833796, 0.11797652759318597, 0.01107354293249700}     // x4
	};
	auto idx = umath::get_least_significant_set_bit_index(umath::to_integral(lanczosFilter.scale)) - 1;
	PushConstants pushConstants {};
	pushConstants.fparam = aaKernel[idx];
	auto res = RecordDraw(bindState, descSetTexture, pushConstants);
	RecordEndDraw(bindState);
	return res;
}

bool ShaderResizeImage::RecordDraw(prosper::ShaderBindState &bindState, prosper::IDescriptorSet &descSetTexture, const PushConstants &pushConstants) const { return RecordPushConstants(bindState, pushConstants) && prosper::ShaderBaseImageProcessing::RecordDraw(bindState, descSetTexture); }
