/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/c_msaa.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/shaders/image/c_shader_calc_image_color.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fxaa.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/gui/widebugmsaatexture.hpp"
#include "pragma/rendering/c_settings.hpp"
#include <textureinfo.h>
#include <wgui/wgui.h>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_fence.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <image/prosper_sampler.hpp>

using namespace pragma::rendering;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static const float EXPOSURE_FRAME_UPDATE = 0.25f; // Exposure will be updated every x seconds

static CVar cvMaxExposure = GetClientConVar("render_hdr_max_exposure");
static void CVAR_CALLBACK_render_hdr_max_exposure(NetworkState*,ConVar*,float,float val)
{
	if(c_game == nullptr)
		return;
	c_game->SetMaxHDRExposure(val);
}
REGISTER_CONVAR_CALLBACK_CL(render_hdr_max_exposure,CVAR_CALLBACK_render_hdr_max_exposure);

HDRData::Exposure::Exposure()
	: lastExposureUpdate(0),averageColor(0.f,0.f,0.f)
{
	m_shaderCalcColor = c_engine->GetShader("calcimagecolor");
}

bool HDRData::Exposure::Initialize(prosper::Texture &texture)
{
	m_exposureColorSource = texture.shared_from_this();
	if(pragma::ShaderCalcImageColor::DESCRIPTOR_SET_COLOR.IsValid() == false || pragma::ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE.IsValid() == false)
		return false;
	m_calcImgColorCmdBuffer = c_engine->GetRenderContext().AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType::Compute,m_cmdBufferQueueFamilyIndex);
	m_calcImgColorFence = c_engine->GetRenderContext().CreateFence();

	descSetGroupAverageColorBuffer = nullptr; // Has to be cleared first! (To release any previous descriptor sets)
	
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit;
	createInfo.size = sizeof(Vector4);
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	avgColorBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo);
	avgColorBuffer->SetDebugName("avg_color_buf");
	descSetGroupAverageColorBuffer = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderCalcImageColor::DESCRIPTOR_SET_COLOR);
	descSetGroupAverageColorBuffer->GetDescriptorSet()->SetBindingStorageBuffer(*avgColorBuffer,0u);
	avgColorBuffer->SetPermanentlyMapped(true);

	descSetGroupAverageColorTexture = nullptr; // Has to be cleared first! (To release any previous descriptor sets)
	descSetGroupAverageColorTexture = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE);
	descSetGroupAverageColorTexture->GetDescriptorSet()->SetBindingTexture(
		texture,0u
	);
	return true;
}

