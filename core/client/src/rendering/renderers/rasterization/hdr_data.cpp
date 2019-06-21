#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/uniformbinding.h"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/c_msaa.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/shaders/image/c_shader_calc_image_color.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/shaders/image/c_shader_additive.h"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
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
	m_calcImgColorCmdBuffer = c_engine->AllocatePrimaryLevelCommandBuffer(Anvil::QueueFamilyType::COMPUTE,m_cmdBufferQueueFamilyIndex);
	m_calcImgColorFence = prosper::Fence::Create(*c_engine);

	descSetGroupAverageColorBuffer = nullptr; // Has to be cleared first! (To release any previous descriptor sets)
	auto &dev = c_engine->GetDevice();
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT;
	createInfo.size = sizeof(Vector4);
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUToCPU;
	avgColorBuffer = prosper::util::create_buffer(dev,createInfo);
	avgColorBuffer->SetDebugName("avg_color_buf");
	descSetGroupAverageColorBuffer = prosper::util::create_descriptor_set_group(dev,pragma::ShaderCalcImageColor::DESCRIPTOR_SET_COLOR);
	prosper::util::set_descriptor_set_binding_storage_buffer(*(*descSetGroupAverageColorBuffer)->get_descriptor_set(0u),*avgColorBuffer,0u);
	avgColorBuffer->SetPermanentlyMapped(true);

	descSetGroupAverageColorTexture = nullptr; // Has to be cleared first! (To release any previous descriptor sets)
	descSetGroupAverageColorTexture = prosper::util::create_descriptor_set_group(dev,pragma::ShaderCalcImageColor::DESCRIPTOR_SET_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(
		*(*descSetGroupAverageColorTexture)->get_descriptor_set(0u),
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
			auto &primCmd = m_calcImgColorCmdBuffer->GetAnvilCommandBuffer();
			primCmd.start_recording(false,true);
			auto &imgSrc = *m_exposureColorSource.lock()->GetImage();
			prosper::util::record_image_barrier(**m_calcImgColorCmdBuffer,*imgSrc,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
			if(shader.BeginCompute(m_calcImgColorCmdBuffer) == true)
			{
				const auto sampleCount = 32u;
				shader.Compute(
					*(*descSetGroupAverageColorTexture)->get_descriptor_set(0u),
					*(*descSetGroupAverageColorBuffer)->get_descriptor_set(0u),
					sampleCount
				);
				shader.EndCompute();
			}
			prosper::util::record_image_barrier(**m_calcImgColorCmdBuffer,*imgSrc,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
			primCmd.stop_recording();

			lastExposureUpdate = t;
			m_bWaitingForResult = true;
			// Issue new query
			c_engine->SubmitCommandBuffer(*m_calcImgColorCmdBuffer,false,&m_calcImgColorFence->GetAnvilFence());
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

bool HDRData::BeginRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::RenderPass *customRenderPass)
{
	return prosper::util::record_begin_render_pass(*(*drawCmd),*hdrRenderTarget,{
		vk::ClearValue{}, // Unused
		vk::ClearValue{vk::ClearColorValue{std::array<float,4>{0.f,0.f,0.f,0.f}}} // Clear bloom
		},customRenderPass);
}
bool HDRData::EndRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	return prosper::util::record_end_render_pass(*(*drawCmd));
}

bool HDRData::ResolveRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	return EndRenderPass(drawCmd) && BeginRenderPass(drawCmd,rpIntermediate.get());
}

void HDRData::InitializeCommandBuffer()
{
	// prosper TODO
	/*
	static auto hShaderBlur = c_engine->GetShader("pp_gaussianblur");
	if(hShaderBlur.IsValid())
	{
	m_cbReloadCommandBuffer = FunctionCallback<void,Shader::Base*>::Create([this](Shader::Base *shader) {
	//auto &context = c_engine->GetRenderContext();

	auto &bloomBlurTexture = GetTargetBloomTexture();
	blurBuffer.tmpBlurTexture->GetImage()->SetInternalLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	bloomBlurTexture->GetImage()->SetInternalLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	//cmdBufferBloom = Vulkan::CommandBuffer::Create(context,vk::CommandBufferLevel::eSecondary);
	//cmdBufferBloom->Begin();
	auto hShader = shader->GetHandle();
	cmdBufferBloom = [this,hShader](Vulkan::CommandBufferObject *cmd) {
	const auto blurSize = 5.f;
	const auto kernelSize = 9;
	const auto blurAmount = 5;

	auto &bloomBlurTexture = GetTargetBloomTexture();
	auto &shaderBlur = static_cast<Shader::GaussianBlur&>(*hShader.get());
	auto &bloomTex = bloomBlurTexture;
	auto &bloomImg = bloomTex->GetImage();
	for(auto i=decltype(blurAmount){0};i<blurAmount;++i)
	{
	shaderBlur.Render(
	cmd,renderPassSwap,Vector2i(bloomImg->GetWidth(),bloomImg->GetHeight()),
	bloomTex,blurBuffer,blurFramebuffer,blurSize,kernelSize
	);
	}
	};
	//cmdBufferBloom->End();
	});
	m_cbReloadCommandBuffer(hShaderBlur.get());
	hShaderBlur->CallOnReload(m_cbReloadCommandBuffer);
	}*/
}

bool HDRData::Initialize(RasterizationRenderer &renderer,uint32_t width,uint32_t height,Anvil::SampleCountFlagBits sampleCount,bool bEnableSSAO)
{
	//texture = nullptr; // prosper TODO

	//auto &context = c_engine->GetRenderContext();
	//auto &drawCmd = context.GetDrawCmd();
	//auto format = Anvil::Format::R8G8B8A8_UNORM;

	// Initialize depth prepass
	auto &context = *c_engine;
	if(prepass.Initialize(context,width,height,sampleCount,bEnableSSAO) == false)
		return false;

	/*auto depthFormat = Anvil::Format::D32_SFLOAT;
	auto initSampler = std::function<void(vk::SamplerCreateInfo&)>([](vk::SamplerCreateInfo &info) {
	info.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	info.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	});*/ // prosper TODO

	auto wpShader = c_engine->GetShader("textured");
	shaderPPHdr = c_engine->GetShader("pp_hdr");
	if(
		wpShader.expired() || shaderPPHdr.expired() || 
		pragma::ShaderPPHDR::DESCRIPTOR_SET_TEXTURE.IsValid() == false || pragma::ShaderPPFog::DESCRIPTOR_SET_TEXTURE.IsValid() == false || 
		pragma::ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER.IsValid() == false
		)
		return false;

	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = pragma::ShaderTextured3DBase::RENDER_PASS_FORMAT;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
	imgCreateInfo.samples = sampleCount;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	auto hdrImg = prosper::util::create_image(dev,imgCreateInfo);

	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT; // Note: Transfer flag required for debugging purposes only (See debug_glow_bloom console command)
	if(sampleCount != Anvil::SampleCountFlagBits::_1_BIT)
		imgCreateInfo.usage |= Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;

	// The bloom image has to be blurred multiple times, which is expensive for larger resolutions.
	// We don't really care about the quality of the blur image though, so using a smaller resolution
	// here works just as well.
	auto hdrBloomImg = prosper::util::create_image(dev,imgCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;

	prosper::util::ImageViewCreateInfo hdrImgViewCreateInfo {};
	prosper::util::SamplerCreateInfo hdrSamplerCreateInfo {};
	hdrSamplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	hdrSamplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	hdrSamplerCreateInfo.minFilter = Anvil::Filter::LINEAR; // Note: These have to be linear for FXAA!
	hdrSamplerCreateInfo.magFilter = Anvil::Filter::LINEAR;
	auto hdrTex = prosper::util::create_texture(dev,texCreateInfo,hdrImg,&hdrImgViewCreateInfo,&hdrSamplerCreateInfo);
	auto resolvedTex = hdrTex;
	if(resolvedTex->IsMSAATexture())
		resolvedTex = static_cast<prosper::MSAATexture&>(*resolvedTex).GetResolvedTexture();

	bloomTexture = prosper::util::create_texture(dev,texCreateInfo,hdrBloomImg,&hdrImgViewCreateInfo,&hdrSamplerCreateInfo);
	hdrRenderTarget = prosper::util::create_render_target(
		dev,{hdrTex,bloomTexture,prepass.textureDepth},
		static_cast<prosper::ShaderGraphics*>(wpShader.get())->GetRenderPass(umath::to_integral(pragma::ShaderTextured3D::GetPipelineIndex(sampleCount)))
	);
	hdrRenderTarget->SetDebugName("scene_hdr_rt");
	auto resolvedBloomTex = bloomTexture;
	if(resolvedBloomTex->IsMSAATexture())
		resolvedBloomTex = static_cast<prosper::MSAATexture&>(*resolvedBloomTex).GetResolvedTexture();

	// Intermediate render pass
	auto &imgDepth = prepass.textureDepth->GetImage();
	prosper::util::RenderPassCreateInfo rpCreateInfo {};
	rpCreateInfo.attachments.push_back({hdrImg->GetFormat(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,Anvil::AttachmentStoreOp::STORE,hdrImg->GetSampleCount(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL});
	rpCreateInfo.attachments.push_back({hdrBloomImg->GetFormat(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,Anvil::AttachmentStoreOp::STORE,hdrBloomImg->GetSampleCount(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL});
	rpCreateInfo.attachments.push_back({imgDepth->GetFormat(),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::LOAD,Anvil::AttachmentStoreOp::STORE,imgDepth->GetSampleCount(),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
	rpIntermediate = prosper::util::create_render_pass(dev,rpCreateInfo);

	// The bloom image has to be blurred multiple times, which is expensive for larger resolutions.
	// We don't really care about the quality of the blur image though, so we're using a smaller
	// version of the bloom image for post-processing.
	imgCreateInfo.width = 1024;
	imgCreateInfo.height = 1024;
	auto hdrBloomBlurImg = prosper::util::create_image(dev,imgCreateInfo);
	bloomBlurTexture = prosper::util::create_texture(dev,texCreateInfo,hdrBloomBlurImg,&hdrImgViewCreateInfo,&hdrSamplerCreateInfo);
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	bloomBlurRenderTarget = prosper::util::create_render_target(dev,{bloomBlurTexture},prosper::ShaderGraphics::GetRenderPass<prosper::ShaderBlurBase>(context,umath::to_integral(prosper::ShaderBlurBase::Pipeline::R16G16B16A16Sfloat)));
	bloomBlurRenderTarget->SetDebugName("scene_bloom_rt");
	bloomBlurSet = prosper::BlurSet::Create(dev,bloomBlurRenderTarget);

	imgCreateInfo.usage |= Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
	auto hdrImgStaging = prosper::util::create_image(dev,imgCreateInfo);
	auto hdrTexStaging = prosper::util::create_texture(dev,texCreateInfo,hdrImgStaging,&hdrImgViewCreateInfo,&hdrSamplerCreateInfo);
	hdrStagingRenderTarget = prosper::util::create_render_target(dev,{hdrTexStaging},prosper::ShaderGraphics::GetRenderPass<pragma::ShaderPPBase>(context));
	hdrStagingRenderTarget->SetDebugName("scene_staging_rt");

	descSetGroupHdrResolve = prosper::util::create_descriptor_set_group(dev,pragma::ShaderPPHDR::DESCRIPTOR_SET_TEXTURE);
	auto &descSetHdrResolve = *(*descSetGroupHdrResolve)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_texture(descSetHdrResolve,*resolvedTex,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
	prosper::util::set_descriptor_set_binding_texture(descSetHdrResolve,*bloomBlurTexture,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Bloom));

	descSetGroupHdrResolveStaging = prosper::util::create_descriptor_set_group(dev,pragma::ShaderPPHDR::DESCRIPTOR_SET_TEXTURE);
	auto &descSetHdrResolveStaging = *(*descSetGroupHdrResolveStaging)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_texture(descSetHdrResolveStaging,*hdrTexStaging,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
	prosper::util::set_descriptor_set_binding_texture(descSetHdrResolveStaging,*bloomBlurTexture,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Bloom));
	//prosper::util::set_descriptor_set_binding_texture(descSetHdrResolveStaging,*glowTex,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));

	descSetGroupHdr = prosper::util::create_descriptor_set_group(dev,pragma::ShaderPPFog::DESCRIPTOR_SET_TEXTURE);
	auto &descSetHdr = *(*descSetGroupHdr)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_texture(*(*descSetGroupHdr)->get_descriptor_set(0u),*resolvedTex,0u);

	imgCreateInfo.format = pragma::ShaderPPHDR::RENDER_PASS_FORMAT;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
	imgCreateInfo.samples = Anvil::SampleCountFlagBits::_1_BIT;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	auto postHdrImg = prosper::util::create_image(dev,imgCreateInfo);
	auto postHdrTex = prosper::util::create_texture(dev,texCreateInfo,postHdrImg,&hdrImgViewCreateInfo,&hdrSamplerCreateInfo);
	postHdrRenderTarget = prosper::util::create_render_target(dev,{postHdrTex},static_cast<prosper::ShaderGraphics*>(shaderPPHdr.get())->GetRenderPass());
	postHdrRenderTarget->SetDebugName("scene_post_hdr_rt");
	descSetGroupPostHdr = prosper::util::create_descriptor_set_group(dev,pragma::ShaderPPFog::DESCRIPTOR_SET_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(*(*descSetGroupPostHdr)->get_descriptor_set(0u),*postHdrTex,0u);

	/*texture = Vulkan::MSAATexture::Create(Vulkan::Texture::Create(context,width,height,hdrFormat,std::function<void(vk::ImageCreateInfo&,vk::MemoryPropertyFlags&)>([samples](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
	info.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
	info.samples = samples;
	}),std::function<void(vk::ImageViewCreateInfo&)>([](vk::ImageViewCreateInfo&) {
	//info.format(Anvil::Format::R8G8B8A8_UNORM);
	}),initSampler));
	textureSwap = Vulkan::Texture::Create(context,width,height,hdrFormat,std::function<void(vk::ImageCreateInfo&,vk::MemoryPropertyFlags&)>([](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
	info.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT;
	}),initSampler);*/ // prosper TODO

	/*textureDepth = Vulkan::Texture::Create(context,width,height,depthFormat,std::function<void(vk::ImageCreateInfo&,vk::MemoryPropertyFlags&)>([samples](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
	info.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
	info.samples = samples;
	}));*/
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

	descSetGroupDepthPostProcessing = prosper::util::create_descriptor_set_group(dev,pragma::ShaderPPFog::DESCRIPTOR_SET_DEPTH_BUFFER);
	prosper::util::set_descriptor_set_binding_texture(*(*descSetGroupDepthPostProcessing)->get_descriptor_set(0u),*resolvedTexture,0u);

#if 0
	textureBloom = Vulkan::MSAATexture::Create(Vulkan::Texture::Create(context,width,height,hdrFormat,std::function<void(vk::ImageCreateInfo&,vk::MemoryPropertyFlags&)>([samples](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
		info.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
		info.samples = samples;
		}),initSampler));

	renderPass = context.GenerateRenderPass({
		{hdrFormat,samples},
		{hdrFormat,samples,true},
		//{ssaoInfo.texturePositions->GetFormat(),samples}, // Position buffer
		//{ssaoInfo.textureNormals->GetFormat(),samples}, // Normal buffer
		{depthFormat,samples,Anvil::AttachmentStoreOp::DONT_CARE,false}
		});
	renderPassSwap = context.GenerateRenderPass(hdrFormat);
	framebuffer = Vulkan::Framebuffer::Create(context,renderPass,width,height,{texture->GetImageView(),textureBloom->GetImageView()/*,ssaoInfo.texturePositions->GetImageView(),ssaoInfo.textureNormals->GetImageView()*/,prepass.textureDepth->GetImageView()});
	framebufferSwap = Vulkan::Framebuffer::Create(context,renderPassSwap,width,height,textureSwap->GetImageView());
	framebufferSwap2 = Vulkan::Framebuffer::Create(context,renderPassSwap,width,height,texture->GetResolvedImageView());

	auto &bloomBlurTexture = GetTargetBloomTexture();
	blurFramebuffer = Vulkan::Framebuffer::Create(context,renderPassSwap,width,height,bloomBlurTexture->GetImageView());
	blurBuffer.Initialize(renderPassSwap,bloomBlurTexture);
#endif // prosper TODO

	InitializeCommandBuffer();
	return true;
}

bool HDRData::InitializeDescriptorSets()
{
	if(pragma::ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP.IsValid() == false)
		return false;
	auto &dev = c_engine->GetDevice();
	descSetGroupDepth = prosper::util::create_descriptor_set_group(dev,pragma::ShaderParticle2DBase::DESCRIPTOR_SET_DEPTH_MAP);

	auto &depthTex = prepass.textureDepthSampled;
	prosper::util::set_descriptor_set_binding_texture(*(*descSetGroupDepth)->get_descriptor_set(0u),*depthTex,0u);

	/*auto hShaderHdr = c_engine->GetShader("hdr");
	if(hShaderHdr.IsValid())
	{
	auto *shader = static_cast<Shader::HDR*>(hShaderHdr.get());
	shader->InitializeInstance(descSetSwap);
	descSetSwap->Update(textureSwap);

	shader->InitializeInstance(descSetSwap2);
	descSetSwap2->Update(texture->GetResolvedTexture());
	}*/ // prosper TODO
	return true;
}

bool HDRData::BlitMainDepthBufferToSamplableDepthBuffer(prosper::CommandBuffer &cmdBuffer,std::function<void(prosper::CommandBuffer&)> &fTransitionSampleImgToTransferDst)
{
	auto &srcDepthTex = *prepass.textureDepth;
	auto &srcDepthImg = srcDepthTex.GetImage()->GetAnvilImage();

	auto &dstDepthTex = *prepass.textureDepthSampled;
	auto &ptrDstDepthImg = dstDepthTex.GetImage();
	auto &dstDepthImg = ptrDstDepthImg->GetAnvilImage();

	auto r = prosper::util::record_image_barrier(*cmdBuffer,srcDepthImg,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL) &&
		prosper::util::record_image_barrier(*cmdBuffer,dstDepthImg,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL) &&
		prosper::util::record_blit_texture(*cmdBuffer,srcDepthTex,dstDepthImg) &&
		prosper::util::record_image_barrier(*cmdBuffer,srcDepthImg,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL) &&
		prosper::util::record_image_barrier(*cmdBuffer,dstDepthImg,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	if(r == false)
		return r;
	//fTransitionSampleImgToTransferDst = [ptrDstDepthImg](prosper::CommandBuffer &cmdBuffer) {
	//	prosper::util::record_image_barrier(*cmdBuffer,ptrDstDepthImg->GetAnvilImage(),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
	//};
	return r;
}

bool HDRData::BlitStagingRenderTargetToMainRenderTarget(prosper::CommandBuffer &cmdBuffer)
{
	auto &hdrTex = hdrRenderTarget->GetTexture();
	auto b = prosper::util::record_image_barrier(*cmdBuffer,hdrTex->GetImage()->GetAnvilImage(),Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
	if(b == false)
		return false;
	b = prosper::util::record_image_barrier(*cmdBuffer,hdrStagingRenderTarget->GetTexture()->GetImage()->GetAnvilImage(),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
	if(b == false)
		return false;
	b = prosper::util::record_blit_texture(
		*cmdBuffer,*hdrStagingRenderTarget->GetTexture(),hdrRenderTarget->GetTexture()->GetImage()->GetAnvilImage()
	);
	if(b == false)
		return false;
	b = prosper::util::record_image_barrier(*cmdBuffer,hdrTex->GetImage()->GetAnvilImage(),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
	if(b == false)
		return false;
	return prosper::util::record_image_barrier(*cmdBuffer,hdrStagingRenderTarget->GetTexture()->GetImage()->GetAnvilImage(),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
}

void HDRData::SwapIOTextures() {m_curTex = (m_curTex == 1u) ? 0u : 1u;}
/*const Vulkan::Texture &HDRData::GetOutputTexture() const {return (m_curTex == 0u) ? textureSwap : texture->GetResolvedTexture();}
const Vulkan::Texture &HDRData::GetInputTexture() const {return (m_curTex == 0u) ? texture->GetResolvedTexture() : textureSwap;}
const Vulkan::DescriptorSet &HDRData::GetInputDescriptorSet() const {return (m_curTex == 0u) ? descSetSwap2 : descSetSwap;}

void HDRData::BeginRenderPass(Vulkan::CommandBufferObject *drawCmd)
{
texture->Resolve();
auto &fb = (m_curTex == 0u) ? framebufferSwap : framebufferSwap2;
auto &texIn = GetInputTexture();
auto &texOut = GetOutputTexture();

texOut->GetImage()->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
texIn->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
drawCmd->BeginRenderPass(renderPassSwap,fb,texOut->GetWidth(),texOut->GetHeight());
}
void HDRData::EndRenderPass(Vulkan::CommandBufferObject *drawCmd)
{
drawCmd->EndRenderPass();
SwapIOTextures();
}*/ // prosper TODO

/*const Vulkan::Texture &HDRData::GetRenderTexture() const {return texture->GetTexture();}
const Vulkan::Texture &HDRData::GetTargetTexture() const {return texture->GetResolvedTexture();}

const Vulkan::Texture &HDRData::GetTargetBloomTexture() const {return textureBloom->GetResolvedTexture();}
const Vulkan::Texture &HDRData::GetTargetDepthTexture() const {return prepass.textureDepth->GetResolvedTexture();}*/ // prosper TODO

void HDRData::ResetIOTextureIndex() {m_curTex = 0u;}

void HDRData::UpdateExposure(prosper::Texture &srcTexture)
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
	pragma::ShaderScene::SetRenderPassSampleCount(static_cast<Anvil::SampleCountFlagBits>(samples));
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
		if(hdrInfo.hdrRenderTarget != nullptr)
		{
			auto *pTexture = wgui.Create<WIDebugMSAATexture>(r);
			pTexture->SetSize(size,size);
			pTexture->SetX(size *idx++);
			pTexture->SetTexture(*hdrInfo.hdrRenderTarget->GetTexture());
			pTexture->SetShouldResolveImage(true);

			hTexture = pTexture->GetHandle();
		}
		if(hdrInfo.bloomBlurRenderTarget != nullptr)
		{
			auto *pTexture = wgui.Create<WIDebugMSAATexture>(r);
			pTexture->SetSize(size,size);
			pTexture->SetX(size *idx++);
			pTexture->SetTexture(*hdrInfo.bloomBlurRenderTarget->GetTexture());
			pTexture->SetShouldResolveImage(true);

			hBloomTexture = pTexture->GetHandle();
		}
		r->SetSize(size *idx,size);
		return r->GetHandle();
		});
	dbg->AddCallback("PostRenderScene",FunctionCallback<>::Create([]() {
		if(hTexture.IsValid() == true)
			static_cast<WIDebugMSAATexture*>(hTexture.get())->Update();
		if(hBloomTexture.IsValid() == true)
			static_cast<WIDebugMSAATexture*>(hBloomTexture.get())->Update();
		}));
}
