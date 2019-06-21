#include "stdafx_client.h"
#include "pragma/rendering/renderers/raytracing_renderer.hpp"
#include "pragma/rendering/shaders/world/raytracing/c_shader_raytracing.hpp"
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>

using namespace pragma::rendering;

extern DLLCENGINE CEngine *c_engine;
bool RaytracingRenderer::Initialize()
{
	auto &dev = c_engine->GetDevice();
	auto &scene = GetScene();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = Anvil::Format::R8G8B8A8_UNORM;
	imgCreateInfo.width = scene.GetWidth();
	imgCreateInfo.height = scene.GetHeight();
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
	return m_whShader.valid() && CRaytracingComponent::InitializeBuffers();
}
bool RaytracingRenderer::IsRayTracingRenderer() const {return true;}
void RaytracingRenderer::OnEntityAddedToScene(CBaseEntity &ent) {ent.AddComponent<CRaytracingComponent>();}
#include <wgui/types/wirect.h>
bool RaytracingRenderer::RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt,FRender renderFlags)
{
	if(m_whShader.expired() || BaseRenderer::RenderScene(drawCmd,rt,renderFlags) == false)
		return false;
	auto &imgOutput = GetSceneTexture()->GetImage();
	prosper::util::record_image_barrier(
		**drawCmd,**imgOutput,
		Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
		Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::GENERAL,
		Anvil::AccessFlagBits::SHADER_READ_BIT,Anvil::AccessFlagBits::SHADER_WRITE_BIT
	);

	auto &scene = GetScene();
	auto &cam = scene.GetActiveCamera();
	auto extents = imgOutput->GetExtents();
	ShaderRayTracing::PushConstants pushConstants {
		pragma::CRaytracingComponent::GetBufferMeshCount(),pragma::CLightComponent::GetLightCount(),
		extents.width,extents.height,cam.valid() ? cam->GetFOVRad() : umath::deg_to_rad(pragma::CCameraComponent::DEFAULT_FOV)
	};

	auto &dsgCam = scene.GetCameraDescriptorSetGroup(vk::PipelineBindPoint::eCompute);
	auto dsgGameScene = CRaytracingComponent::GetGameSceneDescriptorSetGroup();
	if(dsgGameScene == nullptr)
		return false;

	auto swapChainWidth = 256;
	auto swapChainHeight = 256;
	auto &shader = static_cast<ShaderRayTracing&>(*m_whShader);
	if(shader.BeginCompute(drawCmd) == false)
		return false;
	shader.Compute(
		pushConstants,GetOutputImageDescriptorSet(),
		*(*dsgGameScene)->get_descriptor_set(0),*(*dsgCam)->get_descriptor_set(0),
		*(*m_dsgLights)->get_descriptor_set(0),
		swapChainWidth,swapChainHeight
	);

	/*static auto initialized = false;
	if(initialized == false)
	{
		initialized = true;
		auto &wgui = WGUI::GetInstance();
		auto *p = wgui.Create<WITexturedRect>();
		p->SetTexture(*GetSceneTexture());
		p->SetSize(1280,1024);
	}
	*/
	prosper::util::record_image_barrier(
		**drawCmd,**imgOutput,
		Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
		Anvil::ImageLayout::GENERAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
		Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);
	shader.EndCompute();
	return true;
}
bool RaytracingRenderer::ReloadRenderTarget()
{
	// TODO
	return true;
}
const std::shared_ptr<prosper::Texture> &RaytracingRenderer::GetSceneTexture() const {return m_outputTexture;}
void RaytracingRenderer::EndRendering() {}
Anvil::DescriptorSet &RaytracingRenderer::GetOutputImageDescriptorSet() {return *m_dsgOutputImage->GetAnvilDescriptorSetGroup().get_descriptor_set(0);}
