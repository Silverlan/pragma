// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :engine;
import :math;
import :rendering.shaders.cubemap;

using namespace pragma;

decltype(ShaderCubemap::VERTEX_BINDING_VERTEX) ShaderCubemap::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex};
decltype(ShaderCubemap::VERTEX_ATTRIBUTE_POSITION) ShaderCubemap::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX, prosper::Format::R32G32B32_SFloat};

ShaderCubemap::ShaderCubemap(prosper::IPrContext &context, const std::string &identifier, const std::string &vertexShader, const std::string &fragmentShader) : ShaderGraphics {context, identifier, vertexShader, fragmentShader} {}
ShaderCubemap::ShaderCubemap(prosper::IPrContext &context, const std::string &identifier, const std::string &fragmentShader) : ShaderCubemap {context, identifier, "programs/image/cubemap", fragmentShader} {}

void ShaderCubemap::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}
void ShaderCubemap::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderCubemap::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderCubemap>({{prosper::util::RenderPassCreateInfo::AttachmentInfo {prosper::Format::R8G8B8A8_UNorm}}}, outRenderPass, pipelineIdx);
	//R32G32B32A32_SFLOAT
}

std::shared_ptr<prosper::IBuffer> ShaderCubemap::CreateCubeMesh(uint32_t &outNumVerts) const
{
	// Generate cube
	constexpr Vector3 min {-1.f, -1.f, -1.f};
	constexpr Vector3 max {1.f, 1.f, 1.f};
	constexpr std::array<Vector3, 8> uniqueVertices {
	  min,                          // 0
	  Vector3(max.x, min.y, min.z), // 1
	  Vector3(max.x, min.y, max.z), // 2
	  Vector3(max.x, max.y, min.z), // 3
	  max,                          // 4
	  Vector3(min.x, max.y, min.z), // 5
	  Vector3(min.x, min.y, max.z), // 6
	  Vector3(min.x, max.y, max.z)  // 7
	};
	constexpr std::array<Vector3, 36> verts {
	  uniqueVertices[0], uniqueVertices[6], uniqueVertices[7], // 1
	  uniqueVertices[0], uniqueVertices[7], uniqueVertices[5], // 1
	  uniqueVertices[3], uniqueVertices[0], uniqueVertices[5], // 2
	  uniqueVertices[3], uniqueVertices[1], uniqueVertices[0], // 2
	  uniqueVertices[2], uniqueVertices[0], uniqueVertices[1], // 3
	  uniqueVertices[2], uniqueVertices[6], uniqueVertices[0], // 3
	  uniqueVertices[7], uniqueVertices[6], uniqueVertices[2], // 4
	  uniqueVertices[4], uniqueVertices[7], uniqueVertices[2], // 4
	  uniqueVertices[4], uniqueVertices[1], uniqueVertices[3], // 5
	  uniqueVertices[1], uniqueVertices[4], uniqueVertices[2], // 5
	  uniqueVertices[4], uniqueVertices[3], uniqueVertices[5], // 6
	  uniqueVertices[4], uniqueVertices[5], uniqueVertices[7]  // 6
	};
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	bufCreateInfo.size = verts.size() * sizeof(verts.front());
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit;
	outNumVerts = verts.size();
	return get_cengine()->GetRenderContext().CreateBuffer(bufCreateInfo, verts.data());
}
std::shared_ptr<prosper::IImage> ShaderCubemap::CreateCubeMap(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags) const
{
	flags = static_cast<prosper::util::ImageCreateInfo::Flags>(math::to_integral(flags) | math::to_integral(prosper::util::ImageCreateInfo::Flags::Cubemap));
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.format = prosper::Format::R16G16B16A16_SFloat;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.tiling = prosper::ImageTiling::Optimal;
	createInfo.flags = flags;
	createInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	createInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;

	return get_cengine()->GetRenderContext().CreateImage(createInfo);
}
void ShaderCubemap::InitializeSamplerCreateInfo(prosper::util::ImageCreateInfo::Flags flags, prosper::util::SamplerCreateInfo &inOutSamplerCreateInfo)
{
	inOutSamplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	inOutSamplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	inOutSamplerCreateInfo.addressModeW = prosper::SamplerAddressMode::ClampToEdge;
	inOutSamplerCreateInfo.minFilter = prosper::Filter::Linear;
	inOutSamplerCreateInfo.magFilter = prosper::Filter::Linear;
	if(math::is_flag_set(flags, prosper::util::ImageCreateInfo::Flags::FullMipmapChain))
		inOutSamplerCreateInfo.mipmapMode = prosper::SamplerMipmapMode::Linear;
}
void ShaderCubemap::InitializeTextureCreateInfo(prosper::util::TextureCreateInfo &inOutTextureCreateInfo) { inOutTextureCreateInfo.flags |= prosper::util::TextureCreateInfo::Flags::CreateImageViewForEachLayer; }
std::shared_ptr<prosper::RenderTarget> ShaderCubemap::CreateCubeMapRenderTarget(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags) const
{
	auto img = CreateCubeMap(width, height, flags);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	imgViewCreateInfo.mipmapLevels = 1;
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	InitializeSamplerCreateInfo(flags, samplerCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	InitializeTextureCreateInfo(texCreateInfo);
	auto tex = get_cengine()->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);

	prosper::util::RenderTargetCreateInfo rtCreateInfo {};
	rtCreateInfo.useLayerFramebuffers = true;
	return get_cengine()->GetRenderContext().CreateRenderTarget({tex}, GetRenderPass(), rtCreateInfo);
}

const Mat4 &ShaderCubemap::GetProjectionMatrix(float aspectRatio) const { return math::get_cubemap_projection_matrix(aspectRatio); }
const Mat4 &ShaderCubemap::GetViewMatrix(uint8_t layerId) const { return math::get_cubemap_view_matrices().at(layerId); }
