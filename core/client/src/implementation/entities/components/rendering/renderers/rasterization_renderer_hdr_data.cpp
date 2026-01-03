// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :entities.components.rasterization_renderer;
import :debug;
import :engine;
import :entities.components.renderer;
import :game;
import :gui;
import :rendering.shaders;

using namespace pragma::rendering;

static const float EXPOSURE_FRAME_UPDATE = 0.25f; // Exposure will be updated every x seconds

static auto cvMaxExposure = pragma::console::get_client_con_var("render_hdr_max_exposure");
static void CVAR_CALLBACK_render_hdr_max_exposure(pragma::NetworkState *, const pragma::console::ConVar &, float, float val)
{
	if(pragma::get_cgame() == nullptr)
		return;
	pragma::get_cgame()->SetMaxHDRExposure(val);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>("render_hdr_max_exposure", &CVAR_CALLBACK_render_hdr_max_exposure);
}

HDRData::Exposure::Exposure() : lastExposureUpdate(0), averageColor(0.f, 0.f, 0.f) { m_shaderCalcColor = get_cengine()->GetShader("calcimagecolor"); }

bool HDRData::Exposure::Initialize(prosper::Texture &texture)
{
	m_exposureColorSource = texture.shared_from_this();
	if(ShaderCalcImageColor::DESCRIPTOR_SET_COLOR.IsValid() == false || ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE.IsValid() == false)
		return false;
	m_calcImgColorCmdBuffer = get_cengine()->GetRenderContext().AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType::Compute, m_cmdBufferQueueFamilyIndex);
	m_calcImgColorFence = get_cengine()->GetRenderContext().CreateFence();

	descSetGroupAverageColorBuffer = nullptr; // Has to be cleared first! (To release any previous descriptor sets)

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit;
	createInfo.size = sizeof(Vector4);
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	avgColorBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo);
	avgColorBuffer->SetDebugName("avg_color_buf");
	descSetGroupAverageColorBuffer = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderCalcImageColor::DESCRIPTOR_SET_COLOR);
	descSetGroupAverageColorBuffer->GetDescriptorSet()->SetBindingStorageBuffer(*avgColorBuffer, 0u);
	avgColorBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::ReadBit);

	descSetGroupAverageColorTexture = nullptr; // Has to be cleared first! (To release any previous descriptor sets)
	descSetGroupAverageColorTexture = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE);
	descSetGroupAverageColorTexture->GetDescriptorSet()->SetBindingTexture(texture, 0u);
	return true;
}

