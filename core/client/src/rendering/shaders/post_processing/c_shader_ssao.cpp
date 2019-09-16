#include "stdafx_client.h"
#include "pragma/rendering/shaders/post_processing/c_shader_ssao.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_shader_copy_image.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <random>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

decltype(ShaderSSAO::RENDER_PASS_FORMAT) ShaderSSAO::RENDER_PASS_FORMAT = Anvil::Format::R8_UNORM;
decltype(ShaderSSAO::DESCRIPTOR_SET_PREPASS) ShaderSSAO::DESCRIPTOR_SET_PREPASS = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Normal Buffer
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Depth Buffer
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderSSAO::DESCRIPTOR_SET_NOISE_TEXTURE) ShaderSSAO::DESCRIPTOR_SET_NOISE_TEXTURE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Random Normal
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderSSAO::DESCRIPTOR_SET_SAMPLE_BUFFER) ShaderSSAO::DESCRIPTOR_SET_SAMPLE_BUFFER = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Sample Buffer
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderSSAO::DESCRIPTOR_SET_CAMERA) ShaderSSAO::DESCRIPTOR_SET_CAMERA = {&ShaderScene::DESCRIPTOR_SET_CAMERA};
ShaderSSAO::ShaderSSAO(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderBaseImageProcessing(context,identifier,"screen/fs_ssao")
{
	// Generate random sample kernel
	std::uniform_real_distribution<float> randomFloats(0.f,1.f);
	std::default_random_engine generator;
	std::vector<Vector4> ssaoKernel;
	ssaoKernel.reserve(64);
	for(auto i=decltype(ssaoKernel.capacity()){0};i<ssaoKernel.capacity();++i)
	{
		Vector4 sample(
			randomFloats(generator) *2.f -1.f,
			randomFloats(generator) *2.f -1.f,
			randomFloats(generator),
			0.f
		);
		sample  = glm::normalize(sample);
		sample *= randomFloats(generator);
		auto scale = static_cast<float>(i) /64.f;
		scale = umath::lerp(0.1f,1.f,umath::pow2(scale));
		sample *= scale;
		ssaoKernel.push_back(sample);  
	}
	auto size = ssaoKernel.size() *sizeof(ssaoKernel.front());
	auto &dev = c_engine->GetDevice();
	prosper::util::BufferCreateInfo bufferCreateInfo {};
	bufferCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT;
	bufferCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	bufferCreateInfo.size = size;
	m_kernelBuffer = prosper::util::create_buffer(dev,bufferCreateInfo,ssaoKernel.data());
	m_kernelBuffer->SetDebugName("ssao_kernel_buf");

	// Generate kernel rotations
	const auto width = 4u;
	const auto height = 4u;
	std::vector<std::array<uint16_t,4>> ssaoNoise;
	ssaoNoise.reserve(width *height);
	for(auto i=decltype(ssaoNoise.capacity()){0};i<ssaoNoise.capacity();++i)
	{
		ssaoNoise.push_back({
			umath::float32_to_float16(randomFloats(generator) *2.f -1.f),
			umath::float32_to_float16(randomFloats(generator) *2.f -1.f),
			umath::float32_to_float16(0.f), // Rotation axis
			umath::float32_to_float16(0.f) // Alpha (Unused)
		});
	}
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = Anvil::Format::R16G16B16A16_SFLOAT;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
	imgCreateInfo.tiling = Anvil::ImageTiling::LINEAR;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	auto stagingImage = prosper::util::create_image(dev,imgCreateInfo,reinterpret_cast<uint8_t*>(ssaoNoise.data()));

	imgCreateInfo.tiling = Anvil::ImageTiling::OPTIMAL;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::TRANSFER_DST_OPTIMAL;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
	auto noiseImage = prosper::util::create_image(dev,imgCreateInfo);

	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	prosper::util::record_blit_image(*(*setupCmd),{},*(*stagingImage),*(*noiseImage));
	prosper::util::record_image_barrier(*(*setupCmd),*(*noiseImage),Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
	c_engine->FlushSetupCommandBuffer();

	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU =
		samplerCreateInfo.addressModeV =
		samplerCreateInfo.addressModeW = Anvil::SamplerAddressMode::REPEAT;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	m_noiseTexture = prosper::util::create_texture(dev,{},noiseImage,&imgViewCreateInfo,&samplerCreateInfo);

	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderSSAO::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderSSAO>({{{
		RENDER_PASS_FORMAT,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::AttachmentLoadOp::DONT_CARE,
		Anvil::AttachmentStoreOp::STORE,Anvil::SampleCountFlagBits::_1_BIT,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL
	}}},outRenderPass,pipelineIdx);
}

void ShaderSSAO::OnPipelineInitialized(uint32_t pipelineIdx)
{
	prosper::ShaderBaseImageProcessing::OnPipelineInitialized(pipelineIdx);
	if(pipelineIdx != 0u)
		return;
	auto &dev = c_engine->GetDevice();
	m_descSetGroupKernel = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_SAMPLE_BUFFER);
	m_descSetGroupTexture = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_NOISE_TEXTURE);
	
	prosper::util::set_descriptor_set_binding_uniform_buffer(*(*m_descSetGroupKernel)->get_descriptor_set(0u),*m_kernelBuffer,0u);
	prosper::util::set_descriptor_set_binding_texture(*(*m_descSetGroupTexture)->get_descriptor_set(0u),*m_noiseTexture,0u);
}

void ShaderSSAO::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddDefaultVertexAttributes(pipelineInfo);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_PREPASS);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_NOISE_TEXTURE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SAMPLE_BUFFER);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT);
}

bool ShaderSSAO::Draw(const Scene &scene,Anvil::DescriptorSet &descSetPrepass,const std::array<uint32_t,2> &renderTargetDimensions)
{
	auto *descSetCamera = scene.GetCameraDescriptorSetGraphics();
	return RecordBindDescriptorSets({
		&descSetPrepass,(*m_descSetGroupTexture)->get_descriptor_set(0u),(*m_descSetGroupKernel)->get_descriptor_set(0u),
		descSetCamera
	}) && RecordPushConstants(renderTargetDimensions.size() *sizeof(renderTargetDimensions.front()),renderTargetDimensions.data()) &&
		prosper::ShaderBaseImageProcessing::Draw();
}
