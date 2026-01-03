// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.ssao;
import :client_state;
import :debug;
import :engine;
import :entities.components;
import :game;
import :gui;
import :rendering.shaders;

bool pragma::rendering::SSAOInfo::Initialize(prosper::IPrContext &context, uint32_t width, uint32_t height, prosper::SampleCountFlags samples, const std::shared_ptr<prosper::Texture> &texNorm, const std::shared_ptr<prosper::Texture> &texDepth)
{
	if(ShaderSSAO::DESCRIPTOR_SET_PREPASS.IsValid() == false || shaderSSAOBlur::get_descriptor_set_texture().IsValid() == false)
		return false;
	shader = get_cengine()->GetShader("ssao");
	shaderBlur = get_cengine()->GetShader("ssao_blur");

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = ShaderSSAO::RENDER_PASS_FORMAT;
	imgCreateInfo.width = 512u;  // SSAO is very expensive depending on the resolution.
	imgCreateInfo.height = 512u; // 512x512 is a good compromise between quality and performance.
	imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	auto img = context.CreateImage(imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = context.CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	auto rp = prosper::ShaderGraphics::GetRenderPass<ShaderSSAO>(get_cengine()->GetRenderContext());
	renderTarget = context.CreateRenderTarget({tex}, rp);
	renderTarget->SetDebugName("ssao_rt");

	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
	auto imgBlur = context.CreateImage(imgCreateInfo);
	auto texBlur = context.CreateTexture({}, *imgBlur, imgViewCreateInfo, samplerCreateInfo);
	renderTargetBlur = context.CreateRenderTarget({texBlur}, rp);
	renderTargetBlur->SetDebugName("ssao_blur_rt");
	descSetGroupPrepass = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderSSAO::DESCRIPTOR_SET_PREPASS);
	auto &descSetPrepass = *descSetGroupPrepass->GetDescriptorSet();
	descSetPrepass.SetBindingTexture(*texNorm, math::to_integral(ShaderSSAO::PrepassBinding::NormalBuffer));
	descSetPrepass.SetBindingTexture(*texDepth, math::to_integral(ShaderSSAO::PrepassBinding::DepthBuffer));

	descSetGroupOcclusion = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(shaderSSAOBlur::get_descriptor_set_texture());
	descSetGroupOcclusion->GetDescriptorSet()->SetBindingTexture(renderTarget->GetTexture(), 0u);
	return true;
}

void pragma::rendering::SSAOInfo::Clear()
{
	renderTarget = nullptr;
	renderTargetBlur = nullptr;
	descSetGroupPrepass = nullptr;
	descSetGroupOcclusion = nullptr;
}

prosper::Shader *pragma::rendering::SSAOInfo::GetSSAOShader() const { return shader.get(); }
prosper::Shader *pragma::rendering::SSAOInfo::GetSSAOBlurShader() const { return shaderBlur.get(); }

static void debug_ssao(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &wgui = pragma::gui::WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	if(pragma::get_cgame() == nullptr || argv.empty() || pRoot == nullptr)
		return;
	const std::string name = "debug_ssao";
	auto *pEl = pRoot->FindDescendantByName(name);
	auto v = pragma::util::to_int(argv.front());
	if(v == 0) {
		if(pEl != nullptr)
			pEl->Remove();
		return;
	}
	if(pEl != nullptr)
		return;
	pEl = wgui.Create<pragma::gui::types::WIBase>();
	if(pEl == nullptr)
		return;
	pEl->SetName(name);

	auto *scene = pragma::get_cgame()->GetScene<pragma::CSceneComponent>();
	auto *renderer = scene ? scene->GetRenderer<pragma::CRendererComponent>() : nullptr;
	auto rasterizer = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	if(rasterizer.expired())
		return;
	auto &ssaoInfo = rasterizer->GetSSAOInfo();
	auto &prepass = rasterizer->GetPrepass();

	auto bExtended = prepass.IsExtended();
	auto xOffset = 0u;
	if(bExtended == true) {
		auto *pNormals = wgui.Create<pragma::gui::types::WITexturedRect>(pEl);
		if(pNormals != nullptr) {
			pNormals->SetX(xOffset);
			pNormals->SetSize(256, 256);
			pNormals->SetTexture(*prepass.textureNormals);
			pNormals->Update();
			xOffset += 256;
		}
		auto *pPrepassDepth = wgui.Create<pragma::gui::types::WIDebugDepthTexture>(pEl);
		if(pPrepassDepth != nullptr) {
			pPrepassDepth->SetX(xOffset);
			pPrepassDepth->SetSize(256, 256);
			pPrepassDepth->SetTexture(*prepass.textureDepth);
			pPrepassDepth->Update();
			xOffset += 256;
		}
	}
	auto *pSsao = wgui.Create<pragma::gui::types::WIDebugSSAO>(pEl);
	if(pSsao != nullptr) {
		pSsao->SetX(xOffset);
		pSsao->SetSize(256, 256);
		pSsao->Update();
		xOffset += 256;
	}
	auto *pSsaoBlur = wgui.Create<pragma::gui::types::WIDebugSSAO>(pEl);
	if(pSsaoBlur != nullptr) {
		pSsaoBlur->SetX(xOffset);
		pSsaoBlur->SetSize(256, 256);
		pSsaoBlur->SetUseBlurredSSAOImage(true);
		pSsaoBlur->Update();
		xOffset += 256;
	}

	pEl->SizeToContents();
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_ssao", &debug_ssao, pragma::console::ConVarFlags::None, "Displays the ssao buffers to screen.");
}

static void cl_render_ssao_callback(pragma::NetworkState *, const pragma::console::ConVar &, bool, bool val)
{
	auto *client = pragma::get_client_state();
	if(client == nullptr)
		return;
	client->UpdateGameWorldShaderSettings();
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>("cl_render_ssao", &cl_render_ssao_callback);
}
