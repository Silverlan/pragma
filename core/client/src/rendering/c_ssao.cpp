/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao_blur.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/debug/widebugssao.hpp"
#include "pragma/gui/widebugdepthtexture.h"
#include <wgui/types/wirect.h>
#include <wgui/types/wiroot.h>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <shader/prosper_shader_t.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

bool SSAOInfo::Initialize(prosper::IPrContext &context, uint32_t width, uint32_t height, prosper::SampleCountFlags samples, const std::shared_ptr<prosper::Texture> &texNorm, const std::shared_ptr<prosper::Texture> &texDepth)
{
	if(pragma::ShaderSSAO::DESCRIPTOR_SET_PREPASS.IsValid() == false || pragma::ShaderSSAOBlur::DESCRIPTOR_SET_TEXTURE.IsValid() == false)
		return false;
	shader = c_engine->GetShader("ssao");
	shaderBlur = c_engine->GetShader("ssao_blur");

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = pragma::ShaderSSAO::RENDER_PASS_FORMAT;
	imgCreateInfo.width = 512u;  // SSAO is very expensive depending on the resolution.
	imgCreateInfo.height = 512u; // 512x512 is a good compromise between quality and performance.
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	auto img = context.CreateImage(imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = context.CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	auto rp = prosper::ShaderGraphics::GetRenderPass<pragma::ShaderSSAO>(c_engine->GetRenderContext());
	renderTarget = context.CreateRenderTarget({tex}, rp);
	renderTarget->SetDebugName("ssao_rt");

	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
	auto imgBlur = context.CreateImage(imgCreateInfo);
	auto texBlur = context.CreateTexture({}, *imgBlur, imgViewCreateInfo, samplerCreateInfo);
	renderTargetBlur = context.CreateRenderTarget({texBlur}, rp);
	renderTargetBlur->SetDebugName("ssao_blur_rt");
	descSetGroupPrepass = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderSSAO::DESCRIPTOR_SET_PREPASS);
	auto &descSetPrepass = *descSetGroupPrepass->GetDescriptorSet();
	descSetPrepass.SetBindingTexture(*texNorm, umath::to_integral(pragma::ShaderSSAO::PrepassBinding::NormalBuffer));
	descSetPrepass.SetBindingTexture(*texDepth, umath::to_integral(pragma::ShaderSSAO::PrepassBinding::DepthBuffer));

	descSetGroupOcclusion = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderSSAOBlur::DESCRIPTOR_SET_TEXTURE);
	descSetGroupOcclusion->GetDescriptorSet()->SetBindingTexture(renderTarget->GetTexture(), 0u);
	return true;
}

void SSAOInfo::Clear()
{
	renderTarget = nullptr;
	renderTargetBlur = nullptr;
	descSetGroupPrepass = nullptr;
	descSetGroupOcclusion = nullptr;
}

prosper::Shader *SSAOInfo::GetSSAOShader() const { return shader.get(); }
prosper::Shader *SSAOInfo::GetSSAOBlurShader() const { return shaderBlur.get(); }

void Console::commands::debug_ssao(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &wgui = WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	if(c_game == nullptr || argv.empty() || pRoot == nullptr)
		return;
	const std::string name = "debug_ssao";
	auto *pEl = pRoot->FindDescendantByName(name);
	auto v = util::to_int(argv.front());
	if(v == 0) {
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
	auto rasterizer = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	if(rasterizer.expired())
		return;
	auto &ssaoInfo = rasterizer->GetSSAOInfo();
	auto &prepass = rasterizer->GetPrepass();

	auto bExtended = prepass.IsExtended();
	auto xOffset = 0u;
	if(bExtended == true) {
		auto *pNormals = wgui.Create<WITexturedRect>(pEl);
		if(pNormals != nullptr) {
			pNormals->SetX(xOffset);
			pNormals->SetSize(256, 256);
			pNormals->SetTexture(*prepass.textureNormals);
			pNormals->Update();
			xOffset += 256;
		}
		auto *pPrepassDepth = wgui.Create<WIDebugDepthTexture>(pEl);
		if(pPrepassDepth != nullptr) {
			pPrepassDepth->SetX(xOffset);
			pPrepassDepth->SetSize(256, 256);
			pPrepassDepth->SetTexture(*prepass.textureDepth);
			pPrepassDepth->Update();
			xOffset += 256;
		}
	}
	auto *pSsao = wgui.Create<WIDebugSSAO>(pEl);
	if(pSsao != nullptr) {
		pSsao->SetX(xOffset);
		pSsao->SetSize(256, 256);
		pSsao->Update();
		xOffset += 256;
	}
	auto *pSsaoBlur = wgui.Create<WIDebugSSAO>(pEl);
	if(pSsaoBlur != nullptr) {
		pSsaoBlur->SetX(xOffset);
		pSsaoBlur->SetSize(256, 256);
		pSsaoBlur->SetUseBlurredSSAOImage(true);
		pSsaoBlur->Update();
		xOffset += 256;
	}

	pEl->SizeToContents();
}

static void cl_render_ssao_callback(NetworkState *, const ConVar &, bool, bool val)
{
	if(client == nullptr)
		return;
	client->UpdateGameWorldShaderSettings();
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_ssao, cl_render_ssao_callback);
