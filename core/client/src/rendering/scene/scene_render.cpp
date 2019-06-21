#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include "pragma/rendering/shaders/image/c_shader_additive.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>


// TODO: Remove this file
/*
void Scene::BeginRenderPass(Vulkan::CommandBufferObject *drawCmd,const Color *clearColor)
{
	auto &hdrInfo = GetHDRInfo();
	hdrInfo.texture->Reset();
	hdrInfo.textureBloom->Reset();

	hdrInfo.texture->GetImage()->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	hdrInfo.prepass.textureDepth->GetImage()->SetDrawLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	hdrInfo.textureBloom->GetImage()->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);

	vk::ClearColorValue clearCol {};
	if(clearColor != nullptr)
	{
		clearCol.float32[0] = clearColor->r /255.f;
		clearCol.float32[1] = clearColor->g /255.f;
		clearCol.float32[2] = clearColor->b /255.f;
		clearCol.float32[3] = clearColor->a /255.f;
	}
	drawCmd->BeginRenderPass(hdrInfo.renderPass,hdrInfo.framebuffer,GetWidth(),GetHeight(),{
		clearCol, // Color Attachment
		vk::ClearValue{vk::ClearColorValue{std::array<float,4>{0.f,0.f,0.f,1.f}}}, // Bloom Attachment
		vk::ClearColorValue{}, // Position Attachment
		vk::ClearColorValue{}, // Normal Attachment
		vk::ClearValue{vk::ClearDepthStencilValue{1.f,0}} // Depth Attachment
	});
	auto &img = hdrInfo.texture->GetImage();
	if(clearColor != nullptr)
		drawCmd->ClearAttachment(0,img);
}
void Scene::EndRenderPass(Vulkan::CommandBufferObject *drawCmd) {drawCmd->EndRenderPass();}
void Scene::Present(Vulkan::CommandBufferObject *drawCmd,const Vulkan::RenderPassObject *renderPass,const Vulkan::FramebufferObject *framebuffer)
{
	// Blur HDR Bloom
	auto &hdrInfo = GetHDRInfo();

	auto &renderTexture = hdrInfo.texture->Resolve();
	auto &bloomTexture = hdrInfo.textureBloom->Resolve();
	hdrInfo.cmdBufferBloom(drawCmd);
	hdrInfo.blurBuffer.tmpBlurTexture->GetImage()->SetInternalLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

	auto &glowInfo = GetGlowInfo();
	//auto &rt = GetRenderTarget();
	//auto &tex = rt->GetTexture();
	//auto &img = tex->GetImage();
	auto w = GetWidth();//img->GetWidth();
	auto h = GetHeight();//img->GetHeight();

	// Render to screen
	static auto hShaderScreen = ShaderSystem::get_shader("pp_hdr");
	if(hShaderScreen.IsValid() == true)
	{
		auto &shaderScreen = static_cast<Shader::PPHDR&>(*hShaderScreen.get());

		auto &inTex = hdrInfo.GetInputTexture();
		inTex->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

		auto &setTexture = hdrInfo.GetInputDescriptorSet();
		//img->SetDrawLayout(vk::ImageLayout::ePresentSrcKHR); // TODO: Only when presenting to swapchain
		drawCmd->BeginRenderPass(renderPass,framebuffer,w,h);

			if(shaderScreen.BeginDraw() == true)
			{
				const float bloomAdditiveScale = 0.5f;
				auto glowScale = (glowInfo.bGlowScheduled == true) ? 1.f : 0.f;
				shaderScreen.Draw(drawCmd,setTexture,GetBloomGlowDescriptorSet(),GetHDRExposure(),bloomAdditiveScale,glowScale);
				shaderScreen.EndDraw();
			}

		drawCmd->EndRenderPass();
	}
	hdrInfo.ResetIOTextureIndex();
}
void Scene::Present(Vulkan::CommandBufferObject *drawCmd)
{
	auto &rt = GetRenderTarget();
	if(rt == nullptr)
	{
		Con::cwar<<"WARNING: Attempted to present scene to invalid render target! Ignoring..."<<Con::endl;
		return;
	}
	Present(drawCmd,rt->GetRenderPass(),rt->GetFramebuffer());
}
*/ // prosper TODO