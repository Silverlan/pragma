#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/raytracing/c_shader_raytracing.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
decltype(ShaderRayTracing::DESCRIPTOR_SET_IMAGE) ShaderRayTracing::DESCRIPTOR_SET_IMAGE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Image
			Anvil::DescriptorType::STORAGE_IMAGE,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
decltype(ShaderRayTracing::DESCRIPTOR_SET_BUFFERS) ShaderRayTracing::DESCRIPTOR_SET_BUFFERS = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Spheres
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Planes
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Uniform App
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
ShaderRayTracing::ShaderRayTracing(prosper::Context &context,const std::string &identifier)
	: prosper::ShaderCompute(context,identifier,"world/raytracing/raytracing.gls")
{}

void ShaderRayTracing::InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderCompute::InitializeComputePipeline(pipelineInfo,pipelineIdx);

	// Currently not supported on some GPUs?
	// AddSpecializationConstant(pipelineInfo,0u /* constant id */,sizeof(TILE_SIZE),&TILE_SIZE);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_IMAGE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_BUFFERS);
}

bool ShaderRayTracing::Compute(Anvil::DescriptorSet &descSetImage,Anvil::DescriptorSet &descSetBuffers,uint32_t workGroupsX,uint32_t workGroupsY)
{
	return RecordBindDescriptorSet(descSetImage,DESCRIPTOR_SET_IMAGE.setIndex) &&
		RecordBindDescriptorSet(descSetBuffers,DESCRIPTOR_SET_BUFFERS.setIndex) &&
		RecordDispatch(workGroupsX,workGroupsY);
}

////////////////////////

#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>

namespace rtx
{
struct Material
{
	Vector3 color;
	int type;

	Material();
	Material(Vector3, int);
};
Material::Material() { color = Vector3(0, 0, 0);  type = 1; }
Material::Material(Vector3 color, int type) : color(color), type(type) {}

struct Planee
{
	Vector3 normal;
	float distance;

	Material mat;

	Planee();
	Planee(Vector3 normal, float distance);
};
Planee::Planee() { distance = 1; }
Planee::Planee(Vector3 normal, float distance) : normal(normal), distance(distance) {}

struct Sphere
{
	Vector3 position;
	float radius;

	Material mat;

	Sphere();
	Sphere(Vector3 position, float radius);
};

Sphere::Sphere() { radius = 1; }
Sphere::Sphere(Vector3 position, float radius) : position(position), radius(radius) {}
};

struct RTXTest
{
	std::shared_ptr<prosper::Texture> texture;
	std::shared_ptr<prosper::Buffer> sphereBuffer;
	std::shared_ptr<prosper::Buffer> planeBuffer;
	std::shared_ptr<prosper::Buffer> uniformBuffer;

	std::shared_ptr<prosper::DescriptorSetGroup> dsgImage;
	std::shared_ptr<prosper::DescriptorSetGroup> dsgBuffers;
};

static void InitGameObjects(std::vector<rtx::Planee> &planes, std::vector<rtx::Sphere> &spheres)
{
	auto AddPlanee = [&planes](rtx::Planee* go, Vector3 color, int type)
	{
		go->mat = rtx::Material(color, type);
		planes.push_back(*go);
	};

	auto AddSphere = [&spheres](rtx::Sphere* go, Vector3 color, int type)
	{
		go->mat = rtx::Material(color, type);
		spheres.push_back(*go);
	};

	auto sphere = new rtx::Sphere(Vector3(-0.55, -1.55, -4.0), 1.0);
	auto sphere2 = new rtx::Sphere(Vector3(1.3, 1.2, -4.2), 0.8);

	auto bottom = new rtx::Planee(Vector3(0, 1, 0), 2.5);
	auto back = new rtx::Planee(Vector3(0, 0, 1), 5.5);
	auto left = new rtx::Planee(Vector3(1, 0, 0), 2.75);
	auto right = new rtx::Planee(Vector3(-1, 0, 0), 2.75);
	auto ceiling = new rtx::Planee(Vector3(0, -1, 0), 3.0);
	auto front = new rtx::Planee(Vector3(0, 0, -1), 0.5);


	AddSphere(sphere, Vector3(0.3, 0.9, 0.76), 2);
	AddSphere(sphere2, Vector3(0.062, 0.917, 0.078), 1);

	AddPlanee(bottom, Vector3(0.8, 0.8, 0.8), 1);
	AddPlanee(back, Vector3(0.8, 0.8, 0.8), 1);
	AddPlanee(left, Vector3(1, 0.250, 0.019), 1);
	AddPlanee(right, Vector3(0.007, 0.580, 0.8), 1);
	AddPlanee(ceiling, Vector3(0.8, 0.8, 0.8), 1);
	AddPlanee(front, Vector3(0.8, 0.8, 0.8), 1);
}

