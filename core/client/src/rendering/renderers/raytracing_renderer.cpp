/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/renderers/raytracing_renderer.hpp"
#include "pragma/rendering/shaders/world/raytracing/c_shader_raytracing.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma::rendering;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

bool RaytracingRenderer::Initialize()
{
	auto &scene = GetScene();
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.width = scene.GetWidth();
	imgCreateInfo.height = scene.GetHeight();
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.usage = prosper::ImageUsageFlags::StorageBit | prosper::ImageUsageFlags::TransferSrcBit | prosper::ImageUsageFlags::SampledBit;

	auto img = c_engine->GetRenderContext().CreateImage(imgCreateInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	m_outputTexture = c_engine->GetRenderContext().CreateTexture(prosper::util::TextureCreateInfo{},*img,imgViewCreateInfo,samplerCreateInfo);
	auto descSetImage = c_engine->GetRenderContext().CreateDescriptorSetGroup(ShaderRayTracing::DESCRIPTOR_SET_IMAGE_OUTPUT);
	descSetImage->GetDescriptorSet()->SetBindingStorageImage(*m_outputTexture,0u);
	m_dsgOutputImage = descSetImage;

	m_dsgLights = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderRayTracing::DESCRIPTOR_SET_LIGHTS);
	m_dsgLights->GetDescriptorSet()->SetBindingStorageBuffer(
		const_cast<prosper::IUniformResizableBuffer&>(pragma::CLightComponent::GetGlobalRenderBuffer()),0
	);

	m_whShader = c_engine->GetShader("raytracing");
	return m_whShader.valid() && CRaytracingComponent::InitializeBuffers();
}
bool RaytracingRenderer::IsRayTracingRenderer() const {return true;}
// TODO
// void RaytracingRenderer::OnEntityAddedToScene(CBaseEntity &ent) {ent.AddComponent<CRaytracingComponent>();}
//#include <wgui/types/wirect.h>
bool RaytracingRenderer::RenderScene(const util::DrawSceneInfo &drawSceneInfo)
{
	if(m_whShader.expired() || BaseRenderer::RenderScene(drawSceneInfo) == false)
		return false;
	auto &imgOutput = GetSceneTexture()->GetImage();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	drawCmd->RecordImageBarrier(
		imgOutput,
		prosper::PipelineStageFlags::FragmentShaderBit,prosper::PipelineStageFlags::ComputeShaderBit,
		prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::General,
		prosper::AccessFlags::ShaderReadBit,prosper::AccessFlags::ShaderWriteBit
	);

	auto &scene = GetScene();
	auto &cam = scene.GetActiveCamera();
	auto extents = imgOutput.GetExtents();
	ShaderRayTracing::PushConstants pushConstants {
		pragma::CRaytracingComponent::GetBufferMeshCount(),pragma::CLightComponent::GetLightCount(),
		extents.width,extents.height,cam.valid() ? cam->GetFOVRad() : umath::deg_to_rad(pragma::CCameraComponent::DEFAULT_FOV)
	};
	pushConstants.renderFlags = ShaderRayTracing::RenderFlags::None;
	if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::Skybox))
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::RenderSkybox);
	if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::View))
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::RenderView);
	if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::Water))
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::RenderWater);
	if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::World))
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::RenderWorld);

	float intensity;
	auto *dsIBL = CReflectionProbeComponent::FindDescriptorSetForClosestProbe(scene,cam->GetEntity().GetPosition(),intensity);
	if(dsIBL == nullptr)
		umath::set_flag(pushConstants.renderFlags,ShaderRayTracing::RenderFlags::NoIBL);

	auto &dsgCam = scene.GetCameraDescriptorSetGroup(prosper::PipelineBindPoint::Compute);
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
		c_engine->GetRenderContext().Submit(*drawCmd,true);
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
				*dsgGameScene->GetDescriptorSet(),*dsgCam->GetDescriptorSet(),
				*m_dsgLights->GetDescriptorSet(),
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
	drawCmd->RecordImageBarrier(
		imgOutput,
		prosper::PipelineStageFlags::ComputeShaderBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::ImageLayout::General,prosper::ImageLayout::ShaderReadOnlyOptimal,
		prosper::AccessFlags::ShaderWriteBit,prosper::AccessFlags::ShaderReadBit
	);
	return true;
}
bool RaytracingRenderer::ReloadRenderTarget(uint32_t width,uint32_t height)
{
	// TODO
	return true;
}
prosper::Texture *RaytracingRenderer::GetSceneTexture() {return m_outputTexture.get();}
prosper::Texture *RaytracingRenderer::GetHDRPresentationTexture()
{
	// TODO: Add actual HDR texture
	return m_outputTexture.get();
}
void RaytracingRenderer::EndRendering() {}
prosper::IDescriptorSet &RaytracingRenderer::GetOutputImageDescriptorSet() {return *m_dsgOutputImage->GetDescriptorSet();}

