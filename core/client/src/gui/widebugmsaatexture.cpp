#include "stdafx_client.h"
#include "pragma/gui/widebugmsaatexture.hpp"
#include "pragma/rendering/shaders/debug/c_shader_debug_multisample_image.hpp"
#include <image/prosper_msaa_texture.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(widebugmsaatexture,WIDebugMSAATexture);

WIDebugMSAATexture::WIDebugMSAATexture()
	: WIBase()
{}

void WIDebugMSAATexture::Initialize()
{
	m_hTextureRect = CreateChild<WITexturedRect>();
	m_hTextureRect->SetAutoAlignToParent(true);
}

void WIDebugMSAATexture::SetTexture(prosper::Texture &texture)
{
	m_msaaTexture = texture.shared_from_this();
	UpdateResolvedTexture();
	if(m_hTextureRect.IsValid() && m_resolvedTexture != nullptr)
		m_hTextureRect.get<WITexturedRect>()->SetTexture(*m_resolvedTexture);
}

void WIDebugMSAATexture::UpdateResolvedTexture()
{
	m_resolvedTexture = nullptr;
	if(m_msaaTexture == nullptr)
		return;
	if(m_msaaTexture->GetImage()->GetSampleCount() == Anvil::SampleCountFlagBits::_1_BIT)
	{
		m_resolvedTexture = m_msaaTexture;
		static_cast<WITexturedRect&>(*m_hTextureRect.get()).SetTexture(*m_resolvedTexture);
		return;
	}
	if(m_msaaTexture->IsMSAATexture())
	{
		m_resolvedTexture = static_cast<prosper::MSAATexture&>(*m_msaaTexture).GetResolvedTexture();
		// prosper TODO: Use custom shader
		return;
	}
	auto &msaaImg = m_msaaTexture->GetImage();
	auto extents = msaaImg->GetExtents();
	auto &context = *c_engine;
	auto &dev = context.GetDevice();
	prosper::util::ImageCreateInfo resolvedImgCreateInfo {};
	resolvedImgCreateInfo.width = extents.width;
	resolvedImgCreateInfo.height = extents.height;
	resolvedImgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	resolvedImgCreateInfo.format = msaaImg->GetFormat();
	resolvedImgCreateInfo.postCreateLayout = Anvil::ImageLayout::TRANSFER_DST_OPTIMAL;
	resolvedImgCreateInfo.usage = Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	auto resolvedImg = prosper::util::create_image(dev,resolvedImgCreateInfo);
	prosper::util::SamplerCreateInfo resolvedSamplerCreateInfo {};
	m_resolvedTexture = prosper::util::create_texture(dev,{},resolvedImg,nullptr,&resolvedSamplerCreateInfo);
}

void WIDebugMSAATexture::SetShouldResolveImage(bool b)
{
	if(m_bShouldResolveImage == b)
		return;
	m_bShouldResolveImage = b;
	UpdateResolvedTexture();
}

/*
void WIDebugMSAATexture::SetResolveImage(bool b)
{
	if(m_bResolveImage == b)
		return;
	m_bResolveImage = b;
	if(b == true)
	{
		m_renderTarget = nullptr;
		m_descSetImg = nullptr;
		return;
	}
	m_singleSampleTexture = nullptr;
	auto &context = c_engine->GetRenderContext();
	m_renderTarget = Vulkan::RenderTarget::Create(context,1024,1024,Anvil::Format::R8G8B8A8_UNORM,false,[](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
		info.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT;
	}); // Vulkan TODO: Size?
	if(m_hTexture.IsValid())
	{
		auto &tex = *m_hTexture.get();
		static auto hShaderMs = c_engine->GetShader("debug_multisample_image");
		if(hShaderMs.IsValid())
		{
			auto &shaderMs = static_cast<Shader::DebugMultisampleImage&>(*hShaderMs.get());
			if(shaderMs.GenerateDescriptorSet(umath::to_integral(Shader::DebugMultisampleImage::DescSet::Texture),m_descSetImg) == true)
				m_descSetImg->Update(&tex);
		}
	}
	m_renderTarget->GetTexture()->GetImage()->SetSetupLayout(vk::ImageLayout::eGeneral);
	if(m_hTextureRect.IsValid())
		m_hTextureRect.get<WITexturedRect>()->SetTexture(m_renderTarget->GetTexture());
}*/ // prosper TODO

void WIDebugMSAATexture::DoUpdate()
{
	WIBase::DoUpdate();
	if(!m_hTextureRect.IsValid())
		return;
	auto &context = *c_engine;

	
	/*auto hThis = GetHandle();
	auto hDepthTexture = m_hTexture;
	context.ScheduleDrawCommand([this,hThis,hDepthTexture](const Vulkan::Context&,const Vulkan::CommandBuffer &drawCmd) {
		if(!hThis.IsValid() || !hDepthTexture.IsValid() || m_hTexture.IsValid() == false)
			return;
		if(m_renderTarget != nullptr && m_descSetImg != nullptr)
		{
			static auto hShaderMs = c_engine->GetShader("debug_multisample_image");
			if(hShaderMs.IsValid())
			{
				m_renderTarget->GetTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
				hDepthTexture.get()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
				auto &shaderMs = static_cast<Shader::DebugMultisampleImage&>(*hShaderMs.get());
				drawCmd->BeginRenderPass(m_renderTarget);
				drawCmd->SetViewport(m_renderTarget->GetWidth(),m_renderTarget->GetHeight());
					if(shaderMs.BeginDraw(drawCmd) == true)
					{
						shaderMs.Draw(drawCmd,m_descSetImg,m_renderTarget->GetWidth(),m_renderTarget->GetHeight(),umath::to_integral(m_hTexture->GetImage()->GetSampleCount()));
						shaderMs.EndDraw();
					}
				drawCmd->EndRenderPass();
				m_renderTarget->GetTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			}
			return;
		}
		if(m_singleSampleTexture)
			drawCmd->ResolveImage(m_hTexture->GetImage(),m_singleSampleTexture->GetImage());
		auto &targetTexture = GetSamplerTexture()->GetImage();
		targetTexture->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	});*/ // prosper TODO
}