const Vector3 &HDRData::Exposure::UpdateColor()
{
	static auto skip = true;
	if(skip)
		return averageColor;
	if(m_shaderCalcColor.expired() || m_calcImgColorFence == nullptr || m_exposureColorSource.expired())
		return averageColor;
	auto &shader = static_cast<ShaderCalcImageColor &>(*m_shaderCalcColor.get());
	auto t = get_cgame()->RealTime();
	auto bFenceSet = m_calcImgColorFence->IsSet();
	if(m_bWaitingForResult == false) {
		if(t - lastExposureUpdate >= EXPOSURE_FRAME_UPDATE) {
			auto &primCmd = m_calcImgColorCmdBuffer;
			primCmd->StartRecording(false, true);
			auto &imgSrc = m_exposureColorSource.lock()->GetImage();
			m_calcImgColorCmdBuffer->RecordImageBarrier(imgSrc, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
			prosper::ShaderBindState bindState {*m_calcImgColorCmdBuffer};
			if(shader.RecordBeginCompute(bindState) == true) {
				const auto sampleCount = 32u;
				shader.RecordCompute(bindState, *descSetGroupAverageColorTexture->GetDescriptorSet(), *descSetGroupAverageColorBuffer->GetDescriptorSet(), sampleCount);
				shader.RecordEndCompute(bindState);
			}
			m_calcImgColorCmdBuffer->RecordImageBarrier(imgSrc, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
			primCmd->StopRecording();

			lastExposureUpdate = t;
			m_bWaitingForResult = true;
			// Issue new query
			get_cengine()->GetRenderContext().SubmitCommandBuffer(*m_calcImgColorCmdBuffer, false, m_calcImgColorFence.get());
		}
	}
	else if(bFenceSet == true) // Wait until computation has been completed before retrieving the color
	{
		m_bWaitingForResult = false;
		m_calcImgColorFence->Reset();

		Vector4 color(0.f, 0.f, 0.f, 0.f);
		avgColorBuffer->Read(0ull, color);
		averageColor = Vector3(color.r, color.g, color.b);
	}
	return averageColor;
}

/////////////////////////////////

HDRData::HDRData(CRasterizationRendererComponent &rasterizer) : exposure(1.f), m_bMipmapInitialized(false), m_exposure {}, m_cbReloadCommandBuffer {}, forwardPlusInstance {rasterizer} { max_exposure = cvMaxExposure->GetFloat(); }

HDRData::~HDRData()
{
	if(m_cbReloadCommandBuffer.IsValid())
		m_cbReloadCommandBuffer.Remove();

	auto &context = get_cengine()->GetRenderContext();
	context.KeepResourceAliveUntilPresentationComplete(sceneRenderTarget);
	context.KeepResourceAliveUntilPresentationComplete(dsgHDRPostProcessing);
	context.KeepResourceAliveUntilPresentationComplete(bloomTexture);
	context.KeepResourceAliveUntilPresentationComplete(bloomBlurRenderTarget);
	context.KeepResourceAliveUntilPresentationComplete(bloomBlurSet);
	context.KeepResourceAliveUntilPresentationComplete(dsgBloomTonemapping);
	context.KeepResourceAliveUntilPresentationComplete(toneMappedRenderTarget);
	context.KeepResourceAliveUntilPresentationComplete(dsgTonemappedPostProcessing);
	context.KeepResourceAliveUntilPresentationComplete(toneMappedPostProcessingRenderTarget);
	context.KeepResourceAliveUntilPresentationComplete(dsgToneMappedPostProcessing);
	context.KeepResourceAliveUntilPresentationComplete(dsgSceneDepth);
	context.KeepResourceAliveUntilPresentationComplete(dsgDepthPostProcessing);
	context.KeepResourceAliveUntilPresentationComplete(rpPostParticle);
	context.KeepResourceAliveUntilPresentationComplete(m_exposure.descSetGroupAverageColorBuffer);
	context.KeepResourceAliveUntilPresentationComplete(m_exposure.descSetGroupAverageColorTexture);
	context.KeepResourceAliveUntilPresentationComplete(m_exposure.avgColorBuffer);
	context.KeepResourceAliveUntilPresentationComplete(ssaoInfo.renderTarget);
	context.KeepResourceAliveUntilPresentationComplete(ssaoInfo.renderTargetBlur);
	context.KeepResourceAliveUntilPresentationComplete(ssaoInfo.descSetGroupPrepass);
	context.KeepResourceAliveUntilPresentationComplete(ssaoInfo.descSetGroupOcclusion);
	context.KeepResourceAliveUntilPresentationComplete(prepass.textureNormals);
	context.KeepResourceAliveUntilPresentationComplete(prepass.textureDepth);
	context.KeepResourceAliveUntilPresentationComplete(prepass.renderTarget);
	context.KeepResourceAliveUntilPresentationComplete(prepass.subsequentRenderPass);
	context.KeepResourceAliveUntilPresentationComplete(forwardPlusInstance.GetTileVisLightIndexBuffer());
	context.KeepResourceAliveUntilPresentationComplete(forwardPlusInstance.GetVisLightIndexBuffer());
}

bool HDRData::BeginRenderPass(const DrawSceneInfo &drawSceneInfo, prosper::IRenderPass *customRenderPass, bool secondaryCommandBuffers)
{
	auto &rt = GetRenderTarget(drawSceneInfo);
	return drawSceneInfo.commandBuffer->RecordBeginRenderPass(rt,
	  {
	    prosper::ClearValue {},                                                                    // Unused
	    prosper::ClearValue {prosper::ClearColorValue {std::array<float, 4> {0.f, 0.f, 0.f, 0.f}}} // Clear bloom
	  },
	  secondaryCommandBuffers ? prosper::IPrimaryCommandBuffer::RenderPassFlags::SecondaryCommandBuffers : prosper::IPrimaryCommandBuffer::RenderPassFlags::None, customRenderPass);
}
bool HDRData::EndRenderPass(const DrawSceneInfo &drawSceneInfo) { return drawSceneInfo.commandBuffer->RecordEndRenderPass(); }

prosper::util::SamplerCreateInfo HDRData::GetSamplerCreateInfo()
{
	prosper::util::SamplerCreateInfo hdrSamplerCreateInfo {};
	hdrSamplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	hdrSamplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	hdrSamplerCreateInfo.minFilter = prosper::Filter::Linear; // Note: These have to be linear for FXAA!
	hdrSamplerCreateInfo.magFilter = prosper::Filter::Linear;
	return hdrSamplerCreateInfo;
}

void HDRData::ReloadPresentationRenderTarget(uint32_t width, uint32_t height, prosper::SampleCountFlags sampleCount)
{
	auto &context = get_cengine()->GetRenderContext();
	if(toneMappedRenderTarget)
		context.KeepResourceAliveUntilPresentationComplete(toneMappedRenderTarget);
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = ShaderPPHDR::RENDER_PASS_FORMAT;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	imgCreateInfo.samples = prosper::SampleCountFlags::e1Bit;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	imgCreateInfo.samples = sampleCount;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;

	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;

	auto postHdrImg = context.CreateImage(imgCreateInfo);
	auto postHdrTex = context.CreateTexture(texCreateInfo, *postHdrImg, prosper::util::ImageViewCreateInfo {}, GetSamplerCreateInfo());

	auto &hShaderTonemapping = get_cgame()->GetGameShader(CGame::GameShader::PPTonemapping);
	toneMappedRenderTarget = context.CreateRenderTarget({postHdrTex}, static_cast<prosper::ShaderGraphics *>(hShaderTonemapping.get())->GetRenderPass());
	toneMappedRenderTarget->SetDebugName("scene_post_hdr_rt");

	dsgTonemappedPostProcessing->GetDescriptorSet()->SetBindingTexture(*postHdrTex, math::to_integral(ShaderPPFXAA::TextureBinding::SceneTexturePostToneMapping));
}

static auto cvBloomResolution = pragma::console::get_client_con_var("render_bloom_resolution");
bool HDRData::Initialize(uint32_t width, uint32_t height, prosper::SampleCountFlags sampleCount, bool bEnableSSAO)
{
	// Initialize depth prepass
	auto &context = get_cengine()->GetRenderContext();
	if(prepass.Initialize(context, width, height, sampleCount, bEnableSSAO) == false)
		return false;

	auto wpShader = get_cengine()->GetShader("pbr");
	auto &hShaderTonemapping = get_cgame()->GetGameShader(CGame::GameShader::PPTonemapping);
	if(wpShader.expired() || hShaderTonemapping.expired() || ShaderPPHDR::DESCRIPTOR_SET_TEXTURE.IsValid() == false || ShaderPPFog::DESCRIPTOR_SET_TEXTURE.IsValid() == false || ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER.IsValid() == false)
		return false;

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = ShaderGameWorldLightingPass::RENDER_PASS_FORMAT;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	imgCreateInfo.samples = sampleCount;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	auto hdrImg = context.CreateImage(imgCreateInfo);

	imgCreateInfo.usage
	  = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::TransferSrcBit; // Note: Transfer flag required for debugging purposes only (See debug_glow_bloom console command)
	if(sampleCount != prosper::SampleCountFlags::e1Bit)
		imgCreateInfo.usage |= prosper::ImageUsageFlags::TransferSrcBit;

	auto hdrBloomImg = context.CreateImage(imgCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;

	prosper::util::ImageViewCreateInfo hdrImgViewCreateInfo {};
	// Note: We need the alpha channel for FXAA Luma
	// hdrImgViewCreateInfo.swizzleAlpha = prosper::ComponentSwizzle::One;
	auto hdrSamplerCreateInfo = GetSamplerCreateInfo();
	auto hdrTex = context.CreateTexture(texCreateInfo, *hdrImg, hdrImgViewCreateInfo, hdrSamplerCreateInfo);
	auto resolvedTex = hdrTex;
	if(resolvedTex->IsMSAATexture())
		resolvedTex = static_cast<prosper::MSAATexture &>(*resolvedTex).GetResolvedTexture();

	bloomTexture = context.CreateTexture(texCreateInfo, *hdrBloomImg, hdrImgViewCreateInfo, hdrSamplerCreateInfo);
	sceneRenderTarget = get_cengine()->GetRenderContext().CreateRenderTarget({hdrTex, bloomTexture, prepass.textureDepth}, static_cast<prosper::ShaderGraphics *>(wpShader.get())->GetRenderPass());
	sceneRenderTarget->SetDebugName("scene_hdr_rt");
	auto resolvedBloomTex = bloomTexture;
	if(resolvedBloomTex->IsMSAATexture())
		resolvedBloomTex = static_cast<prosper::MSAATexture &>(*resolvedBloomTex).GetResolvedTexture();

	// Intermediate render pass
	auto &imgDepth = prepass.textureDepth->GetImage();
	prosper::util::RenderPassCreateInfo rpCreateInfo {};
	rpCreateInfo.attachments.push_back({hdrImg->GetFormat(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, hdrImg->GetSampleCount(), prosper::ImageLayout::ColorAttachmentOptimal});
	rpCreateInfo.attachments.push_back({hdrBloomImg->GetFormat(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, hdrBloomImg->GetSampleCount(), prosper::ImageLayout::ColorAttachmentOptimal});
	rpCreateInfo.attachments.push_back({imgDepth.GetFormat(), prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AttachmentLoadOp::Load, prosper::AttachmentStoreOp::Store, imgDepth.GetSampleCount(), prosper::ImageLayout::DepthStencilAttachmentOptimal});
	rpPostParticle = context.CreateRenderPass(rpCreateInfo);

	imgCreateInfo.usage |= prosper::ImageUsageFlags::TransferSrcBit;
	auto hdrImgStaging = context.CreateImage(imgCreateInfo);
	auto hdrTexStaging = context.CreateTexture(texCreateInfo, *hdrImgStaging, hdrImgViewCreateInfo, hdrSamplerCreateInfo);
	hdrPostProcessingRenderTarget = context.CreateRenderTarget({hdrTexStaging}, prosper::ShaderGraphics::GetRenderPass<ShaderPPBase>(context));
	hdrPostProcessingRenderTarget->SetDebugName("scene_staging_rt");

	dsgBloomTonemapping = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderPPHDR::DESCRIPTOR_SET_TEXTURE);
	auto &descSetHdrResolve = *dsgBloomTonemapping->GetDescriptorSet();
	descSetHdrResolve.SetBindingTexture(*resolvedTex, math::to_integral(ShaderPPHDR::TextureBinding::Texture));

	ReloadBloomRenderTarget(cvBloomResolution->GetInt());

	dsgHDRPostProcessing = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderPPFog::DESCRIPTOR_SET_TEXTURE);
	auto &descSetHdr = *dsgHDRPostProcessing->GetDescriptorSet();
	dsgHDRPostProcessing->GetDescriptorSet()->SetBindingTexture(*resolvedTex, 0u);

	imgCreateInfo.format = ShaderPPHDR::RENDER_PASS_FORMAT;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	imgCreateInfo.samples = prosper::SampleCountFlags::e1Bit;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	dsgTonemappedPostProcessing = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderPPFXAA::DESCRIPTOR_SET_TEXTURE);
	dsgTonemappedPostProcessing->GetDescriptorSet()->SetBindingTexture(*resolvedTex, math::to_integral(ShaderPPFXAA::TextureBinding::SceneTextureHdr));

	ReloadPresentationRenderTarget(width, height, sampleCount);

	{
		auto hShaderFXAA = get_cengine()->GetShader("pp_fxaa");
		if(hShaderFXAA.valid()) {
			// TODO: Obsolete?
			imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit;
			auto tmPpImg = context.CreateImage(imgCreateInfo);
			auto tmPpTex = context.CreateTexture(texCreateInfo, *tmPpImg, hdrImgViewCreateInfo, hdrSamplerCreateInfo);
			toneMappedPostProcessingRenderTarget = context.CreateRenderTarget({tmPpTex}, static_cast<prosper::ShaderGraphics *>(hShaderFXAA.get())->GetRenderPass());
			toneMappedPostProcessingRenderTarget->SetDebugName("scene_post_tonemapping_post_processing_rt");
			dsgToneMappedPostProcessing = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderPPFXAA::DESCRIPTOR_SET_TEXTURE);
			dsgToneMappedPostProcessing->GetDescriptorSet()->SetBindingTexture(*tmPpTex, 0u);
		}
	}

	if(m_exposure.Initialize(*resolvedTex) == false)
		return false;

	// Initialize SSAO
	if(bEnableSSAO == true) {
		auto texNormals = prepass.textureNormals;
		if(texNormals->IsMSAATexture())
			texNormals = static_cast<prosper::MSAATexture &>(*texNormals).GetResolvedTexture();
		auto texDepth = prepass.textureDepth;
		if(texDepth->IsMSAATexture())
			texDepth = static_cast<prosper::MSAATexture &>(*texDepth).GetResolvedTexture();
		if(ssaoInfo.Initialize(context, width, height, sampleCount, texNormals, texDepth) == false)
			return false;
	}

	// Initialize forward+
	auto resolvedTexture = prepass.textureDepth->IsMSAATexture() ? static_cast<prosper::MSAATexture &>(*prepass.textureDepth).GetResolvedTexture() : prepass.textureDepth;
	if(forwardPlusInstance.Initialize(context, width, height, *resolvedTexture) == false)
		return false;

	dsgDepthPostProcessing = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER);
	dsgDepthPostProcessing->GetDescriptorSet()->SetBindingTexture(*resolvedTexture, 0u);

	// Initialize particle render target
	rtParticle = context.CreateRenderTarget({sceneRenderTarget->GetTexture().shared_from_this(), bloomTexture, prepass.textureDepth}, prosper::ShaderGraphics::GetRenderPass<ShaderParticle2DBase>(get_cengine()->GetRenderContext()));

	return true;
}

