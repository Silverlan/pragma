// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.ssao;

import :engine;

using namespace pragma;

decltype(ShaderSSAO::RENDER_PASS_FORMAT) ShaderSSAO::RENDER_PASS_FORMAT = prosper::Format::R8_UNorm;
decltype(ShaderSSAO::DESCRIPTOR_SET_PREPASS) ShaderSSAO::DESCRIPTOR_SET_PREPASS = {
  "PREPASS",
  {prosper::DescriptorSetInfo::Binding {"NORMAL_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}, prosper::DescriptorSetInfo::Binding {"DEPTH_BUFFER", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderSSAO::DESCRIPTOR_SET_NOISE_TEXTURE) ShaderSSAO::DESCRIPTOR_SET_NOISE_TEXTURE = {
  "NOISE",
  {prosper::DescriptorSetInfo::Binding {"RANDOM_NORMAL", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderSSAO::DESCRIPTOR_SET_SAMPLE_BUFFER) ShaderSSAO::DESCRIPTOR_SET_SAMPLE_BUFFER = {
  "SAMPLE",
  {prosper::DescriptorSetInfo::Binding {"BUFFER", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit}},
};
decltype(ShaderSSAO::DESCRIPTOR_SET_SCENE) ShaderSSAO::DESCRIPTOR_SET_SCENE = {&ShaderScene::DESCRIPTOR_SET_SCENE};
ShaderSSAO::ShaderSSAO(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing(context, identifier, "programs/post_processing/ssao")
{
	// Generate random sample kernel
	std::uniform_real_distribution<float> randomFloats(0.f, 1.f);
	std::default_random_engine generator;
	std::vector<Vector4> ssaoKernel;
	ssaoKernel.reserve(64);
	for(auto i = decltype(ssaoKernel.capacity()) {0}; i < ssaoKernel.capacity(); ++i) {
		Vector4 sample(randomFloats(generator) * 2.f - 1.f, randomFloats(generator) * 2.f - 1.f, randomFloats(generator), 0.f);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		auto scale = static_cast<float>(i) / 64.f;
		scale = math::lerp(0.1f, 1.f, math::pow2(scale));
		sample *= scale;
		ssaoKernel.push_back(sample);
	}
	auto size = ssaoKernel.size() * sizeof(ssaoKernel.front());
	prosper::util::BufferCreateInfo bufferCreateInfo {};
	bufferCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	bufferCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	bufferCreateInfo.size = size;
	m_kernelBuffer = get_cengine()->GetRenderContext().CreateBuffer(bufferCreateInfo, ssaoKernel.data());
	m_kernelBuffer->SetDebugName("ssao_kernel_buf");

	// Generate kernel rotations
	const auto width = 4u;
	const auto height = 4u;
	std::vector<std::array<uint16_t, 4>> ssaoNoise;
	ssaoNoise.reserve(width * height);
	for(auto i = decltype(ssaoNoise.capacity()) {0}; i < ssaoNoise.capacity(); ++i) {
		ssaoNoise.push_back({
		  math::float32_to_float16(randomFloats(generator) * 2.f - 1.f), math::float32_to_float16(randomFloats(generator) * 2.f - 1.f),
		  math::float32_to_float16(0.f), // Rotation axis
		  math::float32_to_float16(0.f)  // Alpha (Unused)
		});
	}
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R16G16B16A16_SFloat;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.usage = prosper::ImageUsageFlags::TransferSrcBit;
	imgCreateInfo.tiling = prosper::ImageTiling::Linear;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::TransferSrcOptimal;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;

	math::set_flag(imgCreateInfo.flags, prosper::util::ImageCreateInfo::Flags::DontAllocateMemory);
	auto stagingImage = context.CreateImage(imgCreateInfo, reinterpret_cast<uint8_t *>(ssaoNoise.data()));
	context.AllocateTemporaryBuffer(*stagingImage);
	math::set_flag(imgCreateInfo.flags, prosper::util::ImageCreateInfo::Flags::DontAllocateMemory, false);

	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferDstBit;
	auto noiseImage = context.CreateImage(imgCreateInfo);

	auto &setupCmd = get_cengine()->GetSetupCommandBuffer();
	setupCmd->RecordBlitImage({}, *stagingImage, *noiseImage);
	setupCmd->RecordImageBarrier(*noiseImage, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
	get_cengine()->FlushSetupCommandBuffer();

	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeW = prosper::SamplerAddressMode::Repeat;
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	m_noiseTexture = context.CreateTexture({}, *noiseImage, imgViewCreateInfo, samplerCreateInfo);

	SetBaseShader<prosper::ShaderCopyImage>();
}

void ShaderSSAO::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderSSAO>({{{RENDER_PASS_FORMAT, prosper::ImageLayout::ColorAttachmentOptimal, prosper::AttachmentLoadOp::DontCare, prosper::AttachmentStoreOp::Store, prosper::SampleCountFlags::e1Bit, prosper::ImageLayout::ShaderReadOnlyOptimal}}}, outRenderPass, pipelineIdx);
}

void ShaderSSAO::OnPipelineInitialized(uint32_t pipelineIdx)
{
	ShaderBaseImageProcessing::OnPipelineInitialized(pipelineIdx);
	if(pipelineIdx != 0u)
		return;
	m_descSetGroupKernel = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_SAMPLE_BUFFER);
	m_descSetGroupTexture = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_NOISE_TEXTURE);

	m_descSetGroupKernel->GetDescriptorSet()->SetBindingUniformBuffer(*m_kernelBuffer, 0u);
	m_descSetGroupTexture->GetDescriptorSet()->SetBindingTexture(*m_noiseTexture, 0u);
}

void ShaderSSAO::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddDefaultVertexAttributes();

	AddDescriptorSetGroup(DESCRIPTOR_SET_PREPASS);
	AddDescriptorSetGroup(DESCRIPTOR_SET_NOISE_TEXTURE);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SAMPLE_BUFFER);
	AddDescriptorSetGroup(DESCRIPTOR_SET_SCENE);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

void ShaderSSAO::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

bool ShaderSSAO::RecordDraw(prosper::ShaderBindState &bindState, const CSceneComponent &scene, prosper::IDescriptorSet &descSetPrepass, const std::array<uint32_t, 2> &renderTargetDimensions) const
{
	auto *descSetCamera = scene.GetCameraDescriptorSetGraphics();
	return RecordBindDescriptorSets(bindState, {&descSetPrepass, m_descSetGroupTexture->GetDescriptorSet(), m_descSetGroupKernel->GetDescriptorSet(), descSetCamera})
	  && RecordPushConstants(bindState, renderTargetDimensions.size() * sizeof(renderTargetDimensions.front()), renderTargetDimensions.data()) && ShaderBaseImageProcessing::RecordDraw(bindState);
}
