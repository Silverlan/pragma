// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.debug_depth_texture;
import :engine;
import :game;
import :rendering.shaders;

pragma::gui::types::WIDebugDepthTexture::WIDebugDepthTexture() : WIBase(), m_imageLayer(0)
{
	m_whDepthToRgbShader = get_cengine()->GetShader("debug_depth_to_rgb");
	m_whCubeDepthToRgbShader = get_cengine()->GetShader("debug_cube_depth_to_rgb");
	m_whCsmDepthToRgbShader = get_cengine()->GetShader("debug_csm_depth_to_rgb");
}

pragma::gui::types::WIDebugDepthTexture::~WIDebugDepthTexture()
{
	if(m_depthToRgbCallback.IsValid())
		m_depthToRgbCallback.Remove();
}

void pragma::gui::types::WIDebugDepthTexture::SetTexture(prosper::Texture &texture, bool stencil)
{
	SetTexture(texture, {prosper::PipelineStageFlags::LateFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit},
	  {prosper::PipelineStageFlags::EarlyFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit}, 0u, stencil);
}

void pragma::gui::types::WIDebugDepthTexture::SetTexture(prosper::Texture &texture, prosper::util::BarrierImageLayout srcLayout, prosper::util::BarrierImageLayout dstLayout, uint32_t layerId, bool stencil)
{
	m_srcDepthTex = nullptr;

	if(m_whDepthToRgbShader.expired() || ShaderDepthToRGB::DESCRIPTOR_SET.IsValid() == false)
		return;
	auto &shader = static_cast<prosper::ShaderGraphics &>(*m_whDepthToRgbShader.get());
	auto &inputImg = texture.GetImage();
	auto extents = inputImg.GetExtents();
	auto &context = get_cengine()->GetRenderContext();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.width = extents.width;
	imgCreateInfo.height = extents.height;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
	auto img = context.CreateImage(imgCreateInfo);
	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = context.CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);
	m_renderTarget = context.CreateRenderTarget({tex}, shader.GetRenderPass());
	m_renderTarget->SetDebugName("debug_depth_rt");

	m_dsgSceneDepthTex = context.CreateDescriptorSetGroup(ShaderDepthToRGB::DESCRIPTOR_SET);

	imgViewCreateInfo = {};
	imgViewCreateInfo.baseLayer = layerId;
	imgViewCreateInfo.levelCount = 1u; //inputImg.get_image_n_layers();
	imgViewCreateInfo.aspectFlags = stencil ? prosper::ImageAspectFlags::StencilBit : prosper::ImageAspectFlags::DepthBit;
	samplerCreateInfo = {};
	m_srcDepthTex = context.CreateTexture({}, texture.GetImage(), imgViewCreateInfo, samplerCreateInfo);
	m_srcDepthTex->SetDebugName("debug_depth_src_rt");
	if(inputImg.GetLayerCount() == 1u)
		m_dsgSceneDepthTex->GetDescriptorSet()->SetBindingTexture(*m_srcDepthTex, 0u);
	else
		m_dsgSceneDepthTex->GetDescriptorSet()->SetBindingArrayTexture(*m_srcDepthTex, 0u, 0u);

	if(m_hTextureRect.IsValid())
		static_cast<WITexturedRect &>(*m_hTextureRect.get()).SetTexture(m_renderTarget->GetTexture());
	m_srcBarrierImageLayout = srcLayout;
	m_dstBarrierImageLayout = dstLayout;
	m_imageLayer = layerId;
}

void pragma::gui::types::WIDebugDepthTexture::UpdateResolvedTexture() {}

void pragma::gui::types::WIDebugDepthTexture::SetShouldResolveImage(bool b) {}

void pragma::gui::types::WIDebugDepthTexture::Initialize()
{
	m_hTextureRect = CreateChild<WITexturedRect>();
	m_hTextureRect->SetAutoAlignToParent(true);
}