bool HDRData::ReloadBloomRenderTarget(uint32_t width)
{
	auto &context = get_cengine()->GetRenderContext();
	context.WaitIdle();

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = ShaderGameWorldLightingPass::RENDER_PASS_FORMAT;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	// imgCreateInfo.samples = sampleCount;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;

	imgCreateInfo.usage
	  = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::TransferSrcBit; // Note: Transfer flag required for debugging purposes only (See debug_glow_bloom console command)
	//if(sampleCount != prosper::SampleCountFlags::e1Bit)
	//	imgCreateInfo.usage |= prosper::ImageUsageFlags::TransferSrcBit;
	imgCreateInfo.usage |= prosper::ImageUsageFlags::TransferSrcBit;

	prosper::util::ImageViewCreateInfo hdrImgViewCreateInfo {};
	// Note: We need the alpha channel for FXAA Luma
	// hdrImgViewCreateInfo.swizzleAlpha = prosper::ComponentSwizzle::One;
	auto hdrSamplerCreateInfo = GetSamplerCreateInfo();

	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;

	auto height = sceneRenderTarget->GetTexture().GetImage().GetHeight();
	// The bloom image has to be blurred multiple times, which is expensive for larger resolutions.
	// We don't really care about the quality of the blur image though, so we're using a smaller
	// version of the bloom image for post-processing.
	auto aspectRatio = width / static_cast<float>(height);
	imgCreateInfo.width = width;
	imgCreateInfo.height = static_cast<uint32_t>(imgCreateInfo.width * aspectRatio);

	if((imgCreateInfo.height % 2) != 0)
		++imgCreateInfo.height;
	auto hdrBloomBlurImg = context.CreateImage(imgCreateInfo);
	auto bloomBlurTexture = context.CreateTexture(texCreateInfo, *hdrBloomBlurImg, hdrImgViewCreateInfo, hdrSamplerCreateInfo);
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	bloomBlurRenderTarget = context.CreateRenderTarget({bloomBlurTexture}, prosper::ShaderGraphics::GetRenderPass<prosper::ShaderBlurBase>(context, math::to_integral(prosper::ShaderBlurBase::Pipeline::R16G16B16A16Sfloat)));
	bloomBlurRenderTarget->SetDebugName("scene_bloom_blur_rt");
	bloomBlurSet = prosper::BlurSet::Create(context, bloomBlurRenderTarget);

	auto &descSetHdrResolve = *dsgBloomTonemapping->GetDescriptorSet();
	descSetHdrResolve.SetBindingTexture(*bloomBlurTexture, math::to_integral(ShaderPPHDR::TextureBinding::Bloom));
	return true;
}

