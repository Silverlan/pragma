/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/game/c_game.h"
#include <image/prosper_msaa_texture.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

void RasterizationRenderer::CullLightSources(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene == nullptr)
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &prepass = GetPrepass();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	{
		c_game->StartProfilingStage(CGame::CPUProfilingPhase::CullLightSources);
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::CullLightSources);
		auto depthTex = prepass.textureDepth;
		auto bMultisampled = depthTex->IsMSAATexture();
		if(depthTex->IsMSAATexture())
		{
			depthTex = static_cast<prosper::MSAATexture&>(*depthTex).Resolve(
				*drawCmd,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal,
				prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal
			);
		}
		else
			drawCmd->RecordImageBarrier(depthTex->GetImage(),prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);

		static std::vector<pragma::CLightComponent*> culledLightSources;
		culledLightSources.clear();
		auto &fp = GetForwardPlusInstance();

		// Camera buffer
		drawCmd->RecordBufferBarrier(
			*scene.GetCameraBuffer(),
			prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::ComputeShaderBit,
			prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
		);

		// Render settings buffer
		drawCmd->RecordBufferBarrier(
			*scene.GetRenderSettingsBuffer(),
			prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::ComputeShaderBit,
			prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
		);

		auto *worldEnv = scene.GetWorldEnvironment();
		if(worldEnv && worldEnv->IsUnlit() == false)
		{
			fp.Compute(*drawCmd,scene,depthTex->GetImage(),*scene.GetCameraDescriptorSetCompute());
			auto &lightBits = fp.GetShadowLightBits();
			for(auto i=decltype(lightBits.size()){0};i<lightBits.size();++i)
			{
				auto &intVal = lightBits.at(i);
				const auto numBits = 32u;
				for(auto j=0u;j<numBits;++j)
				{
					if(!(intVal &(1<<j))) // If bit is set, this light is visible on screen
						continue;
					auto shadowIdx = i *numBits +j;
					auto *l = pragma::CLightComponent::GetLightByShadowBufferIndex(shadowIdx);
					if(l == nullptr || static_cast<CBaseEntity&>(l->GetEntity()).IsInScene(scene) == false)
						continue;
					culledLightSources.push_back(l);

					auto &renderBuffer = l->GetRenderBuffer();
					if(renderBuffer)
					{
						drawCmd->RecordBufferBarrier(
							*renderBuffer,
							prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
							prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
						);
					}
					auto &shadowBuffer = l->GetShadowBuffer();
					if(shadowBuffer)
					{
						drawCmd->RecordBufferBarrier(
							*shadowBuffer,
							prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
							prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
						);
					}
				}
			}
		}

		// Don't write to depth image until compute shader has completed reading from it
		if(!bMultisampled)
			drawCmd->RecordImageBarrier(depthTex->GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal);

		c_game->StopProfilingStage(CGame::GPUProfilingPhase::CullLightSources);
		c_game->StopProfilingStage(CGame::CPUProfilingPhase::CullLightSources);

		c_game->StartProfilingStage(CGame::CPUProfilingPhase::Shadows);
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Shadows);
		// Update shadows
		//c_engine->StartGPUTimer(GPUTimerEvent::Shadow); // TODO: Only for main scene // prosper TODO

		// Entity instance buffer barrier
		drawCmd->RecordBufferBarrier(
			*pragma::CRenderComponent::GetInstanceBuffer(),
			prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
			prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
		);

		// Entity bone buffer barrier
		drawCmd->RecordBufferBarrier(
			*pragma::get_instance_bone_buffer(),
			prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
			prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
		);

		if(worldEnv && worldEnv->IsUnlit() == false)
		{
			// Note: We want light sources to re-use their previous render target texture if they have one.
			// To achieve this, we simply update their render target by calling 'RequestRenderTarget' again.
			// Shadowmaps that don't have a render target assigned, will get one in 'RenderSystem::RenderShadows'.
			for(auto *l : culledLightSources)
			{
				auto hSm = l->GetShadowMap(pragma::CLightComponent::ShadowMapType::Static);
				if(hSm.valid() && hSm->HasRenderTarget())
					hSm->RequestRenderTarget();

				hSm = l->GetShadowMap(pragma::CLightComponent::ShadowMapType::Dynamic);
				if(hSm.valid() && hSm->HasRenderTarget())
					hSm->RequestRenderTarget();
			}
			RenderSystem::RenderShadows(drawSceneInfo,*this,culledLightSources);
		}
		//c_engine->StopGPUTimer(GPUTimerEvent::Shadow); // prosper TODO
		//drawCmd->SetViewport(w,h); // Reset the viewport

		//auto &imgDepth = textureDepth->GetImage(); // prosper TODO
		//imgDepth->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal); // prosper TODO
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Shadows);
		c_game->StopProfilingStage(CGame::CPUProfilingPhase::Shadows);
	}
}
