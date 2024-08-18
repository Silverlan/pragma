/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/info/c_shader_velocity_buffer.hpp"
#include "pragma/rendering/c_renderflags.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/game/c_game.h"
#include "pragma/console/c_cvar.h"
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <pragma/lua/luafunction_call.h>
#include <pragma/console/convars.h>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

#define ENABLE_PARTICLE_RENDERING 1

static auto cvDrawParticles = GetClientConVar("render_draw_particles");
static auto cvDrawGlow = GetClientConVar("render_draw_glow");
static auto cvDrawTranslucent = GetClientConVar("render_draw_translucent");

#if DEBUG_RENDER_PERFORMANCE_TEST_ENABLED == 1
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/rendering/shaders/world/c_shader_test.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/model/c_model.h"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
int g_dbgMode = 5;
#endif

void pragma::CRasterizationRendererComponent::RecordPrepass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &prepass = GetPrepass();

	auto &shaderPrepass = GetPrepassShader();
	auto *prepassStats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr;
	auto &worldRenderQueues = sceneRenderDesc.GetWorldRenderQueues();
	m_prepassCommandBufferGroup->Record([this, &drawSceneInfo, &shaderPrepass, &worldRenderQueues, &sceneRenderDesc, prepassStats](prosper::ISecondaryCommandBuffer &cmd) {
		util::RenderPassDrawInfo renderPassDrawInfo {drawSceneInfo, cmd};
		pragma::rendering::DepthStageRenderProcessor rsys {
		  renderPassDrawInfo,
		  {} /* drawOrigin */,
		};

		CEPrepassStageData evDataPrepassStage {rsys, shaderPrepass};
		InvokeEventCallbacks(EVENT_MT_BEGIN_RECORD_PREPASS, evDataPrepassStage);
		rsys.BindShader(shaderPrepass, umath::to_integral(pragma::ShaderPrepass::Pipeline::Opaque));
		// Render static world geometry
		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
			std::chrono::steady_clock::time_point t;
			if(drawSceneInfo.renderStats)
				t = std::chrono::steady_clock::now();
			sceneRenderDesc.WaitForWorldRenderQueues();
			if(drawSceneInfo.renderStats)
				drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass)->SetTime(RenderPassStats::Timer::RenderThreadWait, std::chrono::steady_clock::now() - t);
			for(auto i = decltype(worldRenderQueues.size()) {0u}; i < worldRenderQueues.size(); ++i)
				rsys.Render(*worldRenderQueues.at(i), prepassStats, i);
		}

		// Note: The non-translucent render queues also include transparent (alpha masked) objects.
		// We don't care about translucent objects here.
		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
			rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World, false /* translucent */), prepassStats);

			auto &queueTranslucent = *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World, true /* translucent */);
			queueTranslucent.WaitForCompletion(prepassStats);
			if(queueTranslucent.queue.empty() == false) {
				// rsys.BindShader(shaderPrepass,umath::to_integral(pragma::ShaderPrepass::Pipeline::AlphaTest));
				rsys.Render(queueTranslucent, prepassStats);
			}
		}

		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::View) != RenderFlags::None) {
			auto &queue = *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::View, false /* translucent */);
			queue.WaitForCompletion(prepassStats);
			if(queue.queue.empty() == false) {
				rsys.UnbindShader();
				rsys.SetCameraType(pragma::rendering::BaseRenderProcessor::CameraType::View);
				rsys.BindShader(shaderPrepass, umath::to_integral(pragma::ShaderPrepass::Pipeline::Opaque));
				// rsys.BindShader(shaderPrepass,umath::to_integral(pragma::ShaderPrepass::Pipeline::Opaque));
				rsys.Render(queue, prepassStats);
			}
		}
		rsys.UnbindShader();

		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::Particles) != RenderFlags::None)
			RenderParticles(cmd, drawSceneInfo, true);
	});
}
void pragma::CRasterizationRendererComponent::UpdatePrepassRenderBuffers(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	// Prepass and lighting pass are now being recorded in parallel on separate threads.
	// In the meantime, we can make use of the wait time by updating the entity buffers
	// for the entity we need for the prepass.
	if((drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
		CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World, false /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
		CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World, true /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
	}

	if((drawSceneInfo.renderFlags & RenderFlags::View) != RenderFlags::None) {
		CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::View, false /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
		CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::View, true /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
	}

	c_game->CallLuaCallbacks<void, const util::DrawSceneInfo *>("UpdateRenderBuffers", &drawSceneInfo);
	//
}
void pragma::CRasterizationRendererComponent::UpdateLightingPassRenderBuffers(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::Sky, false /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass) : nullptr);
	CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::Sky, true /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass) : nullptr);
	//CSceneComponent::UpdateRenderBuffers(drawCmd,*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World,true /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->lightingPass : nullptr);
	CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::View, true /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass) : nullptr);
}
void pragma::CRasterizationRendererComponent::ExecutePrepass(const util::DrawSceneInfo &drawSceneInfo)
{
	if(umath::is_flag_set(drawSceneInfo.flags, util::DrawSceneInfo::Flags::DisablePrepass))
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &hCam = scene.GetActiveCamera();
	// Pre-render depths and normals (if SSAO is enabled)
	c_game->StartProfilingStage("Prepass");
	c_game->StartGPUProfilingStage("Prepass");
	auto &prepass = GetPrepass();
	if(prepass.textureDepth->IsMSAATexture())
		static_cast<prosper::MSAATexture &>(*prepass.textureDepth).Reset();
	if(prepass.textureNormals != nullptr && prepass.textureNormals->IsMSAATexture())
		static_cast<prosper::MSAATexture &>(*prepass.textureNormals).Reset();

	// Entity instance buffer barrier
	auto &drawCmd = drawSceneInfo.commandBuffer;
	drawCmd->RecordBufferBarrier(*pragma::CRenderComponent::GetInstanceBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Entity bone buffer barrier
	drawCmd->RecordBufferBarrier(*pragma::get_instance_bone_buffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Camera buffer barrier
	drawCmd->RecordBufferBarrier(*scene.GetCameraBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	// View camera buffer barrier
	drawCmd->RecordBufferBarrier(*scene.GetViewCameraBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	CEDrawSceneInfo evData {drawSceneInfo};
	InvokeEventCallbacks(EVENT_PRE_PREPASS, evData);

	prepass.BeginRenderPass(drawSceneInfo, nullptr, true);

	InvokeEventCallbacks(EVENT_PRE_EXECUTE_PREPASS, evData);
	m_prepassCommandBufferGroup->ExecuteCommands(*drawCmd);
	InvokeEventCallbacks(EVENT_POST_EXECUTE_PREPASS, evData);

	prepass.EndRenderPass(drawSceneInfo);

	InvokeEventCallbacks(EVENT_POST_PREPASS, evData);

	c_game->StopGPUProfilingStage(); // Prepass
	c_game->StopProfilingStage();    // Prepass
}

void pragma::CRasterizationRendererComponent::ExecuteLightingPass(const util::DrawSceneInfo &drawSceneInfo)
{
	if(umath::is_flag_set(drawSceneInfo.flags, util::DrawSceneInfo::Flags::DisablePrepass))
		return;
	c_game->StartProfilingStage("ExecuteLightingPass");
	auto &scene = const_cast<pragma::CSceneComponent &>(*drawSceneInfo.scene);
	auto &cam = scene.GetActiveCamera();
	bool bShadows = (drawSceneInfo.renderFlags & RenderFlags::Shadows) == RenderFlags::Shadows;
	auto &drawCmd = drawSceneInfo.commandBuffer;

	//ScopeGuard sgDepthImg {};
	m_bFrameDepthBufferSamplingRequired = false;

	// Visible light tile index buffer
	drawCmd->RecordBufferBarrier(*GetForwardPlusInstance().GetTileVisLightIndexBuffer(), prosper::PipelineStageFlags::ComputeShaderBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::ShaderWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Entity instance buffer barrier
	drawCmd->RecordBufferBarrier(*pragma::CRenderComponent::GetInstanceBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Entity bone buffer barrier
	drawCmd->RecordBufferBarrier(*pragma::get_instance_bone_buffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	auto &bufLightSources = pragma::CLightComponent::GetGlobalRenderBuffer();
	auto &bufShadowData = pragma::CLightComponent::GetGlobalShadowBuffer();
	// Light source data barrier
	drawCmd->RecordBufferBarrier(bufLightSources, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	// Shadow data barrier
	drawCmd->RecordBufferBarrier(bufShadowData, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	auto &renderSettingsBufferData = c_game->GetGlobalRenderSettingsBufferData();
	// Debug buffer barrier
	drawCmd->RecordBufferBarrier(*renderSettingsBufferData.debugBuffer, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Time buffer barrier
	drawCmd->RecordBufferBarrier(*renderSettingsBufferData.timeBuffer, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// CSM buffer barrier
	drawCmd->RecordBufferBarrier(*renderSettingsBufferData.csmBuffer, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Camera buffer barrier
	drawCmd->RecordBufferBarrier(*scene.GetCameraBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	// Render settings buffer barrier
	drawCmd->RecordBufferBarrier(*scene.GetRenderSettingsBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit,
	  prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	CEDrawSceneInfo evData {drawSceneInfo};
	InvokeEventCallbacks(EVENT_PRE_LIGHTING_PASS, evData);

	BeginRenderPass(drawSceneInfo, nullptr, true);

	InvokeEventCallbacks(EVENT_PRE_EXECUTE_LIGHTING_PASS, evData);
	m_lightingCommandBufferGroup->ExecuteCommands(*drawCmd);
	InvokeEventCallbacks(EVENT_POST_EXECUTE_LIGHTING_PASS, evData);

	EndRenderPass(drawSceneInfo);

	InvokeEventCallbacks(EVENT_POST_LIGHTING_PASS, evData);

	c_game->StopProfilingStage(); // ExecuteLightingPass
}
void pragma::CRasterizationRendererComponent::StartPrepassRecording(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &prepass = GetPrepass();
	auto &prepassRt = *prepass.renderTarget;
	m_prepassCommandBufferGroup->StartRecording(prepassRt.GetRenderPass(), prepassRt.GetFramebuffer());

	if(!umath::is_flag_set(drawSceneInfo.flags, util::DrawSceneInfo::Flags::DisablePrepass)) {
		RecordPrepass(drawSceneInfo);
		CEDrawSceneInfo evData {drawSceneInfo};
		InvokeEventCallbacks(EVENT_ON_RECORD_PREPASS, evData);
	}

	m_prepassCommandBufferGroup->EndRecording();
}
void pragma::CRasterizationRendererComponent::StartLightingPassRecording(const util::DrawSceneInfo &drawSceneInfo)
{
	auto *rt = GetLightingPassRenderTarget(drawSceneInfo);
	if(rt == nullptr || drawSceneInfo.scene.expired())
		return;
	m_lightingCommandBufferGroup->StartRecording(rt->GetRenderPass(), rt->GetFramebuffer());

	if(!umath::is_flag_set(drawSceneInfo.flags, util::DrawSceneInfo::Flags::DisableLightingPass)) {
		RecordLightingPass(drawSceneInfo);
		CEDrawSceneInfo evData {drawSceneInfo};
		InvokeEventCallbacks(EVENT_ON_RECORD_LIGHTING_PASS, evData);
	}

	m_lightingCommandBufferGroup->EndRecording();
}
#include <pragma/entities/entity_iterator.hpp>
void pragma::CRasterizationRendererComponent::RecordLightingPass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &scene = const_cast<pragma::CSceneComponent &>(*drawSceneInfo.scene);
	auto &cam = scene.GetActiveCamera();
	m_lightingCommandBufferGroup->Record([this, &scene, &cam, &drawSceneInfo](prosper::ISecondaryCommandBuffer &cmd) mutable {
		c_game->StartProfilingStage("RecordLightingPass");
		auto pcmd = cmd.shared_from_this();
		auto bGlow = cvDrawGlow->GetBool();
		auto bTranslucent = cvDrawTranslucent->GetBool();

		auto *lightingStageStats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass) : nullptr;
		auto *lightingStageTranslucentStats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPassTranslucent) : nullptr;
		util::RenderPassDrawInfo rpDrawInfo {drawSceneInfo, cmd};
		pragma::rendering::LightingStageRenderProcessor rsys {rpDrawInfo, {} /* drawOrigin */};
		auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();

		CELightingStageData evDataLightingStage {rsys};
		if((drawSceneInfo.renderFlags & RenderFlags::Skybox) != RenderFlags::None) {
			c_game->StartProfilingStage("Skybox");
			c_game->StartGPUProfilingStage("Skybox");
			InvokeEventCallbacks(EVENT_MT_BEGIN_RECORD_SKYBOX, evDataLightingStage);

			rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::Sky, false /* translucent */), lightingStageStats);
			rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::Sky, true /* translucent */), lightingStageTranslucentStats);

			InvokeEventCallbacks(EVENT_MT_END_RECORD_SKYBOX, evDataLightingStage);
			c_game->StopGPUProfilingStage(); // Skybox
			c_game->StopProfilingStage(); // Skybox
		}

		// Render static world geometry
#if DEBUG_RENDER_PERFORMANCE_TEST_ENABLED == 1
		if(g_dbgMode == 3 || g_dbgMode == 5) {
#endif
			if((drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
				c_game->StartProfilingStage("World");
				c_game->StartGPUProfilingStage("World");
				InvokeEventCallbacks(EVENT_MT_BEGIN_RECORD_WORLD, evDataLightingStage);

				// For optimization purposes, world geometry is stored in separate render queues.
				// This could be less efficient if many models in the scene use the same materials as
				// the world, but this generally doesn't happen.
				// By rendering world geometry first, we can also avoid overdraw.
				// This does *not* include translucent geometry, which is instead copied over to the
				// general translucency world render queue.

				std::chrono::steady_clock::time_point t;
				if(drawSceneInfo.renderStats)
					t = std::chrono::steady_clock::now();
				sceneRenderDesc.WaitForWorldRenderQueues();
				if(drawSceneInfo.renderStats)
					drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass)->SetTime(RenderPassStats::Timer::RenderThreadWait, std::chrono::steady_clock::now() - t);

				auto &worldRenderQueues = sceneRenderDesc.GetWorldRenderQueues();
				for(auto i = decltype(worldRenderQueues.size()) {0u}; i < worldRenderQueues.size(); ++i)
					rsys.Render(*worldRenderQueues.at(i), lightingStageStats, i);

				// Note: The non-translucent render queues also include transparent (alpha masked) objects
				rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World, false /* translucent */), lightingStageStats);
				rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World, true /* translucent */), lightingStageTranslucentStats);

				InvokeEventCallbacks(EVENT_MT_END_RECORD_WORLD, evDataLightingStage);
				c_game->StopGPUProfilingStage(); // World
				c_game->StopProfilingStage(); // World
			}
#if DEBUG_RENDER_PERFORMANCE_TEST_ENABLED == 1
		}
#endif
#if 0
		if(bShouldDrawParticles)
		{
			//c_game->StartProfilingStage(CGame::GPUProfilingPhase::Particles);
			//InvokeEventCallbacks(EVENT_MT_BEGIN_RECORD_PARTICLES,evDataLightingStage);

			//auto &glowInfo = GetGlowInfo();

			// Vertex buffer barrier
			/*for(auto *particle : culledParticles)
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
			}*/

			// RecordRenderParticleSystems(cmd,drawSceneInfo,culledParticles,pragma::rendering::SceneRenderPass::World,false,nullptr);
			// RenderParticleSystems(drawSceneInfo,culledParticles,RenderMode::World,false,&glowInfo.tmpBloomParticles);
			//if(bGlow == false)
			//	glowInfo.tmpBloomParticles.clear();
			//if(!glowInfo.tmpBloomParticles.empty())
			//	glowInfo.bGlowScheduled = true;
			
			// InvokeEventCallbacks(EVENT_MT_END_RECORD_PARTICLES,evDataLightingStage);
			// c_game->StopProfilingStage(CGame::GPUProfilingPhase::Particles);
		}
#endif
		//c_engine->StopGPUTimer(GPUTimerEvent::Particles); // prosper TODO

		// c_game->CallCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("Render",std::ref(drawSceneInfo));
		// c_game->CallLuaCallbacks<void,const util::DrawSceneInfo*>("Render",&drawSceneInfo);

		if((drawSceneInfo.renderFlags & RenderFlags::Debug) == RenderFlags::Debug) {
			c_game->StartProfilingStage("Debug");
			c_game->StartGPUProfilingStage("Debug");
			InvokeEventCallbacks(EVENT_MT_BEGIN_RECORD_DEBUG, evDataLightingStage);

			if(cam.valid())
				DebugRenderer::Render(pcmd, *cam);
			c_game->RenderDebugPhysics(pcmd, *cam);

#if DEBUG_RENDER_PERFORMANCE_TEST_ENABLED == 1
			EntityIterator entIt {*c_game};
			entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CPropDynamicComponent>>();
			auto it = entIt.begin();
			static std::shared_ptr<prosper::IBuffer> dbgBuffer = nullptr;
			static std::shared_ptr<prosper::IBuffer> instBuffer = nullptr;
			static uint32_t vertCount = 0;
			static std::shared_ptr<ModelSubMesh> mesh = nullptr;
			if(it != entIt.end()) {
				auto *ent = *it;
				auto &subMesh = ent->GetModel()->GetMeshGroups().front()->GetMeshes()[0]->GetSubMeshes()[0];
				if(dbgBuffer == nullptr) {
					auto &meshVerts = subMesh->GetVertices();
					auto &tris = subMesh->GetTriangles();

					std::vector<Vector3> verts {};
					verts.reserve(tris.size());
					for(auto idx : tris)
						verts.push_back(meshVerts[idx].position);

					auto createInfo = prosper::util::BufferCreateInfo {};
					createInfo.size = verts.size() * sizeof(verts.front());
					createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit;
					createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
					dbgBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo, verts.data());
					vertCount = verts.size();
					mesh = subMesh;

					std::vector<uint16_t> indices;
					indices.resize(verts.size());
					for(auto i = decltype(indices.size()) {0u}; i < indices.size(); ++i)
						indices[i] = i;
					createInfo.size = indices.size() * sizeof(indices.front());
					createInfo.usageFlags = prosper::BufferUsageFlags::IndexBufferBit;
					createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
					instBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo, indices.data());
				}
			}

			if(dbgBuffer) {
				auto shader = c_engine->GetShader("test");
				if(shader.valid()) {
					if(g_dbgMode == 0) {
						auto *test = static_cast<pragma::ShaderTest *>(shader.get());
						if(test->BeginDraw(pcmd, {})) // && test->BindEntity(static_cast<CBaseEntity&>(*ent)))
						{
							test->BindSceneCamera(scene, static_cast<pragma::CRasterizationRendererComponent &>(*scene.GetRenderer()), false);
							auto instanceBuffer = CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer();
							//test->Draw(static_cast<CModelSubMesh&>(*mesh),0u,*instanceBuffer);
							test->DrawTest(*dbgBuffer, *instBuffer, vertCount);
							//test->DrawTest(*dbgBuffer,vertCount);
							test->EndDraw();
						}
					}
					else if(g_dbgMode == 1) {
						auto &whDebugShader = c_game->GetGameShader(CGame::GameShader::Debug);
						auto *shader = static_cast<pragma::ShaderDebug *>(whDebugShader.get());

						auto m = umat::identity();
						const Vector2 scale {2.f, 2.f};
						const Vector3 matScale {-scale.x, -scale.y, -1.f};
						m = glm::scale(m, matScale);
						m = cam->GetProjectionMatrix() * cam->GetViewMatrix() * m;

						pragma::ShaderDebug::PushConstants pushConstants {m, Vector4 {1.f, 1.f, 1.f, 1.f}};
						shader->BeginDraw(pcmd);
						shader->Draw(*dbgBuffer, vertCount, m);
						shader->EndDraw();
					}
					else if(g_dbgMode == 2) {
						/*auto shader = c_engine->GetShader("pbr");
				auto *test = static_cast<pragma::ShaderPBR*>(shader.get());
				if(test->BeginDraw(drawCmd,{}) && test->BindEntity(static_cast<CBaseEntity&>(*ent)))
				{
					test->BindScene();
					test->BindEntity();
					test->BindLights();
					test->BindMaterial();
					test->BindSceneCamera(scene,static_cast<pragma::CRasterizationRendererComponent&>(*scene.GetRenderer()),false);
					auto instanceBuffer = CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer();
					test->Draw(static_cast<CModelSubMesh&>(*subMesh),0u,*instanceBuffer);
					//test->DrawTest(*dbgBuffer,vertCount);
					test->EndDraw();
				}*/
					}
				}
			}
#endif

			InvokeEventCallbacks(EVENT_MT_END_RECORD_DEBUG, evDataLightingStage);
			c_game->StopGPUProfilingStage(); // Debug
			c_game->StopProfilingStage(); // Debug
		}

		if((drawSceneInfo.renderFlags & RenderFlags::View) != RenderFlags::None) {
			c_game->StartGPUProfilingStage("View");
			c_game->StartProfilingStage("View");
			InvokeEventCallbacks(EVENT_MT_BEGIN_RECORD_VIEW, evDataLightingStage);

			rsys.SetCameraType(pragma::rendering::BaseRenderProcessor::CameraType::View);
			rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::View, false /* translucent */), lightingStageStats);
			rsys.Render(*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::View, true /* translucent */), lightingStageTranslucentStats);

			//if((drawSceneInfo.renderFlags &RenderFlags::Particles) == RenderFlags::Particles)
			{
				//RenderParticleSystems(drawSceneInfo,culledParticles,pragma::rendering::SceneRenderPass::View,false,nullptr);
				//auto &culledParticles = scene.GetSceneRenderDesc().GetCulledParticles();
				//auto &glowInfo = GetGlowInfo();
				//RenderParticleSystems(drawSceneInfo,culledParticles,RenderMode::View,false,&glowInfo.tmpBloomParticles);
				//if(bGlow == false)
				//	glowInfo.tmpBloomParticles.clear();
				//if(!glowInfo.tmpBloomParticles.empty())
				//	glowInfo.bGlowScheduled = true;
			}
			//RenderGlowMeshes(cam,true);

			InvokeEventCallbacks(EVENT_MT_END_RECORD_VIEW, evDataLightingStage);
			c_game->StopGPUProfilingStage(); // View
			c_game->StopProfilingStage(); // View
		}
		c_game->StopProfilingStage(); // RecordLightingPass
	});
}