void pragma::gui::types::WIDebugDepthTexture::Setup(float nearZ, float farZ)
{
	if(m_depthToRgbCallback.IsValid())
		m_depthToRgbCallback.Remove();
	m_depthToRgbCallback = get_cengine()->AddCallback("DrawFrame", FunctionCallback<void, std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>>>::Create([this, nearZ, farZ](std::reference_wrapper<std::shared_ptr<prosper::IPrimaryCommandBuffer>> refDrawCmd) {
		auto &drawCmd = refDrawCmd.get();
		if(m_whDepthToRgbShader.expired() || m_srcDepthTex == nullptr || m_renderTarget == nullptr || m_dsgSceneDepthTex == nullptr)
			return;
		auto &img = m_renderTarget->GetTexture().GetImage();
		drawCmd->RecordImageBarrier(img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
		auto &depthImg = m_srcDepthTex->GetImage();
		auto &imgView = *m_srcDepthTex->GetImageView();
		drawCmd->RecordImageBarrier(depthImg, m_srcBarrierImageLayout.stageMask, prosper::PipelineStageFlags::FragmentShaderBit, m_srcBarrierImageLayout.layout, prosper::ImageLayout::ShaderReadOnlyOptimal, m_srcBarrierImageLayout.accessMask, prosper::AccessFlags::ShaderReadBit,
		  std::numeric_limits<uint32_t>::max(), imgView.GetAspectMask());
		if(drawCmd->RecordBeginRenderPass(*m_renderTarget) == true) {
			if(depthImg.IsCubemap()) {
				auto &shader = static_cast<ShaderCubeDepthToRGB &>(*m_whCubeDepthToRgbShader.get());
				prosper::ShaderBindState bindState {*drawCmd};
				if(shader.RecordBeginDraw(bindState) == true) {
					shader.RecordDraw(bindState, *m_dsgSceneDepthTex->GetDescriptorSet(), nearZ, farZ, m_imageLayer, GetContrastFactor());
					shader.RecordEndDraw(bindState);
				}
			}
			else if(img.GetLayerCount() > 1u) {
				auto &shader = static_cast<ShaderCSMDepthToRGB &>(*m_whCsmDepthToRgbShader.get());
				prosper::ShaderBindState bindState {*drawCmd};
				if(shader.RecordBeginDraw(bindState) == true) {
					shader.RecordDraw(bindState, *m_dsgSceneDepthTex->GetDescriptorSet(), nearZ, farZ, m_imageLayer, GetContrastFactor());
					shader.RecordEndDraw(bindState);
				}
			}
			else {
				auto &shader = static_cast<ShaderDepthToRGB &>(*m_whDepthToRgbShader.get());
				prosper::ShaderBindState bindState {*drawCmd};
				if(shader.RecordBeginDraw(bindState) == true) {
					shader.RecordDraw(bindState, *m_dsgSceneDepthTex->GetDescriptorSet(), nearZ, farZ, GetContrastFactor());
					shader.RecordEndDraw(bindState);
				}
			}
			drawCmd->RecordEndRenderPass();
		}
		drawCmd->RecordImageBarrier(depthImg, prosper::PipelineStageFlags::FragmentShaderBit, m_dstBarrierImageLayout.stageMask, prosper::ImageLayout::ShaderReadOnlyOptimal, m_dstBarrierImageLayout.layout, prosper::AccessFlags::ShaderReadBit, m_dstBarrierImageLayout.accessMask,
		  std::numeric_limits<uint32_t>::max(), imgView.GetAspectMask());
	}));
}

void pragma::gui::types::WIDebugDepthTexture::SetContrastFactor(float contrastFactor) { m_contrastFactor = contrastFactor; }
float pragma::gui::types::WIDebugDepthTexture::GetContrastFactor() const { return m_contrastFactor; }

void pragma::gui::types::WIDebugDepthTexture::DoUpdate()
{
	auto nearZ = baseEnvCameraComponent::DEFAULT_NEAR_Z;
	auto farZ = baseEnvCameraComponent::DEFAULT_FAR_Z;
	if(get_cgame()) {
		auto *cam = get_cgame()->GetPrimaryCamera<CCameraComponent>();
		if(cam) {
			nearZ = cam->GetNearZ();
			farZ = cam->GetFarZ();
		}
	}
	Setup(nearZ, farZ);
}
