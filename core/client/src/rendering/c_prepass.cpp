/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/c_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/gui/widebugmsaatexture.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include <wgui/types/wirect.h>
#include <image/prosper_render_target.hpp>
#include <prosper_util.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

bool pragma::rendering::Prepass::Initialize(prosper::IPrContext &context,uint32_t width,uint32_t height,prosper::SampleCountFlags samples,bool bExtended)
{
	m_shaderPrepass = context.GetShader("prepass");
	m_shaderPrepassDepth = context.GetShader("prepass_depth");

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = ShaderTextured3DBase::RENDER_PASS_DEPTH_FORMAT;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.samples = samples;
	imgCreateInfo.usage = prosper::ImageUsageFlags::DepthStencilAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::SampledBit;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::DepthStencilAttachmentOptimal;
	auto imgDepth = context.CreateImage(imgCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = {};
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	samplerCreateInfo.addressModeW = prosper::SamplerAddressMode::ClampToEdge;
	textureDepth = context.CreateTexture(texCreateInfo,*imgDepth,imgViewCreateInfo,samplerCreateInfo);

	if(textureDepth->IsMSAATexture())
		textureDepthSampled = static_cast<prosper::MSAATexture&>(*textureDepth).GetResolvedTexture();
	else
	{
		imgCreateInfo.usage = prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::SampledBit;
		imgCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
		imgCreateInfo.samples = prosper::SampleCountFlags::e1Bit;
		auto imgDepthSampled = context.CreateImage(imgCreateInfo);

		texCreateInfo.flags = {};
		textureDepthSampled = context.CreateTexture(texCreateInfo,*imgDepthSampled,imgViewCreateInfo,samplerCreateInfo);
	}

	SetUseExtendedPrepass(bExtended,true);
	return true;
}

pragma::ShaderPrepassBase &pragma::rendering::Prepass::GetShader() const
{
	return (m_bExtended == true) ? static_cast<pragma::ShaderPrepassBase&>(*m_shaderPrepass.get()) : static_cast<pragma::ShaderPrepassBase&>(*m_shaderPrepassDepth.get());
}

bool pragma::rendering::Prepass::IsExtended() const {return m_bExtended;}
void pragma::rendering::Prepass::SetUseExtendedPrepass(bool b,bool bForceReload)
{
	if(m_bExtended == b && bForceReload == false)
		return;
	m_bExtended = b;

	auto &context = c_engine->GetRenderContext();
	context.WaitIdle();

	auto &imgDepth = textureDepth->GetImage();
	auto extents = imgDepth.GetExtents();
	auto width = extents.width;
	auto height = extents.height;

	auto whShaderPrepassDepth = c_engine->GetShader("prepass_depth");
	auto whShaderPrepass = c_engine->GetShader("prepass");
	if(whShaderPrepass.expired() || whShaderPrepassDepth.expired())
		return;

	auto *shaderPrepassDepth = static_cast<pragma::ShaderPrepassDepth*>(whShaderPrepassDepth.get());
	auto *shaderPrepass = static_cast<pragma::ShaderPrepass*>(whShaderPrepass.get());
	auto sampleCount = imgDepth.GetSampleCount();
	auto pipelineType = pragma::ShaderPrepassBase::GetPipelineIndex(sampleCount);
	if(b == true)
	{
		prosper::util::ImageCreateInfo imgCreateInfo {};
		imgCreateInfo.samples = imgDepth.GetSampleCount();
		imgCreateInfo.format = ShaderPrepass::RENDER_PASS_NORMAL_FORMAT;
		imgCreateInfo.width = width;
		imgCreateInfo.height = height;
		imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit;
		auto imgNormals = context.CreateImage(imgCreateInfo);

		prosper::util::TextureCreateInfo texCreateInfo {};
		texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
		samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
		samplerCreateInfo.addressModeW = prosper::SamplerAddressMode::ClampToEdge;
		textureNormals = context.CreateTexture(texCreateInfo,*imgNormals,imgViewCreateInfo,samplerCreateInfo);

		auto &imgDepth = textureDepth->GetImage();
		renderTarget = context.CreateRenderTarget({textureNormals,textureDepth},shaderPrepass->GetRenderPass(umath::to_integral(pipelineType)));
		renderTarget->SetDebugName("prepass_depth_normal_rt");
		m_clearValues = {
			prosper::ClearValue{prosper::ClearColorValue{}}, // Unused, but required
			prosper::ClearValue{prosper::ClearDepthStencilValue{1.f,0}} // Clear depth
		};
	}
	else
	{
		textureNormals = nullptr;

		renderTarget = context.CreateRenderTarget({textureDepth},shaderPrepassDepth->GetRenderPass(umath::to_integral(pipelineType)));
		renderTarget->SetDebugName("prepass_depth_rt");
		m_clearValues = {
			prosper::ClearValue{prosper::ClearDepthStencilValue{1.f,0}} // Clear depth
		};
	}
	prosper::util::RenderPassCreateInfo rpInfo {{
		pragma::ShaderPrepass::get_normal_render_pass_attachment_info(sampleCount),
		pragma::ShaderPrepass::get_depth_render_pass_attachment_info(sampleCount)
	}};
	for(auto &att : rpInfo.attachments)
		att.loadOp = prosper::AttachmentLoadOp::Load;
	subsequentRenderPass = c_engine->GetRenderContext().CreateRenderPass(rpInfo);
}

void pragma::rendering::Prepass::BeginRenderPass(const util::DrawSceneInfo &drawSceneInfo,prosper::IRenderPass *optRenderPass)
{
	// prosper TODO: Barriers for imgDepth and imgNormals
	drawSceneInfo.commandBuffer->RecordBeginRenderPass(*renderTarget,m_clearValues,optRenderPass);
}
void pragma::rendering::Prepass::EndRenderPass(const util::DrawSceneInfo &drawSceneInfo)
{
	drawSceneInfo.commandBuffer->RecordEndRenderPass();
}

void Console::commands::debug_prepass(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	auto &wgui = WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	if(c_game == nullptr || argv.empty() || pRoot == nullptr)
		return;
	const std::string name = "debug_ssao";
	auto *pEl = pRoot->FindDescendantByName(name);
	auto v = util::to_int(argv.front());
	if(v == 0)
	{
		if(pEl != nullptr)
			pEl->Remove();
		return;
	}
	if(pEl != nullptr)
		return;
	pEl = wgui.Create<WIBase>();
	if(pEl == nullptr)
		return;
	pEl->SetName(name);

	auto *scene = c_game->GetScene();
	auto *renderer = scene ? scene->GetRenderer() : nullptr;
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto *rasterizer = static_cast<pragma::rendering::RasterizationRenderer*>(renderer);
	auto &ssaoInfo = rasterizer->GetSSAOInfo();
	auto &prepass = rasterizer->GetPrepass();

	auto bExtended = prepass.IsExtended();
	auto xOffset = 0u;
	if(prepass.textureNormals != nullptr)
	{
		auto *pNormals = wgui.Create<WITexturedRect>(pEl);
		if(pNormals != nullptr)
		{
			pNormals->SetX(xOffset);
			pNormals->SetSize(256,256);
			pNormals->SetTexture(*prepass.textureNormals);
			pNormals->Update();
			xOffset += 256;
		}
	}
	auto *pPrepassDepth = wgui.Create<WIDebugDepthTexture>(pEl);
	if(pPrepassDepth != nullptr)
	{
		pPrepassDepth->SetX(xOffset);
		pPrepassDepth->SetSize(256,256);
		pPrepassDepth->SetTexture(*prepass.textureDepth);
		pPrepassDepth->Update();
		xOffset += 256;
	}

	pEl->SizeToContents();
}
