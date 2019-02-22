#include "stdafx_client.h"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/gui/widebugmsaatexture.hpp"
#include <wgui/types/wirect.h>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

bool SSAOInfo::Initialize(
	prosper::Context &context,uint32_t width,uint32_t height,Anvil::SampleCountFlagBits samples,
	const std::shared_ptr<prosper::Texture> &texNorm,const std::shared_ptr<prosper::Texture> &texDepth
)
{
	if(pragma::ShaderSSAO::DESCRIPTOR_SET_PREPASS.IsValid() == false || pragma::ShaderSSAOBlur::DESCRIPTOR_SET_TEXTURE.IsValid() == false)
		return false;
	shader = c_engine->GetShader("ssao");
	shaderBlur = c_engine->GetShader("ssao_blur");

	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = pragma::ShaderSSAO::RENDER_PASS_FORMAT;
	imgCreateInfo.width = 512u; // SSAO is very expensive depending on the resolution.
	imgCreateInfo.height = 512u; // 512x512 is a good compromise between quality and performance.
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	auto img = prosper::util::create_image(dev,imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
	auto rp = prosper::ShaderGraphics::GetRenderPass<pragma::ShaderSSAO>(*c_engine);
	renderTarget = prosper::util::create_render_target(dev,{tex},rp);
	renderTarget->SetDebugName("ssao_rt");

	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;
	auto imgBlur = prosper::util::create_image(dev,imgCreateInfo);
	auto texBlur = prosper::util::create_texture(dev,{},imgBlur,&imgViewCreateInfo,&samplerCreateInfo);
	renderTargetBlur = prosper::util::create_render_target(dev,{texBlur},rp);
	renderTargetBlur->SetDebugName("ssao_blur_rt");
	descSetGroupPrepass = prosper::util::create_descriptor_set_group(dev,pragma::ShaderSSAO::DESCRIPTOR_SET_PREPASS);
	auto &descSetPrepass = *(*descSetGroupPrepass)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_texture(descSetPrepass,*texNorm,umath::to_integral(pragma::ShaderSSAO::PrepassBinding::NormalBuffer));
	prosper::util::set_descriptor_set_binding_texture(descSetPrepass,*texDepth,umath::to_integral(pragma::ShaderSSAO::PrepassBinding::DepthBuffer));

	descSetGroupOcclusion = prosper::util::create_descriptor_set_group(dev,pragma::ShaderSSAOBlur::DESCRIPTOR_SET_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(*(*descSetGroupOcclusion)->get_descriptor_set(0u),*renderTarget->GetTexture(),0u);
	return true;
}

void SSAOInfo::Clear()
{
	renderTarget = nullptr;
	renderTargetBlur = nullptr;
	descSetGroupPrepass = nullptr;
	descSetGroupOcclusion = nullptr;
}

prosper::Shader *SSAOInfo::GetSSAOShader() const {return shader.get();}
prosper::Shader *SSAOInfo::GetSSAOBlurShader() const {return shaderBlur.get();}

 // prosper TODO
/*
void SSAOInfo::Initialize(const Vulkan::Context &context,uint32_t width,uint32_t height,Anvil::SampleCountFlagBits samples,const Vulkan::Texture &texNorm,const Vulkan::Texture &texDepth)
{
	renderPass = context.GenerateRenderPass({
		{texNorm->GetFormat(),texNorm->GetSampleCount(),true},
		{texDepth->GetFormat(),texDepth->GetSampleCount(),true}
	});
	framebuffer = Vulkan::Framebuffer::Create(context,renderPass,width,height,{
		texNorm->GetImageView(),
		texDepth->GetImageView()
	});

	descSetNormalDepthBuffer = Shader::SSAO::CreateNormalDepthBufferDescSet();
	descSetNormalDepthBuffer->Update(umath::to_integral(Shader::SSAO::Binding::NormalBuffer),texNorm);
	descSetNormalDepthBuffer->Update(umath::to_integral(Shader::SSAO::Binding::DepthBuffer),texDepth);

	rtOcclusion = Vulkan::RenderTarget::Create(context,width,height,vk::Format::eR8Unorm,false,false,false,[](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags &flags) {
		info.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	});
	rtOcclusionBlur = Vulkan::RenderTarget::Create(context,width,height,vk::Format::eR8Unorm,false,false,false,[](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags &flags) {
		info.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	});

	static auto hShaderSSAOBlur = ShaderSystem::get_shader("ssao_blur");
	if(hShaderSSAOBlur.IsValid() == false)
		return;
	auto &shaderSSAOBlur = *hShaderSSAOBlur.get();
	if(shaderSSAOBlur.GenerateDescriptorSet(0,descSetOcclusion) == true)
		descSetOcclusion->Update(0,rtOcclusion->GetTexture());
}

void SSAOInfo::Clear()
{
	auto &context = c_engine->GetRenderContext();
	context.WaitIdle();
	renderPass = nullptr;
	framebuffer = nullptr;
	descSetNormalDepthBuffer = nullptr;

	rtOcclusion = nullptr;
	descSetOcclusion = nullptr;

	rtOcclusionBlur = nullptr;
}
*/
void Console::commands::debug_ssao(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg != nullptr)
	{
		dbg = nullptr;
		return;
	}
	dbg = nullptr;
	if(c_game == nullptr)
		return;
	static WIHandle hDepthTex = {};
	dbg = std::make_unique<DebugGameGUI>([]() {
		auto &scene = c_game->GetScene();
		auto &ssaoInfo = scene->GetSSAOInfo();
		auto &prepass = scene->GetPrepass();

		auto bExtended = prepass.IsExtended();
		auto count = (bExtended == true) ? 4 : 2;
		auto &wgui = WGUI::GetInstance();
		auto *r = wgui.Create<WIBase>();
		r->SetSize(256 *count,256);

		auto idx = 0u;
		if(bExtended == true)
		{
			auto *pNormals = wgui.Create<WIDebugMSAATexture>(r);
			pNormals->SetSize(256,256);
			pNormals->SetX(256 *idx++);
			pNormals->SetTexture(*prepass.textureNormals);
			pNormals->SetShouldResolveImage(true);

			auto *pDepths = wgui.Create<WIDebugDepthTexture>(r);
			pDepths->SetSize(256,256);
			pDepths->SetX(256 *idx++);
			pDepths->SetTexture(*prepass.textureDepth,{
				Anvil::PipelineStageFlagBits::LATE_FRAGMENT_TESTS_BIT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AccessFlagBits::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			},{
				Anvil::PipelineStageFlagBits::EARLY_FRAGMENT_TESTS_BIT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AccessFlagBits::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			});
			pDepths->Update();
			hDepthTex = pDepths->GetHandle();
		}

		auto *pOcclusion = wgui.Create<WITexturedRect>(r);
		pOcclusion->SetSize(256,256);
		pOcclusion->SetX(256 *idx++);
		pOcclusion->SetTexture(*ssaoInfo.renderTarget->GetTexture());

		auto *pOcclusionBlur = wgui.Create<WITexturedRect>(r);
		pOcclusionBlur->SetSize(256,256);
		pOcclusionBlur->SetX(256 *idx++);
		pOcclusionBlur->SetTexture(*ssaoInfo.renderTargetBlur->GetTexture());
		return r->GetHandle();
	});
	dbg->AddCallback("PostRenderScene",FunctionCallback<>::Create([]() {
		if(hDepthTex.IsValid() == false)
			return;
		static_cast<WIDebugDepthTexture*>(hDepthTex.get())->Update();
	}));
}
