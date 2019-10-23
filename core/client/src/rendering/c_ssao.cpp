#include "stdafx_client.h"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/debug/widebugssao.hpp"
#include "pragma/gui/widebugdepthtexture.h"
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
	auto &descSetPrepass = *descSetGroupPrepass->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_texture(descSetPrepass,*texNorm,umath::to_integral(pragma::ShaderSSAO::PrepassBinding::NormalBuffer));
	prosper::util::set_descriptor_set_binding_texture(descSetPrepass,*texDepth,umath::to_integral(pragma::ShaderSSAO::PrepassBinding::DepthBuffer));

	descSetGroupOcclusion = prosper::util::create_descriptor_set_group(dev,pragma::ShaderSSAOBlur::DESCRIPTOR_SET_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(*descSetGroupOcclusion->GetDescriptorSet(),*renderTarget->GetTexture(),0u);
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

void Console::commands::debug_ssao(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
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

	auto &scene = c_game->GetScene();
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto *rasterizer = static_cast<pragma::rendering::RasterizationRenderer*>(renderer);
	auto &ssaoInfo = rasterizer->GetSSAOInfo();
	auto &prepass = rasterizer->GetPrepass();

	auto bExtended = prepass.IsExtended();
	auto xOffset = 0u;
	if(bExtended == true)
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
		auto *pPrepassDepth = wgui.Create<WIDebugDepthTexture>(pEl);
		if(pPrepassDepth != nullptr)
		{
			pPrepassDepth->SetX(xOffset);
			pPrepassDepth->SetSize(256,256);
			pPrepassDepth->SetTexture(*prepass.textureDepth);
			pPrepassDepth->Update();
			xOffset += 256;
		}
	}
	auto *pSsao = wgui.Create<WIDebugSSAO>(pEl);
	if(pSsao != nullptr)
	{
		pSsao->SetX(xOffset);
		pSsao->SetSize(256,256);
		pSsao->Update();
		xOffset += 256;
	}
	auto *pSsaoBlur = wgui.Create<WIDebugSSAO>(pEl);
	if(pSsaoBlur != nullptr)
	{
		pSsaoBlur->SetX(xOffset);
		pSsaoBlur->SetSize(256,256);
		pSsaoBlur->SetUseBlurredSSAOImage(true);
		pSsaoBlur->Update();
		xOffset += 256;
	}

	pEl->SizeToContents();
}
