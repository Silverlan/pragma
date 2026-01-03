// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.prepass;
import :debug;
import :engine;
import :entities.components;
import :game;
import :gui;

bool pragma::rendering::Prepass::Initialize(prosper::IPrContext &context, uint32_t width, uint32_t height, prosper::SampleCountFlags samples, bool bExtended)
{
	m_shaderPrepass = context.GetShader("prepass");

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = ShaderGameWorldLightingPass::RENDER_PASS_DEPTH_FORMAT;
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
	textureDepth = context.CreateTexture(texCreateInfo, *imgDepth, imgViewCreateInfo, samplerCreateInfo);

#ifdef ENABLE_TRANSLUCENT_DEPTH_PREPASS
	auto imgDepthTranslucent = context.CreateImage(imgCreateInfo);
	textureDepthTranslucent = context.CreateTexture(texCreateInfo, *imgDepthTranslucent, imgViewCreateInfo, samplerCreateInfo);
#endif

	imgCreateInfo.usage = prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::SampledBit;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
	imgCreateInfo.samples = prosper::SampleCountFlags::e1Bit;
	auto imgDepthSampled = context.CreateImage(imgCreateInfo);

	texCreateInfo.flags = {};
	textureDepthSampled = context.CreateTexture(texCreateInfo, *imgDepthSampled, imgViewCreateInfo, samplerCreateInfo);

	SetUseExtendedPrepass(bExtended, true);
	return true;
}

pragma::ShaderPrepassBase &pragma::rendering::Prepass::GetShader() const { return static_cast<ShaderPrepassBase &>(*m_shaderPrepass.get()); }

bool pragma::rendering::Prepass::IsExtended() const { return m_bExtended; }
void pragma::rendering::Prepass::SetUseExtendedPrepass(bool b, bool bForceReload)
{
	if(m_bExtended == b && bForceReload == false)
		return;
	m_bExtended = b;

	auto &context = get_cengine()->GetRenderContext();
	context.WaitIdle();

	auto &imgDepth = textureDepth->GetImage();
	auto extents = imgDepth.GetExtents();
	auto width = extents.width;
	auto height = extents.height;

	auto whShaderPrepass = get_cengine()->GetShader("prepass");
	if(whShaderPrepass.expired())
		return;

	auto *shaderPrepass = static_cast<ShaderPrepass *>(whShaderPrepass.get());
	auto sampleCount = imgDepth.GetSampleCount();
	//auto pipelineType = pragma::ShaderPrepassBase::GetPipelineIndex(sampleCount);
	//if(b == true)
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
		textureNormals = context.CreateTexture(texCreateInfo, *imgNormals, imgViewCreateInfo, samplerCreateInfo);

		auto &imgDepth = textureDepth->GetImage();
		renderTarget = context.CreateRenderTarget({textureNormals, textureDepth}, shaderPrepass->GetRenderPass()); //pragma::math::to_integral(pipelineType)));
		renderTarget->SetDebugName("prepass_depth_normal_rt");
		m_clearValues = {
		  prosper::ClearValue {prosper::ClearColorValue {}},             // Unused, but required
		  prosper::ClearValue {prosper::ClearDepthStencilValue {1.f, 0}} // Clear depth
		};
	}
	/*else
	{
		textureNormals = nullptr;

		renderTarget = context.CreateRenderTarget({textureDepth},shaderPrepassDepth->GetRenderPass(pragma::math::to_integral(pipelineType)));
		renderTarget->SetDebugName("prepass_depth_rt");
		m_clearValues = {
			prosper::ClearValue{prosper::ClearDepthStencilValue{1.f,0}} // Clear depth
		};
	}*/
	prosper::util::RenderPassCreateInfo rpInfo {{ShaderPrepass::get_normal_render_pass_attachment_info(sampleCount), ShaderPrepass::get_depth_render_pass_attachment_info(sampleCount)}};
	for(auto &att : rpInfo.attachments)
		att.loadOp = prosper::AttachmentLoadOp::Load;
	subsequentRenderPass = get_cengine()->GetRenderContext().CreateRenderPass(rpInfo);
}

prosper::RenderTarget &pragma::rendering::Prepass::BeginRenderPass(const DrawSceneInfo &drawSceneInfo, prosper::IRenderPass *optRenderPass, bool secondaryCommandBuffers)
{
	// prosper TODO: Barriers for imgDepth and imgNormals
	drawSceneInfo.commandBuffer->RecordBeginRenderPass(*renderTarget, m_clearValues, secondaryCommandBuffers ? prosper::IPrimaryCommandBuffer::RenderPassFlags::SecondaryCommandBuffers : prosper::IPrimaryCommandBuffer::RenderPassFlags::None, optRenderPass);
	return *renderTarget;
}
void pragma::rendering::Prepass::EndRenderPass(const DrawSceneInfo &drawSceneInfo) { drawSceneInfo.commandBuffer->RecordEndRenderPass(); }

static void debug_prepass(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
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
	auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	if(raster.expired())
		return;
	auto &ssaoInfo = raster->GetSSAOInfo();
	auto &prepass = raster->GetPrepass();

	auto bExtended = prepass.IsExtended();
	auto xOffset = 0u;
	if(prepass.textureNormals != nullptr) {
		auto *pNormals = wgui.Create<pragma::gui::types::WITexturedRect>(pEl);
		if(pNormals != nullptr) {
			pNormals->SetX(xOffset);
			pNormals->SetSize(256, 256);
			pNormals->SetTexture(*prepass.textureNormals);
			pNormals->Update();
			xOffset += 256;
		}
	}
	auto *pPrepassDepth = wgui.Create<pragma::gui::types::WIDebugDepthTexture>(pEl);
	if(pPrepassDepth != nullptr) {
		pPrepassDepth->SetX(xOffset);
		pPrepassDepth->SetSize(256, 256);
		pPrepassDepth->SetTexture(*prepass.textureDepth);
		pPrepassDepth->Update();
		xOffset += 256;
	}

	pEl->SizeToContents();
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_prepass", &debug_prepass, pragma::console::ConVarFlags::None, "Displays prepass buffers to screen.");
}
