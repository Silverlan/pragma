/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/game/c_game.h"
#include "pragma/console/c_cvar.h"
#include <pragma/console/convars.h>
#include <prosper_util.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;

static auto cvDrawParticles = GetClientConVar("render_draw_particles");
static auto cvDrawGlow = GetClientConVar("render_draw_glow");
static auto cvDrawTranslucent = GetClientConVar("render_draw_translucent");

void RasterizationRenderer::RenderLightingPass(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,FRender renderFlags)
{
	auto &scene = GetScene();
	auto &cam = scene.GetActiveCamera();
	bool bShadows = (renderFlags &FRender::Shadows) == FRender::Shadows;
	auto &renderMeshes = GetCulledMeshes();

	//ScopeGuard sgDepthImg {};
	auto &culledParticles = GetCulledParticles();
	auto bShouldDrawParticles = (renderFlags &FRender::Particles) == FRender::Particles && cvDrawParticles->GetBool() == true && culledParticles.empty() == false;
	if(bShouldDrawParticles == true)
		SetFrameDepthBufferSamplingRequired();
	if(m_bFrameDepthBufferSamplingRequired == true)
	{
		auto &prepass = GetPrepass();
		auto &dstDepthTex = *prepass.textureDepthSampled;
		auto &ptrDstDepthImg = dstDepthTex.GetImage();
		auto &dstDepthImg = ptrDstDepthImg;

		auto &hdrInfo = GetHDRInfo();
		std::function<void(prosper::ICommandBuffer&)> fTransitionSampleImgToTransferDst = nullptr;
		hdrInfo.BlitMainDepthBufferToSamplableDepthBuffer(*drawCmd,fTransitionSampleImgToTransferDst);

		// If this being commented causes issues, check map test_particles for comparison
		//sgDepthImg = [fTransitionSampleImgToTransferDst,drawCmd,ptrDstDepthImg]() { // Transfer destination image back to TransferDstOptimal layout after render pass has ended
		//.RecordImageBarrier(**drawCmd,**ptrDstDepthImg,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);
		//fTransitionSampleImgToTransferDst(*drawCmd);
		//};
	}
	m_bFrameDepthBufferSamplingRequired = false;

	// Visible light tile index buffer
	drawCmd->RecordBufferBarrier(
		*GetForwardPlusInstance().GetTileVisLightIndexBuffer(),
		prosper::PipelineStageFlags::ComputeShaderBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::ShaderWriteBit,prosper::AccessFlags::ShaderReadBit
	);

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

	auto &bufLightSources = pragma::CLightComponent::GetGlobalRenderBuffer();
	auto &bufShadowData = pragma::CLightComponent::GetGlobalShadowBuffer();
	// Light source data barrier
	drawCmd->RecordBufferBarrier(
		bufLightSources,
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);

	// Shadow data barrier
	drawCmd->RecordBufferBarrier(
		bufShadowData,
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);

	auto &renderSettingsBufferData = c_game->GetGlobalRenderSettingsBufferData();
	// Debug buffer barrier
	drawCmd->RecordBufferBarrier(
		*renderSettingsBufferData.debugBuffer,
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);

	// Time buffer barrier
	drawCmd->RecordBufferBarrier(
		*renderSettingsBufferData.timeBuffer,
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);

	// CSM buffer barrier
	drawCmd->RecordBufferBarrier(
		*renderSettingsBufferData.csmBuffer,
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);

	// Camera buffer barrier
	drawCmd->RecordBufferBarrier(
		*scene.GetCameraBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);

	// Render settings buffer barrier
	drawCmd->RecordBufferBarrier(
		*scene.GetRenderSettingsBuffer(),
		prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit,
		prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
	);

	if(BeginRenderPass(drawCmd) == false)
		return;

	c_game->StartProfilingStage(CGame::CPUProfilingPhase::RenderWorld);
	auto bGlow = cvDrawGlow->GetBool();
	auto bTranslucent = cvDrawTranslucent->GetBool();
	auto rsFlags = RenderSystem::RenderFlags::None;
	if(umath::is_flag_set(renderFlags,FRender::Reflection))
		rsFlags |= RenderSystem::RenderFlags::Reflection;
	//c_engine->StartGPUTimer(GPUTimerEvent::Skybox); // prosper TODO
	if((renderFlags &FRender::Skybox) != FRender::None)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Skybox);
		c_game->CallCallbacks("PreRenderSkybox");

		RenderSystem::Render(drawCmd,RenderMode::Skybox,rsFlags);

		// 3D Skybox
		for(auto &hSkyCam : m_3dSkyCameras)
		{
			auto filteredMeshes = hSkyCam.valid() ? hSkyCam->GetRenderMeshCollectionHandler().GetRenderMeshData(RenderMode::World) : nullptr;
			if(filteredMeshes == nullptr)
				continue;
			auto &ent = hSkyCam->GetEntity();
			auto &pos = ent.GetPosition();
			Vector4 drawOrigin {pos.x,pos.y,pos.z,hSkyCam->GetSkyboxScale()};
			RenderSystem::Render(drawCmd,*filteredMeshes,RenderMode::Skybox,rsFlags | RenderSystem::RenderFlags::RenderAs3DSky,drawOrigin);
		}

		c_game->CallCallbacks("PostRenderSkybox");
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Skybox);
	}

	if((renderFlags &FRender::World) != FRender::None)
	{
		//#ifdef _DEBUG
		//#error "Handle this via RenderMode, not renderflags!"
		//#endif
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::World);
		c_game->CallCallbacks("PreRenderWorld");

		//c_engine->StartGPUTimer(GPUTimerEvent::World); // prosper TODO
		RenderSystem::Render(drawCmd,RenderMode::World,rsFlags);
		//c_engine->StopGPUTimer(GPUTimerEvent::World); // prosper TODO

		//c_engine->StartGPUTimer(GPUTimerEvent::WorldTranslucent); // prosper TODO
		auto *renderInfo = GetRenderInfo(RenderMode::World);
		if(renderInfo != nullptr && cam.valid())
			RenderSystem::Render(drawCmd,*cam,RenderMode::World,rsFlags,renderInfo->translucentMeshes);
		//c_engine->StopGPUTimer(GPUTimerEvent::WorldTranslucent); // prosper TODO

		c_game->CallCallbacks("PostRenderWorld");
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::World);
	}

	// Start particle sub-pass
	//prosper::util::record_next_sub_pass(**drawCmd);

	//if((renderFlags &FRENDER_WORLD) == FRENDER_WORLD)
	//	RenderWorldEntities(interpolation,entsRender);
	//c_engine->StartGPUTimer(GPUTimerEvent::Particles); // prosper TODO
	if(bShouldDrawParticles)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Particles);
		c_game->CallCallbacks("PreRenderParticles");

		auto &glowInfo = GetGlowInfo();

		// Vertex buffer barrier
		drawCmd->RecordBufferBarrier(
			*pragma::CParticleSystemComponent::GetGlobalVertexBuffer(),
			prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::VertexInputBit,
			prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::VertexAttributeReadBit
		);

		for(auto *particle : culledParticles)
		{
			auto &ptBuffer = particle->GetParticleBuffer();
			if (ptBuffer != nullptr)
			{
				// Particle buffer barrier
				drawCmd->RecordBufferBarrier(
					*ptBuffer,
					prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::VertexInputBit,
					prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::VertexAttributeReadBit
				);
			}

			auto &animStartBuffer = particle->GetAnimationStartBuffer();
			if (animStartBuffer != nullptr)
			{
				// Animation start buffer barrier
				drawCmd->RecordBufferBarrier(
					*animStartBuffer,
					prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::VertexInputBit,
					prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::VertexAttributeReadBit
				);
			}

			auto &animBuffer = particle->GetAnimationBuffer();
			if (animBuffer != nullptr)
			{
				// Animation buffer barrier
				drawCmd->RecordBufferBarrier(
					*animBuffer,
					prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit,
					prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit
				);
			}
		}

		RenderParticleSystems(drawCmd,culledParticles,RenderMode::World,false,&glowInfo.tmpBloomParticles);
		if(bGlow == false)
			glowInfo.tmpBloomParticles.clear();
		if(!glowInfo.tmpBloomParticles.empty())
			glowInfo.bGlowScheduled = true;

		c_game->CallCallbacks("PostRenderParticles");
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Particles);
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::Particles); // prosper TODO

	c_game->CallCallbacks<void>("Render");
	c_game->CallLuaCallbacks("Render");

	//c_engine->StartGPUTimer(GPUTimerEvent::Debug); // prosper TODO
	if((renderFlags &FRender::Debug) == FRender::Debug)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Debug);
		c_game->CallCallbacks("PreRenderDebug");

		if(cam.valid())
			DebugRenderer::Render(drawCmd,*cam);
		c_game->RenderDebugPhysics(drawCmd,*cam);
		c_game->CallCallbacks("PostRenderDebug");
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Debug);
	}
	//c_engine->StopGPUTimer(GPUTimerEvent::Debug); // prosper TODO

	if((renderFlags &FRender::Water) != FRender::None)
	{
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Water);
		c_game->CallCallbacks("PreRenderWater");

		//c_engine->StartGPUTimer(GPUTimerEvent::Water); // prosper TODO
		auto numShaderInvocations = RenderSystem::Render(drawCmd,RenderMode::Water,rsFlags);
		//c_engine->StopGPUTimer(GPUTimerEvent::Water); // prosper TODO

		c_game->CallCallbacks("PostRenderWater");

		if(numShaderInvocations > 0)
		{
			//auto &prepass = scene->GetPrepass(); // prosper TODO
			// Note: The texture buffer was probably already resolved at this point (e.g. by particle render pass), however
			// we will need the water depth values for the upcoming render passes as well, so we'll have to resolve again.
			// To do that, we'll reset the MSAA texture, so at the next resolve-call it will update accordingly.
			//prepass.textureDepth->Reset(); // prosper TODO
			//prepass.textureDepth->Resolve(); // prosper TODO
		}
		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Water);
	}

	if((renderFlags &FRender::View) != FRender::None)
	{
		auto *pl = c_game->GetLocalPlayer();
		if(pl != nullptr && pl->IsInFirstPersonMode())
		{
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::View);
			c_game->CallCallbacks("PreRenderView");

			//c_engine->StartGPUTimer(GPUTimerEvent::View); // prosper TODO
			RenderSystem::Render(drawCmd,RenderMode::View,rsFlags);
			//c_engine->StopGPUTimer(GPUTimerEvent::View); // prosper TODO

			//c_engine->StartGPUTimer(GPUTimerEvent::ViewParticles); // prosper TODO
			if((renderFlags &FRender::Particles) == FRender::Particles)
			{
				auto &culledParticles = GetCulledParticles();
				auto &glowInfo = GetGlowInfo();
				RenderParticleSystems(drawCmd,culledParticles,RenderMode::View,false,&glowInfo.tmpBloomParticles);
				if(bGlow == false)
					glowInfo.tmpBloomParticles.clear();
				if(!glowInfo.tmpBloomParticles.empty())
					glowInfo.bGlowScheduled = true;
			}
			//c_engine->StartGPUTimer(GPUTimerEvent::ViewParticles); // prosper TODO
			//RenderGlowMeshes(cam,true);

			c_game->CallCallbacks("PostRenderView");
			c_game->StopProfilingStage(CGame::GPUProfilingPhase::View);
		}
	}
	EndRenderPass(drawCmd);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::RenderWorld);
}

