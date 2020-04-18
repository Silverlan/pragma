#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_base_cubemap.hpp"
#include "pragma/math/c_util_math.hpp"
#include <image/prosper_sampler.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderCubemap::VERTEX_BINDING_VERTEX) ShaderCubemap::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX};
decltype(ShaderCubemap::VERTEX_ATTRIBUTE_POSITION) ShaderCubemap::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32_SFLOAT};


ShaderCubemap::ShaderCubemap(prosper::Context &context,const std::string &identifier,const std::string &vertexShader,const std::string &fragmentShader)
	: ShaderGraphics{context,identifier,vertexShader,fragmentShader}
{}
ShaderCubemap::ShaderCubemap(prosper::Context &context,const std::string &identifier,const std::string &fragmentShader)
	: ShaderCubemap{context,identifier,"screen/vs_cubemap",fragmentShader}
{}

void ShaderCubemap::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT);
}

void ShaderCubemap::InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderCubemap>({{prosper::util::RenderPassCreateInfo::AttachmentInfo{Anvil::Format::R8G8B8A8_UNORM}}},outRenderPass,pipelineIdx);
	//R32G32B32A32_SFLOAT
}

std::shared_ptr<prosper::Buffer> ShaderCubemap::CreateCubeMesh(uint32_t &outNumVerts) const
{
	// Generate cube
	constexpr Vector3 min {-1.f,-1.f,-1.f};
	constexpr Vector3 max {1.f,1.f,1.f};
	constexpr std::array<Vector3,8> uniqueVertices {
		min, // 0
		Vector3(max.x,min.y,min.z), // 1
		Vector3(max.x,min.y,max.z), // 2
		Vector3(max.x,max.y,min.z), // 3
		max, // 4
		Vector3(min.x,max.y,min.z), // 5
		Vector3(min.x,min.y,max.z), // 6
		Vector3(min.x,max.y,max.z) // 7
	};
	constexpr std::array<Vector3,36> verts {
		uniqueVertices[0],uniqueVertices[6],uniqueVertices[7], // 1
		uniqueVertices[0],uniqueVertices[7],uniqueVertices[5], // 1
		uniqueVertices[3],uniqueVertices[0],uniqueVertices[5], // 2
		uniqueVertices[3],uniqueVertices[1],uniqueVertices[0], // 2
		uniqueVertices[2],uniqueVertices[0],uniqueVertices[1], // 3
		uniqueVertices[2],uniqueVertices[6],uniqueVertices[0], // 3
		uniqueVertices[7],uniqueVertices[6],uniqueVertices[2], // 4
		uniqueVertices[4],uniqueVertices[7],uniqueVertices[2], // 4
		uniqueVertices[4],uniqueVertices[1],uniqueVertices[3], // 5
		uniqueVertices[1],uniqueVertices[4],uniqueVertices[2], // 5
		uniqueVertices[4],uniqueVertices[3],uniqueVertices[5], // 6
		uniqueVertices[4],uniqueVertices[5],uniqueVertices[7] // 6
	};
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	bufCreateInfo.size = verts.size() *sizeof(verts.front());
	bufCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::VERTEX_BUFFER_BIT;
	outNumVerts = verts.size();
	return prosper::util::create_buffer(c_engine->GetDevice(),bufCreateInfo,verts.data());
}
std::shared_ptr<prosper::Image> ShaderCubemap::CreateCubeMap(uint32_t width,uint32_t height,prosper::util::ImageCreateInfo::Flags flags) const
{
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.format = Anvil::Format::R16G16B16A16_SFLOAT;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.tiling = Anvil::ImageTiling::OPTIMAL;
	createInfo.flags |= flags | prosper::util::ImageCreateInfo::Flags::Cubemap;
	createInfo.usage = Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	createInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;

	auto &dev = c_engine->GetDevice();
	return prosper::util::create_image(dev,createInfo);
}
void ShaderCubemap::InitializeSamplerCreateInfo(prosper::util::ImageCreateInfo::Flags flags,prosper::util::SamplerCreateInfo &inOutSamplerCreateInfo)
{
	inOutSamplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	inOutSamplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	inOutSamplerCreateInfo.addressModeW = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	inOutSamplerCreateInfo.minFilter = Anvil::Filter::LINEAR;
	inOutSamplerCreateInfo.magFilter = Anvil::Filter::LINEAR;
	if(umath::is_flag_set(flags,prosper::util::ImageCreateInfo::Flags::FullMipmapChain))
		inOutSamplerCreateInfo.mipmapMode = Anvil::SamplerMipmapMode::LINEAR;
}
void ShaderCubemap::InitializeTextureCreateInfo(prosper::util::TextureCreateInfo &inOutTextureCreateInfo)
{
	inOutTextureCreateInfo.flags |= prosper::util::TextureCreateInfo::Flags::CreateImageViewForEachLayer;
}
std::shared_ptr<prosper::RenderTarget> ShaderCubemap::CreateCubeMapRenderTarget(uint32_t width,uint32_t height,prosper::util::ImageCreateInfo::Flags flags) const
{
	auto &dev = c_engine->GetDevice();
	auto img = CreateCubeMap(width,height,flags);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	InitializeSamplerCreateInfo(flags,samplerCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	InitializeTextureCreateInfo(texCreateInfo);
	auto tex = prosper::util::create_texture(dev,texCreateInfo,img,&imgViewCreateInfo,&samplerCreateInfo);

	prosper::util::RenderTargetCreateInfo rtCreateInfo {};
	rtCreateInfo.useLayerFramebuffers = true;
	return prosper::util::create_render_target(dev,{tex},GetRenderPass(),rtCreateInfo);
}

const Mat4 &ShaderCubemap::GetProjectionMatrix(float aspectRatio) const
{
	return pragma::math::get_cubemap_projection_matrix(aspectRatio);
}
const Mat4 &ShaderCubemap::GetViewMatrix(uint8_t layerId) const
{
	return pragma::math::get_cubemap_view_matrices().at(layerId);
}