RTXTest rtxTest {};
void ShaderRayTracing::Test()
{
	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = Anvil::Format::R8G8B8A8_UNORM;
	imgCreateInfo.height = 1024;
	imgCreateInfo.width = 1024;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.tiling = Anvil::ImageTiling::OPTIMAL;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::STORAGE_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
	imgCreateInfo.usage |= Anvil::ImageUsageFlagBits::SAMPLED_BIT;

	auto img = prosper::util::create_image(dev,imgCreateInfo);
	
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto tex = prosper::util::create_texture(dev,prosper::util::TextureCreateInfo{},img,&imgViewCreateInfo,&samplerCreateInfo);
	rtxTest.texture = tex;

	auto descSetImage = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_IMAGE);
	prosper::util::set_descriptor_set_binding_storage_image(*(*descSetImage)->get_descriptor_set(0),*tex,0u);
	rtxTest.dsgImage = descSetImage;

	std::vector<rtx::Planee> planes;
	std::vector<rtx::Sphere> spheres;
	InitGameObjects(planes,spheres);

	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
	bufCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT;
	bufCreateInfo.size = spheres.size() *sizeof(spheres.front());
	auto sphereBuffer = prosper::util::create_buffer(dev,bufCreateInfo,spheres.data());
	rtxTest.sphereBuffer = sphereBuffer;

	bufCreateInfo.size = planes.size() *sizeof(planes.front());
	auto planeBuffer = prosper::util::create_buffer(dev,bufCreateInfo,planes.data());
	rtxTest.planeBuffer = planeBuffer;

	struct App
	{
		float time;
	} app;


	bufCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT;
	bufCreateInfo.size = sizeof(app);
	auto uniformBuffer = prosper::util::create_buffer(dev,bufCreateInfo,&app);
	rtxTest.uniformBuffer = uniformBuffer;

	auto descSetBuffers = prosper::util::create_descriptor_set_group(dev,DESCRIPTOR_SET_BUFFERS);
	prosper::util::set_descriptor_set_binding_storage_buffer(*(*descSetBuffers)->get_descriptor_set(0),*sphereBuffer,0);
	prosper::util::set_descriptor_set_binding_storage_buffer(*(*descSetBuffers)->get_descriptor_set(0),*planeBuffer,1);
	prosper::util::set_descriptor_set_binding_uniform_buffer(*(*descSetBuffers)->get_descriptor_set(0),*uniformBuffer,2);
	rtxTest.dsgBuffers = descSetBuffers;
}

#include <wgui/wgui.h>
#include <wgui/types/wirect.h>
#include <prosper_command_buffer.hpp>
bool ShaderRayTracing::ComputeTest()
{
	if(rtxTest.dsgBuffers == nullptr)
	{
		Test();

		auto &wgui = WGUI::GetInstance();
		auto *p = wgui.Create<WITexturedRect>();
		p->SetTexture(*rtxTest.texture);
		p->SetSize(1024,1024);
	}

	prosper::util::record_image_barrier(
		**c_engine->GetDrawCommandBuffer(),**rtxTest.texture->GetImage(),
		Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
		Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::GENERAL,
		Anvil::AccessFlagBits::SHADER_READ_BIT,Anvil::AccessFlagBits::SHADER_WRITE_BIT
	);

	auto swapChainWidth = 1024;
	auto swapChainHeight = 1024;
	auto result = RecordBindDescriptorSet(*rtxTest.dsgImage->GetAnvilDescriptorSetGroup().get_descriptor_set(0),DESCRIPTOR_SET_IMAGE.setIndex) &&
		RecordBindDescriptorSet(*rtxTest.dsgBuffers->GetAnvilDescriptorSetGroup().get_descriptor_set(0),DESCRIPTOR_SET_BUFFERS.setIndex) &&
		RecordDispatch(swapChainWidth, swapChainHeight);

	prosper::util::record_image_barrier(
		**c_engine->GetDrawCommandBuffer(),**rtxTest.texture->GetImage(),
		Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
		Anvil::ImageLayout::GENERAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
		Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);
	return result;
}
#pragma optimize("",on)