const Vector3 &HDRData::Exposure::UpdateColor()
{
	static auto skip = true;
	if(skip)
		return averageColor;
	if(m_shaderCalcColor.expired() || m_calcImgColorFence == nullptr || m_exposureColorSource.expired())
		return averageColor;
	auto &shader = static_cast<pragma::ShaderCalcImageColor&>(*m_shaderCalcColor.get());
	auto t = c_game->RealTime();
	auto bFenceSet = m_calcImgColorFence->IsSet();
	if(m_bWaitingForResult == false)
	{
		if(t -lastExposureUpdate >= EXPOSURE_FRAME_UPDATE)
		{
			auto &primCmd = m_calcImgColorCmdBuffer;
			primCmd->StartRecording(false,true);
			auto &imgSrc = m_exposureColorSource.lock()->GetImage();
			m_calcImgColorCmdBuffer->RecordImageBarrier(imgSrc,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
			if(shader.BeginCompute(m_calcImgColorCmdBuffer) == true)
			{
				const auto sampleCount = 32u;
				shader.Compute(
					*descSetGroupAverageColorTexture->GetDescriptorSet(),
					*descSetGroupAverageColorBuffer->GetDescriptorSet(),
					sampleCount
				);
				shader.EndCompute();
			}
			m_calcImgColorCmdBuffer->RecordImageBarrier(imgSrc,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
			primCmd->StopRecording();

			lastExposureUpdate = t;
			m_bWaitingForResult = true;
			// Issue new query
			c_engine->GetRenderContext().SubmitCommandBuffer(*m_calcImgColorCmdBuffer,false,m_calcImgColorFence.get());
		}
	}
	else if(bFenceSet == true) // Wait until computation has been completed before retrieving the color
	{
		m_bWaitingForResult = false;
		m_calcImgColorFence->Reset();

		Vector4 color(0.f,0.f,0.f,0.f);
		avgColorBuffer->Read(0ull,color);
		averageColor = Vector3(color.r,color.g,color.b);
	}
	return averageColor;
}

/////////////////////////////////

HDRData::HDRData(RasterizationRenderer &rasterizer)
	: exposure(1.f),m_bMipmapInitialized(false),m_exposure{},m_cbReloadCommandBuffer{},
	forwardPlusInstance{rasterizer}
{
	max_exposure = cvMaxExposure->GetFloat();
}

HDRData::~HDRData()
{
	if(m_cbReloadCommandBuffer.IsValid())
		m_cbReloadCommandBuffer.Remove();
}

bool HDRData::BeginRenderPass(const util::DrawSceneInfo &drawSceneInfo,prosper::IRenderPass *customRenderPass)
{
	auto &rt = GetRenderTarget(drawSceneInfo);
	return drawSceneInfo.commandBuffer->RecordBeginRenderPass(rt,{
		prosper::ClearValue{}, // Unused
		prosper::ClearValue{prosper::ClearColorValue{std::array<float,4>{0.f,0.f,0.f,0.f}}} // Clear bloom
	},customRenderPass);
}
bool HDRData::EndRenderPass(const util::DrawSceneInfo &drawSceneInfo)
{
	return drawSceneInfo.commandBuffer->RecordEndRenderPass();
}

bool HDRData::Initialize(RasterizationRenderer &renderer,uint32_t width,uint32_t height,prosper::SampleCountFlags sampleCount,bool bEnableSSAO)
{
	// Initialize depth prepass
	auto &context = c_engine->GetRenderContext();
	if(prepass.Initialize(context,width,height,sampleCount,bEnableSSAO) == false)
		return false;

	auto wpShader = c_engine->GetShader("pbr");
	auto &hShaderTonemapping = c_game->GetGameShader(CGame::GameShader::PPTonemapping);
	if(
		wpShader.expired() || hShaderTonemapping.expired() || 
		pragma::ShaderPPHDR::DESCRIPTOR_SET_TEXTURE.IsValid() == false || pragma::ShaderPPFog::DESCRIPTOR_SET_TEXTURE.IsValid() == false || 
		pragma::ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER.IsValid() == false
		)
		return false;

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = pragma::ShaderTextured3DBase::RENDER_PASS_FORMAT;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	imgCreateInfo.samples = sampleCount;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	auto hdrImg = context.CreateImage(imgCreateInfo);

	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::TransferSrcBit; // Note: Transfer flag required for debugging purposes only (See debug_glow_bloom console command)
	if(sampleCount != prosper::SampleCountFlags::e1Bit)
		imgCreateInfo.usage |= prosper::ImageUsageFlags::TransferSrcBit;

	// The bloom image has to be blurred multiple times, which is expensive for larger resolutions.
	// We don't really care about the quality of the blur image though, so using a smaller resolution
	// here works just as well.
	auto hdrBloomImg = context.CreateImage(imgCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;

	prosper::util::ImageViewCreateInfo hdrImgViewCreateInfo {};
	// Note: We need the alpha channel for FXAA Luma
	// hdrImgViewCreateInfo.swizzleAlpha = prosper::ComponentSwizzle::One;
	prosper::util::SamplerCreateInfo hdrSamplerCreateInfo {};
	hdrSamplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	hdrSamplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	hdrSamplerCreateInfo.minFilter = prosper::Filter::Linear; // Note: These have to be linear for FXAA!
	hdrSamplerCreateInfo.magFilter = prosper::Filter::Linear;
	auto hdrTex = context.CreateTexture(texCreateInfo,*hdrImg,hdrImgViewCreateInfo,hdrSamplerCreateInfo);
	auto resolvedTex = hdrTex;
	if(resolvedTex->IsMSAATexture())
		resolvedTex = static_cast<prosper::MSAATexture&>(*resolvedTex).GetResolvedTexture();

	bloomTexture = context.CreateTexture(texCreateInfo,*hdrBloomImg,hdrImgViewCreateInfo,hdrSamplerCreateInfo);
	sceneRenderTarget = c_engine->GetRenderContext().CreateRenderTarget(
		{hdrTex,bloomTexture,prepass.textureDepth},
		static_cast<prosper::ShaderGraphics*>(wpShader.get())->GetRenderPass(umath::to_integral(pragma::ShaderTextured3DBase::GetPipelineIndex(sampleCount)))
	);
	sceneRenderTarget->SetDebugName("scene_hdr_rt");
	auto resolvedBloomTex = bloomTexture;
	if(resolvedBloomTex->IsMSAATexture())
		resolvedBloomTex = static_cast<prosper::MSAATexture&>(*resolvedBloomTex).GetResolvedTexture();

	// Intermediate render pass
	auto &imgDepth = prepass.textureDepth->GetImage();
	prosper::util::RenderPassCreateInfo rpCreateInfo {};
	rpCreateInfo.attachments.push_back({hdrImg->GetFormat(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::Load,prosper::AttachmentStoreOp::Store,hdrImg->GetSampleCount(),prosper::ImageLayout::ColorAttachmentOptimal});
	rpCreateInfo.attachments.push_back({hdrBloomImg->GetFormat(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::AttachmentLoadOp::Load,prosper::AttachmentStoreOp::Store,hdrBloomImg->GetSampleCount(),prosper::ImageLayout::ColorAttachmentOptimal});
	rpCreateInfo.attachments.push_back({imgDepth.GetFormat(),prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AttachmentLoadOp::Load,prosper::AttachmentStoreOp::Store,imgDepth.GetSampleCount(),prosper::ImageLayout::DepthStencilAttachmentOptimal});
	rpPostParticle = context.CreateRenderPass(rpCreateInfo);

	// The bloom image has to be blurred multiple times, which is expensive for larger resolutions.
	// We don't really care about the quality of the blur image though, so we're using a smaller
	// version of the bloom image for post-processing.
	imgCreateInfo.width = 1024;
	imgCreateInfo.height = 1024;
	auto hdrBloomBlurImg = context.CreateImage(imgCreateInfo);
	auto bloomBlurTexture = context.CreateTexture(texCreateInfo,*hdrBloomBlurImg,hdrImgViewCreateInfo,hdrSamplerCreateInfo);
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	bloomBlurRenderTarget = context.CreateRenderTarget({bloomBlurTexture},prosper::ShaderGraphics::GetRenderPass<prosper::ShaderBlurBase>(context,umath::to_integral(prosper::ShaderBlurBase::Pipeline::R16G16B16A16Sfloat)));
	bloomBlurRenderTarget->SetDebugName("scene_bloom_rt");
	bloomBlurSet = prosper::BlurSet::Create(c_engine->GetRenderContext(),bloomBlurRenderTarget);

	imgCreateInfo.usage |= prosper::ImageUsageFlags::TransferSrcBit;
	auto hdrImgStaging = context.CreateImage(imgCreateInfo);
	auto hdrTexStaging = context.CreateTexture(texCreateInfo,*hdrImgStaging,hdrImgViewCreateInfo,hdrSamplerCreateInfo);
	hdrPostProcessingRenderTarget = context.CreateRenderTarget({hdrTexStaging},prosper::ShaderGraphics::GetRenderPass<pragma::ShaderPPBase>(context));
	hdrPostProcessingRenderTarget->SetDebugName("scene_staging_rt");

	dsgBloomTonemapping = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPPHDR::DESCRIPTOR_SET_TEXTURE);
	auto &descSetHdrResolve = *dsgBloomTonemapping->GetDescriptorSet();
	descSetHdrResolve.SetBindingTexture(*resolvedTex,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
	descSetHdrResolve.SetBindingTexture(*bloomBlurTexture,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Bloom));

	dsgHDRPostProcessing = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPPFog::DESCRIPTOR_SET_TEXTURE);
	auto &descSetHdr = *dsgHDRPostProcessing->GetDescriptorSet();
	dsgHDRPostProcessing->GetDescriptorSet()->SetBindingTexture(*resolvedTex,0u);

	imgCreateInfo.format = pragma::ShaderPPHDR::RENDER_PASS_FORMAT;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	imgCreateInfo.samples = prosper::SampleCountFlags::e1Bit;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::ColorAttachmentOptimal;
	auto postHdrImg = context.CreateImage(imgCreateInfo);
	auto postHdrTex = context.CreateTexture(texCreateInfo,*postHdrImg,hdrImgViewCreateInfo,hdrSamplerCreateInfo);
	toneMappedRenderTarget = context.CreateRenderTarget({postHdrTex},static_cast<prosper::ShaderGraphics*>(hShaderTonemapping.get())->GetRenderPass());
	toneMappedRenderTarget->SetDebugName("scene_post_hdr_rt");
	dsgTonemappedPostProcessing = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPPFXAA::DESCRIPTOR_SET_TEXTURE);
	dsgTonemappedPostProcessing->GetDescriptorSet()->SetBindingTexture(*postHdrTex,0u);
	dsgTonemappedPostProcessing->GetDescriptorSet()->SetBindingTexture(*resolvedTex,1u);

	{
		auto hShaderFXAA = c_engine->GetShader("pp_fxaa");
		if(hShaderFXAA.valid())
		{
			// TODO: Obsolete?
			imgCreateInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::TransferSrcBit;
			auto tmPpImg = context.CreateImage(imgCreateInfo);
			auto tmPpTex = context.CreateTexture(texCreateInfo,*tmPpImg,hdrImgViewCreateInfo,hdrSamplerCreateInfo);
			toneMappedPostProcessingRenderTarget = context.CreateRenderTarget({tmPpTex},static_cast<prosper::ShaderGraphics*>(hShaderFXAA.get())->GetRenderPass());
			toneMappedPostProcessingRenderTarget->SetDebugName("scene_post_tonemapping_post_processing_rt");
			dsgToneMappedPostProcessing = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPPFXAA::DESCRIPTOR_SET_TEXTURE);
			dsgToneMappedPostProcessing->GetDescriptorSet()->SetBindingTexture(*tmPpTex,0u);
		}
	}

	if(m_exposure.Initialize(*resolvedTex) == false)
		return false;

	// Initialize SSAO
	if(bEnableSSAO == true)
	{
		auto texNormals = prepass.textureNormals;
		if(texNormals->IsMSAATexture())
			texNormals = static_cast<prosper::MSAATexture&>(*texNormals).GetResolvedTexture();
		auto texDepth = prepass.textureDepth;
		if(texDepth->IsMSAATexture())
			texDepth = static_cast<prosper::MSAATexture&>(*texDepth).GetResolvedTexture();
		if(ssaoInfo.Initialize(context,width,height,sampleCount,texNormals,texDepth) == false)
			return false;
	}

	// Initialize forward+
	auto &scene = renderer.GetScene();
	auto resolvedTexture = prepass.textureDepth->IsMSAATexture() ? static_cast<prosper::MSAATexture&>(*prepass.textureDepth).GetResolvedTexture() : prepass.textureDepth;
	if(forwardPlusInstance.Initialize(context,width,height,*resolvedTexture) == false)
		return false;

	dsgDepthPostProcessing = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER);
	dsgDepthPostProcessing->GetDescriptorSet()->SetBindingTexture(*resolvedTexture,0u);
	return true;
}

