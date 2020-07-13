/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/rendering/shaders/c_shader_depth_to_rgb.h"
#include <image/prosper_render_target.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(widebugdepthtexture,WIDebugDepthTexture);
#pragma optimize("",off)
WIDebugDepthTexture::WIDebugDepthTexture()
	: WIBase(),m_imageLayer(0)
{
	m_whDepthToRgbShader = c_engine->GetShader("debug_depth_to_rgb");
	m_whCubeDepthToRgbShader = c_engine->GetShader("debug_cube_depth_to_rgb");
	m_whCsmDepthToRgbShader = c_engine->GetShader("debug_csm_depth_to_rgb");
}

WIDebugDepthTexture::~WIDebugDepthTexture()
{
	if(m_depthToRgbCallback.IsValid())
		m_depthToRgbCallback.Remove();
}

void WIDebugDepthTexture::SetTexture(prosper::Texture &texture)
{
	SetTexture(texture,{
		prosper::PipelineStageFlags::LateFragmentTestsBit,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AccessFlags::DepthStencilAttachmentWriteBit
	},{
		prosper::PipelineStageFlags::EarlyFragmentTestsBit,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AccessFlags::DepthStencilAttachmentWriteBit
	});
}

void WIDebugDepthTexture::SetTexture(prosper::Texture &texture,prosper::util::BarrierImageLayout srcLayout,prosper::util::BarrierImageLayout dstLayout,uint32_t layerId)
{
	m_srcDepthTex = nullptr;

	if(m_whDepthToRgbShader.expired() || pragma::ShaderDepthToRGB::DESCRIPTOR_SET.IsValid() == false)
		return;
	auto &shader = static_cast<prosper::ShaderGraphics&>(*m_whDepthToRgbShader.get());
	auto &inputImg = texture.GetImage();
	auto extents = inputImg.GetExtents();
	auto &context = c_engine->GetRenderContext();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
	auto img = context.CreateImage(imgCreateInfo);
	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = context.CreateTexture(texCreateInfo,*img,imgViewCreateInfo,samplerCreateInfo);
	m_renderTarget = context.CreateRenderTarget({tex},shader.GetRenderPass());
	m_renderTarget->SetDebugName("debug_depth_rt");

	m_dsgSceneDepthTex = context.CreateDescriptorSetGroup(pragma::ShaderDepthToRGB::DESCRIPTOR_SET);

	imgViewCreateInfo = {};
	imgViewCreateInfo.baseLayer = layerId;
	imgViewCreateInfo.levelCount = 1u;//inputImg.get_image_n_layers();
	samplerCreateInfo = {};
	m_srcDepthTex = context.CreateTexture({},texture.GetImage(),imgViewCreateInfo,samplerCreateInfo);
	m_srcDepthTex->SetDebugName("debug_depth_src_rt");
	if(inputImg.GetLayerCount() == 1u)
		m_dsgSceneDepthTex->GetDescriptorSet()->SetBindingTexture(*m_srcDepthTex,0u);
	else
		m_dsgSceneDepthTex->GetDescriptorSet()->SetBindingArrayTexture(*m_srcDepthTex,0u,0u);

	if(m_hTextureRect.IsValid())
		static_cast<WITexturedRect&>(*m_hTextureRect.get()).SetTexture(m_renderTarget->GetTexture());
	m_srcBarrierImageLayout = srcLayout;
	m_dstBarrierImageLayout = dstLayout;
	m_imageLayer = layerId;
}

void WIDebugDepthTexture::UpdateResolvedTexture()
{

}

void WIDebugDepthTexture::SetShouldResolveImage(bool b)
{

}

void WIDebugDepthTexture::Initialize()
{
	m_hTextureRect = CreateChild<WITexturedRect>();
	m_hTextureRect->SetAutoAlignToParent(true);
}

void WIDebugDepthTexture::Setup(float nearZ,float farZ)
{
	if(m_depthToRgbCallback.IsValid())
		m_depthToRgbCallback.Remove();
	m_depthToRgbCallback = c_engine->AddCallback("DrawFrame",FunctionCallback<void,std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>::Create([this,nearZ,farZ](std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>> refDrawCmd) {
		auto &drawCmd = refDrawCmd.get();
		if(m_whDepthToRgbShader.expired() || m_srcDepthTex == nullptr || m_renderTarget == nullptr || m_dsgSceneDepthTex == nullptr)
			return;
		auto &img = m_renderTarget->GetTexture().GetImage();
		drawCmd->RecordImageBarrier(img,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
		auto &depthImg = m_srcDepthTex->GetImage();
		drawCmd->RecordImageBarrier(
			depthImg,
			m_srcBarrierImageLayout.stageMask,prosper::PipelineStageFlags::FragmentShaderBit,
			m_srcBarrierImageLayout.layout,prosper::ImageLayout::ShaderReadOnlyOptimal,
			m_srcBarrierImageLayout.accessMask,prosper::AccessFlags::ShaderReadBit
		);
		if(drawCmd->RecordBeginRenderPass(*m_renderTarget) == true)
		{
			if(depthImg.IsCubemap())
			{
				auto &shader = static_cast<pragma::ShaderCubeDepthToRGB&>(*m_whCubeDepthToRgbShader.get());
				if(shader.BeginDraw(drawCmd) == true)
				{
					shader.Draw(*m_dsgSceneDepthTex->GetDescriptorSet(),nearZ,farZ,m_imageLayer,GetContrastFactor());
					shader.EndDraw();
				}
			}
			else if(img.GetLayerCount() > 1u)
			{
				auto &shader = static_cast<pragma::ShaderCSMDepthToRGB&>(*m_whCsmDepthToRgbShader.get());
				if(shader.BeginDraw(drawCmd) == true)
				{
					shader.Draw(*m_dsgSceneDepthTex->GetDescriptorSet(),nearZ,farZ,m_imageLayer,GetContrastFactor());
					shader.EndDraw();
				}
			}
			else
			{
				auto &shader = static_cast<pragma::ShaderDepthToRGB&>(*m_whDepthToRgbShader.get());
				if(shader.BeginDraw(drawCmd) == true)
				{
					shader.Draw(*m_dsgSceneDepthTex->GetDescriptorSet(),nearZ,farZ,GetContrastFactor());
					shader.EndDraw();
				}
			}
			drawCmd->RecordEndRenderPass();
		}
		drawCmd->RecordImageBarrier(
			depthImg,
			prosper::PipelineStageFlags::FragmentShaderBit,m_dstBarrierImageLayout.stageMask,
			prosper::ImageLayout::ShaderReadOnlyOptimal,m_dstBarrierImageLayout.layout,
			prosper::AccessFlags::ShaderReadBit,m_dstBarrierImageLayout.accessMask
		);
	}));
}

void WIDebugDepthTexture::SetContrastFactor(float contrastFactor) {m_contrastFactor = contrastFactor;}
float WIDebugDepthTexture::GetContrastFactor() const {return m_contrastFactor;}

void WIDebugDepthTexture::DoUpdate()
{
	auto *cam = c_game->GetPrimaryCamera();
	if(cam == nullptr)
		return;
	Setup(cam->GetNearZ(),cam->GetFarZ());
}
#pragma optimize("",on)
