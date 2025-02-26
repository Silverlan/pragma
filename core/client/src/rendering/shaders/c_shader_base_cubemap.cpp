/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/shaders/c_shader_base_cubemap.hpp"
#include "pragma/math/c_util_math.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <shader/prosper_shader_t.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;

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
	return c_engine->GetRenderContext().CreateBuffer(bufCreateInfo, verts.data());
}
std::shared_ptr<prosper::IImage> ShaderCubemap::CreateCubeMap(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags) const
{
	flags = static_cast<prosper::util::ImageCreateInfo::Flags>(umath::to_integral(flags) | umath::to_integral(prosper::util::ImageCreateInfo::Flags::Cubemap));
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.format = prosper::Format::R16G16B16A16_SFloat;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.tiling = prosper::ImageTiling::Optimal;
	createInfo.flags = flags;
	createInfo.usage = prosper::ImageUsageFlags::ColorAttachmentBit | prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::TransferDstBit;
	createInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;

	return c_engine->GetRenderContext().CreateImage(createInfo);
}
void ShaderCubemap::InitializeSamplerCreateInfo(prosper::util::ImageCreateInfo::Flags flags, prosper::util::SamplerCreateInfo &inOutSamplerCreateInfo)
{
	inOutSamplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
	inOutSamplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
	inOutSamplerCreateInfo.addressModeW = prosper::SamplerAddressMode::ClampToEdge;
	inOutSamplerCreateInfo.minFilter = prosper::Filter::Linear;
	inOutSamplerCreateInfo.magFilter = prosper::Filter::Linear;
	if(umath::is_flag_set(flags, prosper::util::ImageCreateInfo::Flags::FullMipmapChain))
		inOutSamplerCreateInfo.mipmapMode = prosper::SamplerMipmapMode::Linear;
}
void ShaderCubemap::InitializeTextureCreateInfo(prosper::util::TextureCreateInfo &inOutTextureCreateInfo) { inOutTextureCreateInfo.flags |= prosper::util::TextureCreateInfo::Flags::CreateImageViewForEachLayer; }
std::shared_ptr<prosper::RenderTarget> ShaderCubemap::CreateCubeMapRenderTarget(uint32_t width, uint32_t height, prosper::util::ImageCreateInfo::Flags flags) const
{
	auto img = CreateCubeMap(width, height, flags);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	InitializeSamplerCreateInfo(flags, samplerCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	InitializeTextureCreateInfo(texCreateInfo);
	auto tex = c_engine->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);

	prosper::util::RenderTargetCreateInfo rtCreateInfo {};
	rtCreateInfo.useLayerFramebuffers = true;
	return c_engine->GetRenderContext().CreateRenderTarget({tex}, GetRenderPass(), rtCreateInfo);
}

const Mat4 &ShaderCubemap::GetProjectionMatrix(float aspectRatio) const { return pragma::math::get_cubemap_projection_matrix(aspectRatio); }
const Mat4 &ShaderCubemap::GetViewMatrix(uint8_t layerId) const { return pragma::math::get_cubemap_view_matrices().at(layerId); }
