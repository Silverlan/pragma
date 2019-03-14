#include "stdafx_client.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/rendering/shaders/c_shader_depth_to_rgb.h"
#include "pragma/rendering/shaders/debug/c_shader_debug_multisample_image.hpp"
#include <image/prosper_render_target.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

LINK_WGUI_TO_CLASS(widebugdepthtexture,WIDebugDepthTexture);

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
		Anvil::PipelineStageFlagBits::LATE_FRAGMENT_TESTS_BIT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AccessFlagBits::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	},{
		Anvil::PipelineStageFlagBits::EARLY_FRAGMENT_TESTS_BIT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AccessFlagBits::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	});
}

void WIDebugDepthTexture::SetTexture(prosper::Texture &texture,prosper::util::BarrierImageLayout srcLayout,prosper::util::BarrierImageLayout dstLayout,uint32_t layerId)
{
	m_srcDepthRenderTarget = nullptr;

	if(m_whDepthToRgbShader.expired() || pragma::ShaderDepthToRGB::DESCRIPTOR_SET.IsValid() == false)
		return;
	auto &shader = static_cast<prosper::ShaderGraphics&>(*m_whDepthToRgbShader.get());
	auto &inputImg = *texture.GetImage();
	auto extents = inputImg.GetExtents();
	auto &context = *c_engine;
	auto &dev = context.GetDevice();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = Anvil::Format::R8G8B8A8_UNORM;
	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;
	auto img = prosper::util::create_image(dev,imgCreateInfo);
	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = prosper::util::create_texture(dev,texCreateInfo,img,&imgViewCreateInfo,&samplerCreateInfo);
	m_renderTarget = prosper::util::create_render_target(dev,{tex},shader.GetRenderPass());
	m_renderTarget->SetDebugName("debug_depth_rt");

	m_descSetGroupDepthTex = prosper::util::create_descriptor_set_group(dev,pragma::ShaderDepthToRGB::DESCRIPTOR_SET);

	imgViewCreateInfo = {};
	imgViewCreateInfo.baseLayer = layerId;
	imgViewCreateInfo.levelCount = 1u;//inputImg.get_image_n_layers();
	samplerCreateInfo = {};
	auto texDepthSrc = prosper::util::create_texture(dev,{},texture.GetImage(),&imgViewCreateInfo,&samplerCreateInfo);
	m_srcDepthRenderTarget = prosper::util::create_render_target(dev,{texDepthSrc});
	m_srcDepthRenderTarget->SetDebugName("debug_depth_src_rt");
	if(inputImg.GetLayerCount() == 1u)
		prosper::util::set_descriptor_set_binding_texture(*(*m_descSetGroupDepthTex)->get_descriptor_set(0u),*m_srcDepthRenderTarget->GetTexture(),0u);
	else
		prosper::util::set_descriptor_set_binding_array_texture(*(*m_descSetGroupDepthTex)->get_descriptor_set(0u),*m_srcDepthRenderTarget->GetTexture(),0u,0u);

	if(m_hTextureRect.IsValid())
		static_cast<WITexturedRect&>(*m_hTextureRect.get()).SetTexture(*m_renderTarget->GetTexture());
	m_srcBarrierImageLayout = srcLayout;
	m_dstBarrierImageLayout = dstLayout;
	m_imageLayer = layerId;
}

void WIDebugDepthTexture::UpdateResolvedTexture()
{
	// prosper TODO
}

void WIDebugDepthTexture::SetShouldResolveImage(bool b)
{
	/*if(m_bShouldResolveImage == b)
		return;
	m_bShouldResolveImage = b;
	UpdateResolvedTexture();*/// prosper TODO
}

void WIDebugDepthTexture::Initialize()
{
	m_hTextureRect = CreateChild<WITexturedRect>();
	m_hTextureRect->SetAutoAlignToParent(true);

	/*auto &context = *WGUI::GetContext();
	m_renderTarget = Vulkan::RenderTarget::Create(context,1024,1024,Anvil::Format::R8G8B8A8_UNORM,false,[](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
		info.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT;
	}); // Vulkan TODO: Size?
	m_renderTarget->GetTexture()->GetImage()->SetSetupLayout(vk::ImageLayout::eGeneral);
	m_hTextureRect.get<WITexturedRect>()->SetTexture(m_renderTarget->GetTexture());
	static auto hShaderDepth = c_engine->GetShader("depthtorgb");
	if(!hShaderDepth.IsValid())
		return;
	static_cast<Shader::DepthToRGB&>(*hShaderDepth.get()).InitializeInstance(m_descTexture);
	auto texSampler = GetSamplerTexture();
	if(texSampler != nullptr && m_descTexture != nullptr)
		m_descTexture->Update(texSampler);*/// prosper TODO
}

