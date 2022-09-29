/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/rasterization/glow_data.hpp"
#include "pragma/entities/components/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include <prosper_util.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <shader/prosper_shader_t.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_sampler.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

using namespace pragma::rendering;

GlowData::GlowData()
	: bGlowScheduled(false)
{
	shader = c_engine->GetShader("glow");
}
GlowData::~GlowData()
{
	if(m_cbReloadCommandBuffer.IsValid())
		m_cbReloadCommandBuffer.Remove();
}
bool GlowData::Initialize(uint32_t width,uint32_t height,const HDRData &hdrInfo)
{
	auto depthTex = hdrInfo.prepass.textureDepth;
	if(depthTex->IsMSAATexture())
		depthTex = static_cast<prosper::MSAATexture&>(*depthTex).GetResolvedTexture();

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.format = pragma::ShaderGlow::RENDER_PASS_FORMAT;
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferSrcBit; // Note: Transfer flag required for debugging purposes only (See debug_glow_bloom console command)
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	auto img = c_engine->GetRenderContext().CreateImage(imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	auto tex = c_engine->GetRenderContext().CreateTexture({},*img,imgViewCreateInfo,samplerCreateInfo);
	renderTarget = c_engine->GetRenderContext().CreateRenderTarget({tex,depthTex},prosper::ShaderGraphics::GetRenderPass<pragma::ShaderGlow>(c_engine->GetRenderContext()));
	renderTarget->SetDebugName("glow_rt");

	auto rtBlur = c_engine->GetRenderContext().CreateRenderTarget({tex},prosper::ShaderGraphics::GetRenderPass<prosper::ShaderBlurBase>(c_engine->GetRenderContext(),umath::to_integral(prosper::ShaderBlurBase::Pipeline::R8G8B8A8Unorm)));
	rtBlur->SetDebugName("glow_blur_rt");
	blurSet = prosper::BlurSet::Create(c_engine->GetRenderContext(),rtBlur);
	return true;
}
