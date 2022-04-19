/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/game/c_game.h"
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

#include "pragma/console/c_cvar.h"
#include <pragma/entities/entity_iterator.hpp>
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#pragma optimize("",off)
static auto cvDrawParticles = GetClientConVar("render_draw_particles");
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include <prosper_descriptor_set_group.hpp>
void pragma::CRasterizationRendererComponent::RenderParticles(const util::DrawSceneInfo &drawSceneInfo)
{
	// TODO: Only render particles if they're visible
	std::vector<pragma::CParticleSystemComponent*> culledParticles;
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	culledParticles.reserve(entIt.GetCount());
	for(auto *ent : entIt)
		culledParticles.push_back(ent->GetComponent<pragma::CParticleSystemComponent>().get());
	auto bShouldDrawParticles = (drawSceneInfo.renderFlags &RenderFlags::Particles) == RenderFlags::Particles && cvDrawParticles->GetBool() == true && culledParticles.empty() == false;

	c_game->StartProfilingStage(CGame::GPUProfilingPhase::Particles);
	///InvokeEventCallbacks(EVENT_MT_BEGIN_RECORD_PARTICLES,evDataLightingStage);

	// Vertex buffer barrier
	auto &cmd = *drawSceneInfo.commandBuffer;
	for(auto *particle : culledParticles)
	{
		auto &ptBuffer = particle->GetParticleBuffer();
		if (ptBuffer != nullptr)
		{
			// Particle buffer barrier
			cmd.RecordBufferBarrier(
				*ptBuffer,
				prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::VertexInputBit,
				prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::VertexAttributeReadBit
			);
		}

		auto &animBuffer = particle->GetParticleAnimationBuffer();
		if (animBuffer != nullptr)
		{
			// Animation start buffer barrier
			cmd.RecordBufferBarrier(
				*animBuffer,
				prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::VertexInputBit,
				prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::VertexAttributeReadBit
			);
		}

		auto &spriteSheetBuffer = particle->GetSpriteSheetBuffer();
		if (spriteSheetBuffer != nullptr)
		{
			// Animation buffer barrier
			cmd.RecordBufferBarrier(
				*spriteSheetBuffer,
				prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit,
				prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit
			);
		}
	}
	
	auto &hdrInfo = GetHDRInfo();
	auto &prepass = hdrInfo.prepass;
	auto texDepth = prepass.textureDepth;

	auto &rt = hdrInfo.GetRenderTarget(drawSceneInfo);
	auto &hdrTex = rt.GetTexture();

	if(drawSceneInfo.renderTarget)
	{
		// Custom render target; Blit to staging target
		auto &imgColorRender = drawSceneInfo.renderTarget->GetTexture().GetImage();
		auto &imgColorSampled = prepass.textureDepthSampled->GetImage();
		cmd.RecordImageBarrier(imgColorSampled,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);
		cmd.RecordImageBarrier(imgColorRender,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal);
		cmd.RecordBlitImage({},imgColorRender,imgColorSampled);
		cmd.RecordImageBarrier(imgColorSampled,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	}
	
	// We need to sample depth buffer, so we need to blit
	auto &imgDepthRender = prepass.textureDepth->GetImage();
	auto &imgDepthSampled = prepass.textureDepthSampled->GetImage();
	cmd.RecordImageBarrier(imgDepthSampled,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);
	cmd.RecordImageBarrier(imgDepthRender,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal);
	cmd.RecordBlitImage({},imgDepthRender,imgDepthSampled);
	cmd.RecordImageBarrier(imgDepthSampled,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	cmd.RecordImageBarrier(imgDepthRender,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal);

	if(cmd.RecordBeginRenderPass(*hdrInfo.rtParticle) == true)
	{
		RecordRenderParticleSystems(cmd,drawSceneInfo,culledParticles,pragma::rendering::SceneRenderPass::World,false,nullptr);
		cmd.RecordEndRenderPass();
	}
	cmd.RecordImageBarrier(texDepth->GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal);
	
	if(drawSceneInfo.renderTarget)
	{
		// Custom render target; Blit from staging target
		auto &imgColorRender = drawSceneInfo.renderTarget->GetTexture().GetImage();
		auto &imgColorSampled = prepass.textureDepthSampled->GetImage();
		cmd.RecordImageBarrier(imgColorSampled,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferSrcOptimal);
		cmd.RecordImageBarrier(imgColorRender,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::TransferDstOptimal);
		cmd.RecordBlitImage({},imgColorRender,imgColorSampled);
		cmd.RecordImageBarrier(imgColorSampled,prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
		cmd.RecordImageBarrier(imgColorRender,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
	}

	//InvokeEventCallbacks(EVENT_MT_END_RECORD_PARTICLES,evDataLightingStage);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::Particles);
}

void pragma::CRasterizationRendererComponent::RenderSceneFog(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &hdrInfo = GetHDRInfo();
	auto descSetGroupFog = m_descSetGroupFogOverride;
	if(descSetGroupFog == nullptr)
	{
		auto *worldEnv = scene.GetWorldEnvironment();
		if(worldEnv != nullptr)
		{
			auto &fog = worldEnv->GetFogSettings();
			if(fog.IsEnabled() == true)
				descSetGroupFog = scene.GetFogDescriptorSetGroup();
		}
	}
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto hShaderFog = c_game->GetGameShader(CGame::GameShader::PPFog);
	if(descSetGroupFog == nullptr || hShaderFog.expired())
		return;
	auto &shaderFog = static_cast<pragma::ShaderPPFog&>(*hShaderFog.get());
	auto &prepass = hdrInfo.prepass;
	auto texDepth = prepass.textureDepth;
	if(texDepth->IsMSAATexture())
	{
		texDepth = static_cast<prosper::MSAATexture&>(*texDepth).Resolve(
			*drawCmd,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal,
			prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal
		);
	}
	else
		drawCmd->RecordImageBarrier(texDepth->GetImage(),prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	//texDepth->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal);

	auto &hdrTex = hdrInfo.sceneRenderTarget->GetTexture();
	drawCmd->RecordImageBarrier(hdrTex.GetImage(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
	drawCmd->RecordBufferBarrier(
		*scene.GetCameraBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);
	drawCmd->RecordBufferBarrier(
		*scene.GetRenderSettingsBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);
	drawCmd->RecordBufferBarrier(
		*scene.GetFogBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);
	if(drawCmd->RecordBeginRenderPass(*hdrInfo.hdrPostProcessingRenderTarget) == true)
	{
		prosper::ShaderBindState bindState {*drawCmd};
		if(shaderFog.RecordBeginDraw(bindState) == true)
		{
			shaderFog.RecordDraw(
				bindState,
				*hdrInfo.dsgHDRPostProcessing->GetDescriptorSet(),
				*hdrInfo.dsgDepthPostProcessing->GetDescriptorSet(),
				*scene.GetCameraDescriptorSetGraphics(),
				*scene.GetFogDescriptorSetGroup()->GetDescriptorSet()
			);
			shaderFog.RecordEndDraw(bindState);
		}
		drawCmd->RecordEndRenderPass();
	}
	drawCmd->RecordImageBarrier(texDepth->GetImage(),prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal);

	hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
}
#pragma optimize("",on)