bool HDRData::ResolveRenderPass(const DrawSceneInfo &drawSceneInfo) { return EndRenderPass(drawSceneInfo) && BeginRenderPass(drawSceneInfo, rpPostParticle.get()); }

bool HDRData::InitializeDescriptorSets()
{
	if(ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP.IsValid() == false)
		return false;
	dsgSceneDepth = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP);
	dsgSceneDepth->GetDescriptorSet()->SetBindingTexture(*prepass.textureDepth, 0u);

	//auto &depthTex = prepass.textureDepthSampled;
	//dsgSceneDepth->GetDescriptorSet()->SetBindingTexture(*depthTex,0u);
	return true;
}

bool HDRData::BlitMainDepthBufferToSamplableDepthBuffer(const DrawSceneInfo &drawSceneInfo, std::function<void(prosper::ICommandBuffer &)> &fTransitionSampleImgToTransferDst)
{
#if 0
	auto &srcDepthTex = *prepass.textureDepth;
	auto &srcDepthImg = srcDepthTex.GetImage();

	auto &dstDepthTex = *prepass.textureDepthSampled;
	auto &ptrDstDepthImg = dstDepthTex.GetImage();
	auto &dstDepthImg = ptrDstDepthImg;

	auto &cmdBuffer = *drawSceneInfo.commandBuffer;
	auto r = cmdBuffer.RecordImageBarrier(srcDepthImg,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal) &&
		cmdBuffer.RecordImageBarrier(dstDepthImg,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal) &&
		cmdBuffer.RecordBlitTexture(srcDepthTex,dstDepthImg) &&
		cmdBuffer.RecordImageBarrier(srcDepthImg,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal) &&
		cmdBuffer.RecordImageBarrier(dstDepthImg,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	if(r == false)
		return r;
	//fTransitionSampleImgToTransferDst = [ptrDstDepthImg](prosper::CommandBuffer &cmdBuffer) {
	//	.RecordImageBarrier(*cmdBuffer,ptrDstDepthImg->GetAnvilImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);
	//};
	return r;
#endif
	return false;
}

prosper::RenderTarget &HDRData::GetRenderTarget(const DrawSceneInfo &drawSceneInfo) { return drawSceneInfo.renderTarget ? *drawSceneInfo.renderTarget : *sceneRenderTarget; }

bool HDRData::BlitStagingRenderTargetToMainRenderTarget(const DrawSceneInfo &drawSceneInfo, prosper::ImageLayout srcHdrLayout, prosper::ImageLayout dstHdrLayout)
{
	auto &rt = GetRenderTarget(drawSceneInfo);
	auto &hdrTex = rt.GetTexture();
	auto &cmdBuffer = *drawSceneInfo.commandBuffer;
	auto b = cmdBuffer.RecordImageBarrier(hdrTex.GetImage(), srcHdrLayout, prosper::ImageLayout::TransferDstOptimal);
	if(b == false)
		return false;
	b = cmdBuffer.RecordImageBarrier(hdrPostProcessingRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
	if(b == false)
		return false;
	b = cmdBuffer.RecordBlitTexture(hdrPostProcessingRenderTarget->GetTexture(), rt.GetTexture().GetImage());
	if(b == false)
		return false;
	b = cmdBuffer.RecordImageBarrier(hdrTex.GetImage(), prosper::ImageLayout::TransferDstOptimal, dstHdrLayout);
	if(b == false)
		return false;
	return cmdBuffer.RecordImageBarrier(hdrPostProcessingRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ColorAttachmentOptimal);
}

void HDRData::SwapIOTextures() { m_curTex = (m_curTex == 1u) ? 0u : 1u; }

void HDRData::ResetIOTextureIndex() { m_curTex = 0u; }

void HDRData::UpdateExposure()
{
	auto &avgColor = m_exposure.UpdateColor();
	const auto inc = 0.05f;
	auto lum = 0.2126f * avgColor.r + 0.7152f * avgColor.g + 0.0722f * avgColor.b;
	if(lum > 0.f) {
		exposure = math::lerp(exposure, 0.5f / lum, inc);
		exposure = math::min(exposure, max_exposure);
	}
	exposure = math::lerp(exposure, 1.f, inc);
}

static auto cvAntiAliasing = pragma::console::get_client_con_var("cl_render_anti_aliasing");
static auto cvMsaaSamples = pragma::console::get_client_con_var("cl_render_msaa_samples");
static void CVAR_CALLBACK_render_msaa_enabled(pragma::NetworkState *, const pragma::console::ConVar &, int, int)
{
	if(pragma::get_cgame() == nullptr)
		return;
	auto bMsaaEnabled = static_cast<AntiAliasing>(cvAntiAliasing->GetInt()) == AntiAliasing::MSAA;
	auto samples = bMsaaEnabled ? static_cast<uint32_t>(pragma::math::pow(2, cvMsaaSamples->GetInt())) : 0u;
	auto err = ClampMSAASampleCount(&samples);
	switch(err) {
	case 1:
		{
			auto maxSamples = GetMaxMSAASampleCount();
			Con::CWAR << "Your hardware doesn't support a sample count above " << samples << "! Clamping to " << maxSamples << "..." << Con::endl;
			break;
		}
	case 2:
		Con::CWAR << "Sample count has to be a power of two! Clamping to " << samples << "..." << Con::endl;
		break;
	}
	pragma::ShaderScene::SetRenderPassSampleCount(static_cast<prosper::SampleCountFlags>(samples));
	// Reload entity shader pipelines
	auto &shaderManager = pragma::get_cengine()->GetShaderManager();
	for(auto &hShader : shaderManager.GetShaders()) {
		auto *shader = dynamic_cast<pragma::ShaderScene *>(hShader.get());
		if(shader == nullptr)
			continue;
		shader->ReloadPipelines();
	}
	pragma::ecs::EntityIterator entIt {*pragma::get_cgame(), pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CSceneComponent>>();
	for(auto *ent : entIt) {
		auto sceneC = ent->GetComponent<pragma::CSceneComponent>();
		sceneC->ReloadRenderTarget(sceneC->GetWidth(), sceneC->GetHeight());
	}
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("cl_render_anti_aliasing", &CVAR_CALLBACK_render_msaa_enabled);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("cl_render_msaa_samples", &CVAR_CALLBACK_render_msaa_enabled);
}

static void CVAR_CALLBACK_render_bloom_resolution(pragma::NetworkState *, const pragma::console::ConVar &, int, int width)
{
	if(pragma::get_cgame() == nullptr)
		return;
	for(auto &c : EntityCIterator<pragma::CRendererComponent> {*pragma::get_cgame()})
		c.ReloadBloomRenderTarget(width);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("render_bloom_resolution", &CVAR_CALLBACK_render_bloom_resolution);
}

static void debug_render_scene(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg != nullptr) {
		dbg = nullptr;
		return;
	}
	dbg = nullptr;
	if(pragma::get_cgame() == nullptr)
		return;
	auto size = 256u;
	if(argv.empty() == false)
		size = pragma::util::to_float(argv.front());
	static pragma::gui::WIHandle hTexture = {};
	static pragma::gui::WIHandle hBloomTexture = {};
	dbg = std::make_unique<DebugGameGUI>([size]() {
		auto *scene = pragma::get_cgame()->GetScene<pragma::CSceneComponent>();
		auto *renderer = scene ? scene->GetRenderer<pragma::CRendererComponent>() : nullptr;
		auto raster = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};

		if(raster.expired())
			return pragma::gui::WIHandle {};
		auto &hdrInfo = raster->GetHDRInfo();

		auto &wgui = pragma::gui::WGUI::GetInstance();
		auto *r = wgui.Create<pragma::gui::types::WIBase>();

		auto idx = 0u;
		if(hdrInfo.sceneRenderTarget != nullptr) {
			auto *pTexture = wgui.Create<pragma::gui::types::WIDebugMSAATexture>(r);
			pTexture->SetSize(size, size);
			pTexture->SetX(size * idx++);
			pTexture->SetTexture(hdrInfo.sceneRenderTarget->GetTexture());
			pTexture->SetShouldResolveImage(true);

			hTexture = pTexture->GetHandle();
		}
		if(hdrInfo.bloomBlurRenderTarget != nullptr) {
			auto *pTexture = wgui.Create<pragma::gui::types::WIDebugMSAATexture>(r);
			pTexture->SetSize(size, size);
			pTexture->SetX(size * idx++);
			pTexture->SetTexture(hdrInfo.bloomBlurRenderTarget->GetTexture());
			pTexture->SetShouldResolveImage(true);

			hBloomTexture = pTexture->GetHandle();
		}
		r->SetSize(size * idx, size);
		return r->GetHandle();
	});
	dbg->AddCallback("PostRenderScene", FunctionCallback<void, std::reference_wrapper<const DrawSceneInfo>>::Create([](std::reference_wrapper<const DrawSceneInfo> drawSceneInfo) {
		if(hTexture.IsValid() == true)
			static_cast<pragma::gui::types::WIDebugMSAATexture *>(hTexture.get())->Update();
		if(hBloomTexture.IsValid() == true)
			static_cast<pragma::gui::types::WIDebugMSAATexture *>(hBloomTexture.get())->Update();
	}));
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_render_scene", &debug_render_scene, pragma::console::ConVarFlags::None, "Displays scene to screen.");
}
