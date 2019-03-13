#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/shaders/image/c_shader_additive.h"
#include "pragma/rendering/shaders/post_processing/c_shader_glow.hpp"
#include <prosper_util.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_sampler.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

Scene::GlowInfo::GlowInfo()
	: //renderTarget(),cmdBufferBlur(nullptr),descSetAdditive(nullptr), // prosper TODO
	/*renderPass(nullptr),framebuffer(nullptr),*/bGlowScheduled(false) // prosper TODO
{
	shader = c_engine->GetShader("glow");
}
Scene::GlowInfo::~GlowInfo()
{
	if(m_cbReloadCommandBuffer.IsValid())
		m_cbReloadCommandBuffer.Remove();
}
bool Scene::GlowInfo::Initialize(uint32_t width,uint32_t height,const HDRInfo &hdrInfo)
{
	auto depthTex = hdrInfo.prepass.textureDepth;
	if(depthTex->IsMSAATexture())
		depthTex = static_cast<prosper::MSAATexture&>(*depthTex).GetResolvedTexture();

	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.format = pragma::ShaderGlow::RENDER_PASS_FORMAT;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT; // Note: Transfer flag required for debugging purposes only (See debug_glow_bloom console command)
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	auto img = prosper::util::create_image(dev,imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	auto tex = prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
	renderTarget = prosper::util::create_render_target(dev,{tex,depthTex},prosper::ShaderGraphics::GetRenderPass<pragma::ShaderGlow>(*c_engine));
	renderTarget->SetDebugName("glow_rt");

	auto rtBlur = prosper::util::create_render_target(dev,{tex},prosper::ShaderGraphics::GetRenderPass<prosper::ShaderBlurBase>(*c_engine,umath::to_integral(prosper::ShaderBlurBase::Pipeline::R8G8B8A8Unorm)));
	rtBlur->SetDebugName("glow_blur_rt");
	blurSet = prosper::BlurSet::Create(dev,rtBlur);
	//	std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
	//	std::shared_ptr<prosper::BlurSet> blurSet = nullptr;
	/*auto &context = c_engine->GetRenderContext();
	auto format = Anvil::Format::R8G8B8A8_UNORM;
	renderTarget.Initialize(context,width,height,format,true);

	auto &texDepth = hdrInfo.GetTargetDepthTexture();
	renderPass = context.GenerateRenderPass({
		{format,true},
		{texDepth->GetFormat()}
	});
	framebuffer = Vulkan::Framebuffer::Create(context,renderPass,width,height,{renderTarget.GetTexture()->GetImageView(),texDepth->GetImageView()});

	static auto hShaderBlur = ShaderSystem::get_shader("pp_gaussianblur");
	if(hShaderBlur.IsValid())
	{
		m_cbReloadCommandBuffer = FunctionCallback<void,Shader::Base*>::Create([this,width,height](Shader::Base *shader) {
			//auto &context = c_engine->GetRenderContext();
			renderTarget.blurBuffer.tmpBlurTexture->GetImage()->SetInternalLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			renderTarget.GetTexture()->GetImage()->SetInternalLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
			//cmdBufferBlur = Vulkan::CommandBuffer::Create(context,vk::CommandBufferLevel::eSecondary);
			//cmdBufferBlur->Begin();
			auto hShader = shader->GetHandle();
			cmdBufferBlur = [this,hShader,width,height](Vulkan::CommandBufferObject *cmd) {
				const auto blurSize = 1.75f;
				const auto kernelSize = 3;
				const auto blurAmount = 5;

				auto &shaderBlur = static_cast<Shader::GaussianBlur&>(*hShader.get());
				for(auto i=decltype(blurAmount){0};i<blurAmount;++i)
					shaderBlur.Render(cmd,renderTarget.GetRenderPass(),Vector2i(width,height),renderTarget.GetTexture(),renderTarget.blurBuffer,renderTarget.GetFramebuffer(),blurSize,kernelSize,Vector4(1.f,1.f,1.f,1.f));
			};
			//cmdBufferBlur->End();
		});
		m_cbReloadCommandBuffer(hShaderBlur.get());
		hShaderBlur->CallOnReload(m_cbReloadCommandBuffer);
	}

	auto hShaderAdditive = c_engine->GetShader("additive");
	if(hShaderAdditive.IsValid())
	{
		auto *shader = static_cast<Shader::Additive*>(hShaderAdditive.get());
		shader->InitializeInstance(descSetAdditive);
		auto &descSet = descSetAdditive;
		descSet->Update(static_cast<uint32_t>(Shader::Additive::Binding::Texture),hdrInfo.GetTargetTexture());
		descSet->Update(static_cast<uint32_t>(Shader::Additive::Binding::TextureAdd),renderTarget.GetTexture());
	}*/ // prosper TODO
	return true;
}
