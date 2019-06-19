#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/raytracing/c_shader_raytracing.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <misc/compute_pipeline_create_info.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
decltype(ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT) ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Image
			Anvil::DescriptorType::STORAGE_IMAGE,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		}
	}
};
decltype(ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE) ShaderRayTracing::DESCRIPTOR_SET_GAME_SCENE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Textures
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT,
			umath::to_integral(GameLimits::MaxImageArrayLayers)
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Materials
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // SubMeshInfos
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // EntityInstanceData
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // BoneMatrices
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // VertexBuffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // IndexBuffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // VertexWeightBuffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // AlphaBuffer
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::COMPUTE_BIT
		},
	}
};

decltype(ShaderRayTracing::DESCRIPTOR_SET_CAMERA) ShaderRayTracing::DESCRIPTOR_SET_CAMERA = {&ShaderForwardPLightCulling::DESCRIPTOR_SET_CAMERA};
decltype(ShaderRayTracing::DESCRIPTOR_SET_LIGHTS) ShaderRayTracing::DESCRIPTOR_SET_LIGHTS = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Light Buffers
			Anvil::DescriptorType::STORAGE_BUFFER,
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

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),Anvil::ShaderStageFlagBits::COMPUTE_BIT);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_IMAGE_OUTPUT);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_GAME_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_LIGHTS);
}

bool ShaderRayTracing::Compute(
	const PushConstants &pushConstants,
	Anvil::DescriptorSet &descSetOutputImage,Anvil::DescriptorSet &descSetGameScene,
	Anvil::DescriptorSet &descSetCamera,Anvil::DescriptorSet &descSetLightSources,
	uint32_t workGroupsX,uint32_t workGroupsY
)
{
	return RecordBindDescriptorSets({
		&descSetOutputImage,
		&descSetGameScene,
		&descSetCamera,
		&descSetLightSources
	}) && RecordPushConstants(pushConstants) && RecordDispatch(workGroupsX,workGroupsY);
}

////////////////////////

#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>

#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"

namespace pragma::rendering
{
	class BaseRenderer
	{
	public:
		template<class TRenderer>
			static std::unique_ptr<TRenderer> Create(uint32_t width,uint32_t height);
		virtual void Render(Scene &scene)=0;
		void Resize(uint32_t width,uint32_t height);
	protected:
		virtual bool Initialize(uint32_t width,uint32_t height)=0;
		BaseRenderer()=default;
	};

	class RaytracingRenderer
		: public BaseRenderer
	{
	public:

		const std::shared_ptr<prosper::Texture> &GetOutputTexture() const;
		Anvil::DescriptorSet &GetOutputImageDescriptorSet();

		virtual void Render(Scene &scene) override;
	private:
		friend BaseRenderer;
		RaytracingRenderer()=default;
		virtual bool Initialize(uint32_t width,uint32_t height) override;
		std::shared_ptr<prosper::Texture> m_outputTexture = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_dsgOutputImage = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_dsgLights = nullptr;
		util::WeakHandle<prosper::Shader> m_whShader = {};
	};
};

template<class TRenderer>
	std::unique_ptr<TRenderer> pragma::rendering::BaseRenderer::Create(uint32_t width,uint32_t height)
{
	auto res = std::unique_ptr<TRenderer>{new TRenderer{}};
	if(res->Initialize(width,height) == false)
		return nullptr;
	return res;
}

#include <wgui/wgui.h>
#include <wgui/types/wirect.h>
#include <prosper_command_buffer.hpp>
void pragma::rendering::RaytracingRenderer::Render(Scene &scene)
{
	if(m_whShader.expired())
		return;
	auto &imgOutput = GetOutputTexture()->GetImage();
	auto &drawCmd = c_engine->GetDrawCommandBuffer();
	prosper::util::record_image_barrier(
		**drawCmd,**imgOutput,
		Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
		Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::GENERAL,
		Anvil::AccessFlagBits::SHADER_READ_BIT,Anvil::AccessFlagBits::SHADER_WRITE_BIT
	);

	auto &cam = *scene.GetCamera();
	auto extents = imgOutput->GetExtents();
	ShaderRayTracing::PushConstants pushConstants {
		pragma::CRaytracingComponent::GetBufferMeshCount(),pragma::CLightComponent::GetLightCount(),
		extents.width,extents.height,cam.GetFOVRad()
	};

	auto &dsgCam = scene.GetCameraDescriptorSetGroup(vk::PipelineBindPoint::eCompute);
	auto dsgGameScene = CRaytracingComponent::GetGameSceneDescriptorSetGroup();

	auto swapChainWidth = 256;
	auto swapChainHeight = 256;
	auto &shader = static_cast<ShaderRayTracing&>(*m_whShader);
	shader.Compute(
		pushConstants,GetOutputImageDescriptorSet(),
		*(*dsgGameScene)->get_descriptor_set(0),*(*dsgCam)->get_descriptor_set(0),
		*(*m_dsgLights)->get_descriptor_set(0),
		swapChainWidth,swapChainHeight
	);

	prosper::util::record_image_barrier(
		**drawCmd,**imgOutput,
		Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
		Anvil::ImageLayout::GENERAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
		Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);
}

bool pragma::rendering::RaytracingRenderer::Initialize(uint32_t width,uint32_t height)
{
	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = Anvil::Format::R8G8B8A8_UNORM;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.tiling = Anvil::ImageTiling::OPTIMAL;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::STORAGE_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;

	auto img = prosper::util::create_image(dev,imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	m_outputTexture = prosper::util::create_texture(dev,prosper::util::TextureCreateInfo{},img,&imgViewCreateInfo,&samplerCreateInfo);
	auto descSetImage = prosper::util::create_descriptor_set_group(dev,ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT);
	prosper::util::set_descriptor_set_binding_storage_image(*(*descSetImage)->get_descriptor_set(0),*m_outputTexture,0u);
	m_dsgOutputImage = descSetImage;

	m_dsgLights = prosper::util::create_descriptor_set_group(c_engine->GetDevice(),pragma::ShaderRayTracing::DESCRIPTOR_SET_LIGHTS);
	prosper::util::set_descriptor_set_binding_storage_buffer(
		*(*m_dsgLights)->get_descriptor_set(0u),const_cast<prosper::UniformResizableBuffer&>(pragma::CLightComponent::GetGlobalRenderBuffer()),0
	);

	m_whShader = c_engine->GetShader("raytracing");
	return m_whShader.valid();
}
const std::shared_ptr<prosper::Texture> &pragma::rendering::RaytracingRenderer::GetOutputTexture() const {return m_outputTexture;}
Anvil::DescriptorSet &pragma::rendering::RaytracingRenderer::GetOutputImageDescriptorSet() {return *m_dsgOutputImage->GetAnvilDescriptorSetGroup().get_descriptor_set(0);}

bool ShaderRayTracing::ComputeTest()
{
	if(CRaytracingComponent::IsRaytracingEnabled() == false)
		return false;
	static auto rtxScene = pragma::rendering::RaytracingRenderer::Create<pragma::rendering::RaytracingRenderer>(1280,1024);
	static auto initialized = false;
	if(initialized == false)
	{
		initialized = true;
		auto &wgui = WGUI::GetInstance();
		auto *p = wgui.Create<WITexturedRect>();
		p->SetTexture(*rtxScene->GetOutputTexture());
		p->SetSize(1280,1024);
	}
	rtxScene->Render(*static_cast<CGame*>(c_engine->GetClientState()->GetGameState())->GetRenderScene());
	return true;
}
#pragma optimize("",on)
