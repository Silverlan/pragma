#include "stdafx_client.h"
#include "pragma/rendering/renderers/raytracing_renderer.hpp"
#include "pragma/rendering/shaders/world/raytracing/c_shader_raytracing.hpp"
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma::rendering;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
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
	prosper::util::set_descriptor_set_binding_storage_image(*descSetImage->GetDescriptorSet(),*m_outputTexture,0u);
	m_dsgOutputImage = descSetImage;

	m_dsgLights = prosper::util::create_descriptor_set_group(c_engine->GetDevice(),pragma::ShaderRayTracing::DESCRIPTOR_SET_LIGHTS);
	prosper::util::set_descriptor_set_binding_storage_buffer(
		*m_dsgLights->GetDescriptorSet(),const_cast<prosper::UniformResizableBuffer&>(pragma::CLightComponent::GetGlobalRenderBuffer()),0
	);

	m_whShader = c_engine->GetShader("raytracing");
	return m_whShader.valid() && CRaytracingComponent::InitializeBuffers();
}
bool RaytracingRenderer::IsRayTracingRenderer() const {return true;}
void RaytracingRenderer::OnEntityAddedToScene(CBaseEntity &ent) {ent.AddComponent<CRaytracingComponent>();}
//#include <wgui/types/wirect.h>
bool RaytracingRenderer::RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags)
{
	if(m_whShader.expired() || BaseRenderer::RenderScene(drawCmd,renderFlags) == false)
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
	pushConstants.renderFlags = ShaderRayTracing::RenderFlags::None;
	if(umath::is_flag_set(renderFlags,FRender::Skybox))
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::RenderSkybox);
	if(umath::is_flag_set(renderFlags,FRender::View))
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::RenderView);
	if(umath::is_flag_set(renderFlags,FRender::Water))
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::RenderWater);
	if(umath::is_flag_set(renderFlags,FRender::World))
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::RenderWorld);

	auto *dsIBL = CReflectionProbeComponent::FindDescriptorSetForClosestProbe(cam->GetEntity().GetPosition());
	if(dsIBL == nullptr)
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::NoIBL);

	auto &dsgCam = scene.GetCameraDescriptorSetGroup(vk::PipelineBindPoint::eCompute);
	auto dsgGameScene = CRaytracingComponent::GetGameSceneDescriptorSetGroup();
	if(dsgGameScene == nullptr)
		return false;

	// Update entity buffers
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CRaytracingComponent>>();
	entIt.AttachFilter<TEntityIteratorFilterComponent<CRenderComponent>>();
	for(auto *ent : entIt)
	{
		auto renderC = ent->GetComponent<CRenderComponent>();
		renderC->UpdateRenderData(drawCmd);
	}

	constexpr uint8_t localWorkGroupSize = 4;
	auto width = extents.width;
	auto height = extents.height;
	if((width %localWorkGroupSize) != 0 || (height %localWorkGroupSize) != 0)
		Con::cwar<<"WARNING: Raytracing output image resolution "<<width<<"x"<<height<<" is not a multiple of 4! Image may become partially black."<<Con::endl;

	// Resolution is limited by ShaderRaytracing::PushConstants::pxOffset only
	constexpr auto maxResolution = std::numeric_limits<uint16_t>::max();
	if(width > maxResolution || height > maxResolution)
		Con::cwar<<"WARNING: Raytracing output image resolution exceeds limit of "<<maxResolution<<"! Image may become partially black."<<Con::endl;
	width = umath::min(width,static_cast<uint32_t>(maxResolution));
	height = umath::min(height,static_cast<uint32_t>(maxResolution));

	auto numWorkGroupsX = width /localWorkGroupSize;
	auto numWorkGroupsY = height /localWorkGroupSize;

	//
	const auto fFlushCommandBuffer = [&drawCmd]() {
		drawCmd->StopRecording();

		auto &dev = c_engine->GetDevice();
		dev.get_universal_queue(0)->submit(Anvil::SubmitInfo::create(
			&drawCmd->GetAnvilCommandBuffer(),0u,nullptr,
			0u,nullptr,nullptr,true
		));
		drawCmd->StartRecording();
	};
	//
	fFlushCommandBuffer();

	auto &shader = static_cast<ShaderRayTracing&>(*m_whShader);
	constexpr uint8_t numWorkGroupsPerIterationSqr = 2; // Lower this value if the GPU's TDR is being hit
	// Number of batches to record before flushing the command buffer. If this value is too low, execution time will
	// increase drastically. If the value is too high, the GPU's TDR may be hit!
	constexpr uint8_t flushBatchCount = 20;
	uint8_t batchCounter = 0;
	auto numWorkGroupsTotal = numWorkGroupsX *numWorkGroupsY;
	for(auto x=decltype(numWorkGroupsX){0u};x<numWorkGroupsX;x+=numWorkGroupsPerIterationSqr)
	{
		for(auto y=decltype(numWorkGroupsY){0u};y<numWorkGroupsY;y+=numWorkGroupsPerIterationSqr)
		{
			if(shader.BeginCompute(drawCmd) == false)
				return false;
			auto &pixelOffset = pushConstants.pxOffset;
			pixelOffset = y *localWorkGroupSize;
			pixelOffset<<=16;
			pixelOffset |= static_cast<uint16_t>(x *localWorkGroupSize);
			shader.Compute(
				pushConstants,GetOutputImageDescriptorSet(),
				*(*dsgGameScene)->get_descriptor_set(0),*(*dsgCam)->get_descriptor_set(0),
				*(*m_dsgLights)->get_descriptor_set(0),
				dsIBL,numWorkGroupsPerIterationSqr,numWorkGroupsPerIterationSqr
			);
			shader.EndCompute();

			if(++batchCounter == flushBatchCount)
			{
				batchCounter = 0;
				fFlushCommandBuffer();
			}

			auto progress = ((x *numWorkGroupsY) +y) /static_cast<float>(numWorkGroupsTotal);
			Con::cout<<"Raytracing progress: "<<(progress *100.f)<<"%"<<Con::endl;
		}
	}
	prosper::util::record_image_barrier(
		**drawCmd,**imgOutput,
		Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
		Anvil::ImageLayout::GENERAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
		Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);
	return true;
}
bool RaytracingRenderer::ReloadRenderTarget()
{
	// TODO
	return true;
}
const std::shared_ptr<prosper::Texture> &RaytracingRenderer::GetSceneTexture() const {return m_outputTexture;}
const std::shared_ptr<prosper::Texture> &RaytracingRenderer::GetHDRPresentationTexture() const
{
	// TODO: Add actual HDR texture
	return m_outputTexture;
}
void RaytracingRenderer::EndRendering() {}
Anvil::DescriptorSet &RaytracingRenderer::GetOutputImageDescriptorSet() {return *m_dsgOutputImage->GetAnvilDescriptorSetGroup().get_descriptor_set(0);}
#pragma optimize("",on)