// prosper TODO
/*void WIDebugDepthTexture::SetTexture(const Vulkan::MSAATexture &texture,uint32_t imgLayer)
{
	m_hTexture = texture->GetTexture()->GetHandle();
	m_singleSampleTexture = texture->GetResolvedTexture();

	m_imageLayer = 0;
	auto texSampler = GetSamplerTexture();
	if(texSampler == nullptr || m_descTexture == nullptr)
		return;
	m_descTexture->Update(texSampler,imgLayer);
	m_imageLayer = imgLayer;
}

void WIDebugDepthTexture::SetTexture(const Vulkan::Texture &texture,uint32_t imgLayer)
{
	m_singleSampleTexture = nullptr;
	m_imageLayer = 0;
	if(texture == nullptr)
	{
		m_hTexture = Vulkan::TextureHandle();
		return;
	}
	m_hTexture = texture->GetHandle();
	if(texture->GetSampleCount() != Anvil::SampleCountFlagBits::_1_BIT)
	{
		m_singleSampleTexture = Vulkan::Texture::Create(texture->GetContext(),texture->GetWidth(),texture->GetHeight(),texture->GetFormat(),[](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
			info.usage |= Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
		});
	}
	auto texSampler = GetSamplerTexture();
	if(texSampler == nullptr || m_descTexture == nullptr)
		return;
	m_descTexture->Update(texSampler,imgLayer);
	m_imageLayer = imgLayer;
}

void WIDebugDepthTexture::SetResolveImage(bool b)
{
	if(m_bResolveImage == b)
		return;
	m_bResolveImage = b;
	if(b == true)
	{
		m_renderTarget = nullptr;
		m_descTexture = nullptr;
		return;
	}
	m_singleSampleTexture = nullptr;
	auto &context = c_engine->GetRenderContext();
	if(m_hTexture.IsValid())
	{
		auto &tex = *m_hTexture.get();
		static auto hShaderMs = c_engine->GetShader("debug_multisample_depth");
		if(hShaderMs.IsValid())
		{
			auto &shaderMs = static_cast<Shader::DebugMultisampleDepth&>(*hShaderMs.get());
			if(shaderMs.GenerateDescriptorSet(umath::to_integral(Shader::DebugMultisampleDepth::DescSet::Texture),m_descTexture) == true)
				m_descTexture->Update(&tex);
		}
	}
}
*/
void WIDebugDepthTexture::Update(float nearZ,float farZ)
{
	WIBase::Update();
	if(m_depthToRgbCallback.IsValid())
		m_depthToRgbCallback.Remove();
	m_depthToRgbCallback = c_engine->AddCallback("DrawFrame",FunctionCallback<void,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>>::Create([this,nearZ,farZ](std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>> refDrawCmd) {
		auto &drawCmd = refDrawCmd.get();
		if(m_whDepthToRgbShader.expired() || m_srcDepthRenderTarget == nullptr || m_renderTarget == nullptr || m_descSetGroupDepthTex == nullptr)
			return;
		auto &img = *m_renderTarget->GetTexture()->GetImage();
		prosper::util::record_image_barrier(*(*drawCmd),*img,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		auto &depthImg = *m_srcDepthRenderTarget->GetTexture()->GetImage();
		prosper::util::record_image_barrier(
			*(*drawCmd),*depthImg,
			m_srcBarrierImageLayout.stageMask,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
			m_srcBarrierImageLayout.layout,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
			m_srcBarrierImageLayout.accessMask,Anvil::AccessFlagBits::SHADER_READ_BIT
		);
		if(prosper::util::record_begin_render_pass(*(*drawCmd),*m_renderTarget) == true)
		{
			if((depthImg.GetCreateFlags() &Anvil::ImageCreateFlagBits::CUBE_COMPATIBLE_BIT) != Anvil::ImageCreateFlagBits::NONE)
			{
				auto &shader = static_cast<pragma::ShaderCubeDepthToRGB&>(*m_whCubeDepthToRgbShader.get());
				if(shader.BeginDraw(drawCmd) == true)
				{
					shader.Draw(*(*m_descSetGroupDepthTex)->get_descriptor_set(0u),nearZ,farZ,m_imageLayer,GetContrastFactor());
					shader.EndDraw();
				}
			}
			else if(img.GetLayerCount() > 1u)
			{
				auto &shader = static_cast<pragma::ShaderCSMDepthToRGB&>(*m_whCsmDepthToRgbShader.get());
				if(shader.BeginDraw(drawCmd) == true)
				{
					shader.Draw(*(*m_descSetGroupDepthTex)->get_descriptor_set(0u),nearZ,farZ,m_imageLayer,GetContrastFactor());
					shader.EndDraw();
				}
			}
			else
			{
				auto &shader = static_cast<pragma::ShaderDepthToRGB&>(*m_whDepthToRgbShader.get());
				if(shader.BeginDraw(drawCmd) == true)
				{
					shader.Draw(*(*m_descSetGroupDepthTex)->get_descriptor_set(0u),nearZ,farZ,GetContrastFactor());
					shader.EndDraw();
				}
			}
			prosper::util::record_end_render_pass(*(*drawCmd));
		}
		prosper::util::record_image_barrier(
			*(*drawCmd),*depthImg,
			Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,m_dstBarrierImageLayout.stageMask,
			Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,m_dstBarrierImageLayout.layout,
			Anvil::AccessFlagBits::SHADER_READ_BIT,m_dstBarrierImageLayout.accessMask
		);
	}));
	/*if(m_renderTarget == nullptr || !m_hTextureRect.IsValid())
		return;
	auto &context = *WGUI::GetContext();
	auto hThis = GetHandle();
	auto hDepthTexture = m_hTexture;
	context.ScheduleDrawCommand([this,hThis,hDepthTexture,nearZ,farZ](const Vulkan::Context&,const Vulkan::CommandBuffer &drawCmd) {
		if(!hThis.IsValid() || !hDepthTexture.IsValid())
			return;
		static auto hShaderDepth = c_engine->GetShader("depthtorgb");
		static auto hShaderCubeDepth = c_engine->GetShader("cubedepthtorgb");
		static auto hShaderCsmDepth = c_engine->GetShader("csmdepthtorgb");
		if(!hShaderDepth.IsValid() || !hShaderCubeDepth.IsValid() || !hShaderCsmDepth.IsValid() || m_hTexture.IsValid() == false)
			return;
		auto &depthImg = GetSamplerTexture()->GetImage();
		auto bCubemap = depthImg->IsCubemap();
		auto bCsm = (bCubemap == false && depthImg->GetLayerCount() > 1) ? true : false;
		if(m_singleSampleTexture != nullptr)
			drawCmd->ResolveImage(m_hTexture->GetImage(),m_singleSampleTexture->GetImage());
		else if(m_bResolveImage == false && bCubemap == false && bCsm == false)
		{
			static auto hShaderMs = c_engine->GetShader("debug_multisample_depth");
			if(hShaderMs.IsValid())
			{
				m_renderTarget->GetTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
				hDepthTexture.get()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
				auto &shaderMs = static_cast<Shader::DebugMultisampleDepth&>(*hShaderMs.get());
				drawCmd->BeginRenderPass(m_renderTarget);
				drawCmd->SetViewport(m_renderTarget->GetWidth(),m_renderTarget->GetHeight());
					if(shaderMs.BeginDraw(drawCmd) == true)
					{
						shaderMs.Draw(drawCmd,m_descTexture,m_renderTarget->GetWidth(),m_renderTarget->GetHeight(),umath::to_integral(m_hTexture->GetImage()->GetSampleCount()));
						shaderMs.EndDraw();
					}
				drawCmd->EndRenderPass();
				m_renderTarget->GetTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			}
			return;
		}

		auto &el = static_cast<WIDebugDepthTexture&>(*hThis.get());
		depthImg->SetDrawLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);

		auto &renderImg = el.m_renderTarget->GetTexture()->GetImage();
		renderImg->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);

		auto *shader = (bCubemap == true) ? hShaderCubeDepth.get<Shader::DepthToRGB>() : ((bCsm == true) ? hShaderCsmDepth.get<Shader::DepthToRGB>() : hShaderDepth.get<Shader::DepthToRGB>());
		//auto &shaderDepth = static_cast<Shader::DepthToRGB&>(*hShaderDepth.get());
		//auto &shaderCubeDepth = static_cast<Shader::CubeDepthToRGB&>(*hShaderCubeDepth.get());

		drawCmd->BeginRenderPass(el.m_renderTarget);
		drawCmd->SetViewport(el.m_renderTarget->GetWidth(),el.m_renderTarget->GetHeight());
		auto *pipeline = shader->GetPipeline();
		if(
			(bCubemap == true && static_cast<Shader::CubeDepthToRGB*>(shader)->BeginDraw(drawCmd,pipeline,nearZ,farZ,el.m_imageLayer) == true) ||
			(bCsm == true && static_cast<Shader::CSMDepthToRGB*>(shader)->BeginDraw(drawCmd,pipeline,nearZ,farZ,el.m_imageLayer) == true) ||
			shader->BeginDraw(drawCmd,pipeline,nearZ,farZ)
		)
		{
			shader->Draw(el.m_descTexture);
			shader->EndDraw();
		}
		drawCmd->EndRenderPass();

		renderImg->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	});*/
}

void WIDebugDepthTexture::SetContrastFactor(float contrastFactor) {m_contrastFactor = contrastFactor;}
float WIDebugDepthTexture::GetContrastFactor() const {return m_contrastFactor;}

void WIDebugDepthTexture::Update()
{
	auto &scene = c_game->GetRenderScene();
	if(scene == nullptr)
		return;
	auto &cam = scene->camera;
	Update(cam->GetZNear(),cam->GetZFar());
}