bool HDRData::ResolveRenderPass(const util::DrawSceneInfo &drawSceneInfo)
{
	return EndRenderPass(drawSceneInfo) && BeginRenderPass(drawSceneInfo,rpPostParticle.get());
}

bool HDRData::InitializeDescriptorSets()
{
	if(pragma::ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP.IsValid() == false)
		return false;
	dsgSceneDepth = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP);

	auto &depthTex = prepass.textureDepthSampled;
	dsgSceneDepth->GetDescriptorSet()->SetBindingTexture(*depthTex,0u);
	return true;
}

bool HDRData::BlitMainDepthBufferToSamplableDepthBuffer(const util::DrawSceneInfo &drawSceneInfo,std::function<void(prosper::ICommandBuffer&)> &fTransitionSampleImgToTransferDst)
{
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
}

prosper::RenderTarget &HDRData::GetRenderTarget(const util::DrawSceneInfo &drawSceneInfo)
{
	return drawSceneInfo.renderTarget ? *drawSceneInfo.renderTarget : *sceneRenderTarget;
}

bool HDRData::BlitStagingRenderTargetToMainRenderTarget(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &rt = GetRenderTarget(drawSceneInfo);
	auto &hdrTex = rt.GetTexture();
	auto &cmdBuffer = *drawSceneInfo.commandBuffer;
	auto b = cmdBuffer.RecordImageBarrier(hdrTex.GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);
	if(b == false)
		return false;
	b = cmdBuffer.RecordImageBarrier(hdrPostProcessingRenderTarget->GetTexture().GetImage(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal);
	if(b == false)
		return false;
	b = cmdBuffer.RecordBlitTexture(
		hdrPostProcessingRenderTarget->GetTexture(),rt.GetTexture().GetImage()
	);
	if(b == false)
		return false;
	b = cmdBuffer.RecordImageBarrier(hdrTex.GetImage(),prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	if(b == false)
		return false;
	return cmdBuffer.RecordImageBarrier(hdrPostProcessingRenderTarget->GetTexture().GetImage(),prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
}

void HDRData::SwapIOTextures() {m_curTex = (m_curTex == 1u) ? 0u : 1u;}

void HDRData::ResetIOTextureIndex() {m_curTex = 0u;}

void HDRData::UpdateExposure()
{
	auto &avgColor = m_exposure.UpdateColor();
	const auto inc = 0.05f;
	auto lum = 0.2126f *avgColor.r +0.7152f *avgColor.g +0.0722f *avgColor.b;
	if(lum > 0.f)
	{
		exposure = umath::lerp(exposure,0.5f /lum,inc);
		exposure = umath::min(exposure,max_exposure);
	}
	exposure = umath::lerp(exposure,1.f,inc);
}

static auto cvAntiAliasing = GetClientConVar("cl_render_anti_aliasing");
static auto cvMsaaSamples = GetClientConVar("cl_render_msaa_samples");
static void CVAR_CALLBACK_render_msaa_enabled(NetworkState*,ConVar*,int,int)
{
	if(c_game == nullptr)
		return;
	auto bMsaaEnabled = static_cast<AntiAliasing>(cvAntiAliasing->GetInt()) == AntiAliasing::MSAA;
	auto samples = bMsaaEnabled ? static_cast<uint32_t>(umath::pow(2,cvMsaaSamples->GetInt())) : 0u;
	auto err = ClampMSAASampleCount(&samples);
	switch(err)
	{
	case 1:
	{
		auto maxSamples = GetMaxMSAASampleCount();
		Con::cwar<<"WARNING: Your hardware doesn't support a sample count above "<<samples<<"! Clamping to "<<maxSamples<<"..."<<Con::endl;
		break;
	}
	case 2:
		Con::cwar<<"WARNING: Sample count has to be a power of two! Clamping to "<<samples<<"..."<<Con::endl;
		break;
	}
	pragma::ShaderScene::SetRenderPassSampleCount(static_cast<prosper::SampleCountFlags>(samples));
	// Reload entity shader pipelines
	auto &shaderManager = c_engine->GetShaderManager();
	for(auto &pair : shaderManager.GetShaders())
	{
		auto *shader = dynamic_cast<pragma::ShaderScene*>(pair.second.get());
		if(shader == nullptr)
			continue;
		shader->ReloadPipelines();
	}
	c_game->ReloadRenderFrameBuffer();
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_anti_aliasing,CVAR_CALLBACK_render_msaa_enabled);
REGISTER_CONVAR_CALLBACK_CL(cl_render_msaa_samples,CVAR_CALLBACK_render_msaa_enabled);

void Console::commands::debug_render_scene(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
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
	auto size = 256u;
	if(argv.empty() == false)
		size = util::to_float(argv.front());
	static WIHandle hTexture = {};
	static WIHandle hBloomTexture = {};
	dbg = std::make_unique<DebugGameGUI>([size]() {
		auto &scene = c_game->GetScene();
		auto *renderer = scene->GetRenderer();
		if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
			return WIHandle{};
		auto &hdrInfo = static_cast<pragma::rendering::RasterizationRenderer*>(renderer)->GetHDRInfo();

		auto &wgui = WGUI::GetInstance();
		auto *r = wgui.Create<WIBase>();

		auto idx = 0u;
		if(hdrInfo.sceneRenderTarget != nullptr)
		{
			auto *pTexture = wgui.Create<WIDebugMSAATexture>(r);
			pTexture->SetSize(size,size);
			pTexture->SetX(size *idx++);
			pTexture->SetTexture(hdrInfo.sceneRenderTarget->GetTexture());
			pTexture->SetShouldResolveImage(true);

			hTexture = pTexture->GetHandle();
		}
		if(hdrInfo.bloomBlurRenderTarget != nullptr)
		{
			auto *pTexture = wgui.Create<WIDebugMSAATexture>(r);
			pTexture->SetSize(size,size);
			pTexture->SetX(size *idx++);
			pTexture->SetTexture(hdrInfo.bloomBlurRenderTarget->GetTexture());
			pTexture->SetShouldResolveImage(true);

			hBloomTexture = pTexture->GetHandle();
		}
		r->SetSize(size *idx,size);
		return r->GetHandle();
		});
	dbg->AddCallback("PostRenderScene",FunctionCallback<void,std::reference_wrapper<const util::DrawSceneInfo>>::Create([](std::reference_wrapper<const util::DrawSceneInfo> drawSceneInfo) {
		if(hTexture.IsValid() == true)
			static_cast<WIDebugMSAATexture*>(hTexture.get())->Update();
		if(hBloomTexture.IsValid() == true)
			static_cast<WIDebugMSAATexture*>(hBloomTexture.get())->Update();
		}));
}
