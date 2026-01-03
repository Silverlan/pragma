// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :client_state;
import :engine;
import :game;
import :particle_system.enums;
import :rendering.shaders;

using namespace pragma::rendering;

void pragma::CRasterizationRendererComponent::RecordRenderParticleSystems(prosper::ICommandBuffer &cmd, const DrawSceneInfo &drawSceneInfo, const std::vector<ecs::CParticleSystemComponent *> &particles, SceneRenderPass renderMode, bool depthPass, Bool bloom)
{
	auto depthOnly = math::is_flag_set(drawSceneInfo.renderFlags, RenderFlags::ParticleDepth);
	if((depthOnly && bloom) || drawSceneInfo.scene.expired())
		return;
	auto &scene = *drawSceneInfo.scene;
	auto renderFlags = pts::ParticleRenderFlags::None;
	math::set_flag(renderFlags, pts::ParticleRenderFlags::DepthOnly, depthOnly || depthPass);
	math::set_flag(renderFlags, pts::ParticleRenderFlags::Bloom, bloom);
	for(auto *particle : particles) {
		if(particle != nullptr && particle->IsActive() == true && particle->GetSceneRenderPass() == renderMode && particle->GetParent() == nullptr) {
			if(bloom && !particle->IsBloomEnabled())
				continue;
			particle->RecordRender(cmd, const_cast<CSceneComponent &>(scene), *this, renderFlags);
		}
	}
}

static auto cvLockCommandBuffers = pragma::console::get_client_con_var("debug_render_lock_render_command_buffers");
void pragma::CRasterizationRendererComponent::RecordCommandBuffers(const DrawSceneInfo &drawSceneInfo)
{
	// Debug
	static auto debugLockCmdBuffers = false;
	if(cvLockCommandBuffers->GetBool()) {
		if(debugLockCmdBuffers) {
			m_prepassCommandBufferGroup->Reuse();
			m_lightingCommandBufferGroup->Reuse();
			return;
		}
		debugLockCmdBuffers = true;
		m_prepassCommandBufferGroup->SetOneTimeSubmit(false);
		m_lightingCommandBufferGroup->SetOneTimeSubmit(false);
	}
	else if(debugLockCmdBuffers) {
		debugLockCmdBuffers = false;
		m_prepassCommandBufferGroup->SetOneTimeSubmit(true);
		m_lightingCommandBufferGroup->SetOneTimeSubmit(true);
	}
	//

	auto prepassMode = GetPrepassMode();
	auto runPrepass = (drawSceneInfo.renderTarget == nullptr && prepassMode != PrepassMode::NoPrepass && drawSceneInfo.scene.valid());
	if(runPrepass)
		StartPrepassRecording(drawSceneInfo);
	//drawSceneInfo.scene->InvokeEventCallbacks(CSceneComponent::EVENT_POST_RENDER_PREPASS,pragma::CEDrawSceneInfo{drawSceneInfo});

	StartLightingPassRecording(drawSceneInfo);
}
void pragma::CRasterizationRendererComponent::Render(const DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired())
		return;
	if(math::is_flag_set(m_stateFlags, StateFlags::InitialRender)) {
		// HACK: For whatever reason the render code causes a crash on the very first frame when using OpenGL.
		// If we skip the first frame, it doesn't crash, so we just skip the first frame for now.
		// This is a workaround until the actual issue is found.
		// Unfortunately the OpenGL debug output doesn't give any useful information.
		math::set_flag(m_stateFlags, StateFlags::InitialRender, false);
		if(get_cengine()->GetRenderAPI() == "opengl")
			return;
	}
	auto &scene = const_cast<CSceneComponent &>(*drawSceneInfo.scene);
	get_cgame()->CallCallbacks<void, std::reference_wrapper<const DrawSceneInfo>>("OnPreRender", drawSceneInfo);
	// pragma::get_cgame()->CallLuaCallbacks<void,RasterizationRenderer*>("PrepareRendering",this);

	// scene.GetSceneRenderDesc().BuildRenderQueue(drawSceneInfo);

	// Prepass
	get_cgame()->StartGPUProfilingStage("Scene");

	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	auto prepassMode = GetPrepassMode();
	auto runPrepass = (drawSceneInfo.renderTarget == nullptr && prepassMode != PrepassMode::NoPrepass && drawSceneInfo.scene.valid());

	// We still have to update entity buffers *before* we start the render pass (since buffer updates
	// are not allowed during a render pass).
	auto &worldRenderQueues = sceneRenderDesc.GetWorldRenderQueues();
	if((drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
		std::chrono::steady_clock::time_point t;
		if(drawSceneInfo.renderStats)
			t = std::chrono::steady_clock::now();
		sceneRenderDesc.WaitForWorldRenderQueues();
		if(drawSceneInfo.renderStats)
			drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass)->SetTime(RenderPassStats::Timer::RenderThreadWait, std::chrono::steady_clock::now() - t);

		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::UpdateRenderBuffersGpu, *drawSceneInfo.commandBuffer);
		for(auto &renderQueue : worldRenderQueues)
			CSceneComponent::UpdateRenderBuffers(drawCmd, *renderQueue, drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
		if(drawSceneInfo.renderStats)
			(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::UpdateRenderBuffersGpu, *drawSceneInfo.commandBuffer);
	}

	// If we're lucky, the render queues for everything else have already been built
	// as well, so we can update them right now and do everything in one render pass.
	// Otherwise we have to split the pass into two and update the remaining render buffers
	// inbetween both passes.
	// auto &worldObjectsRenderQueue = *sceneRenderDesc.GetRenderQueue(RenderMode::World,false /* translucent */);
	// worldObjectsRenderQueue.WaitForCompletion();
#if 0
	auto worldObjectRenderQueueReady = worldObjectsRenderQueue.IsComplete();
	if(worldObjectRenderQueueReady)
	{
		if((drawSceneInfo.renderFlags &FRender::World) != FRender::None)
		{
			CSceneComponent::UpdateRenderBuffers(drawCmd,worldObjectsRenderQueue,drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(rendering::RenderStats::RenderPass::Prepass) : nullptr);
			//CSceneComponent::UpdateRenderBuffers(drawCmd,*sceneRenderDesc.GetRenderQueue(RenderMode::World,true /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->prepass : nullptr);

		}

		if((drawSceneInfo.renderFlags &FRender::View) != FRender::None)
			CSceneComponent::UpdateRenderBuffers(drawCmd,*sceneRenderDesc.GetRenderQueue(RenderMode::View,false /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(rendering::RenderStats::RenderPass::Prepass) : nullptr);
		pragma::get_cgame()->CallLuaCallbacks<void,const pragma::rendering::DrawSceneInfo*>("UpdateRenderBuffers",&drawSceneInfo);
	}
#endif

	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::UpdatePrepassRenderBuffersGpu, *drawSceneInfo.commandBuffer);
	UpdatePrepassRenderBuffers(drawSceneInfo);
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::UpdatePrepassRenderBuffersGpu, *drawSceneInfo.commandBuffer);

	std::chrono::steady_clock::time_point t;
	// Start executing the prepass; This may require a waiting period of the recording
	// of the prepass hasn't completed yet
	if(runPrepass) {
		if(drawSceneInfo.renderStats) {
			drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass)->BeginGpuTimer(RenderPassStats::Timer::GpuExecution, *drawSceneInfo.commandBuffer);
			t = std::chrono::steady_clock::now();
		}

		ExecutePrepass(drawSceneInfo);
		if(drawSceneInfo.renderStats) {
			(*drawSceneInfo.renderStats)->SetTime(RenderStats::RenderStage::PrepassExecutionCpu, std::chrono::steady_clock::now() - t);
			drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass)->EndGpuTimer(RenderPassStats::Timer::GpuExecution, *drawSceneInfo.commandBuffer);
		}
	}

	// SSAO (requires prepass)
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::PostProcessingGpuSsao, *drawSceneInfo.commandBuffer);
	RenderSSAO(drawSceneInfo);
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::PostProcessingGpuSsao, *drawSceneInfo.commandBuffer);

	// Cull light sources (requires prepass)
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::LightCullingGpu, *drawSceneInfo.commandBuffer);
	CullLightSources(drawSceneInfo);
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::LightCullingGpu, *drawSceneInfo.commandBuffer);

	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::RenderShadowsGpu, *drawSceneInfo.commandBuffer);
	RenderShadows(drawSceneInfo);
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::RenderShadowsGpu, *drawSceneInfo.commandBuffer);

	// We still need to update the render buffers for some entities
	// (All others have already been updated in the prepass)
	if(drawSceneInfo.renderStats)
		t = std::chrono::steady_clock::now();
	// TODO: This would be a good spot to start recording the shadow command buffers
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->SetTime(RenderStats::RenderStage::UpdateRenderBuffersCpu, std::chrono::steady_clock::now() - t);
	// TODO: Execute shadow command buffers here

	// Lighting pass
	if(drawSceneInfo.renderStats) {
		drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass)->BeginGpuTimer(RenderPassStats::Timer::GpuExecution, *drawSceneInfo.commandBuffer);
		t = std::chrono::steady_clock::now();
	}
	UpdateLightingPassRenderBuffers(drawSceneInfo);
	ExecuteLightingPass(drawSceneInfo);
	if(drawSceneInfo.renderStats) {
		(*drawSceneInfo.renderStats)->SetTime(RenderStats::RenderStage::LightingPassExecutionCpu, std::chrono::steady_clock::now() - t);
		drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass)->EndGpuTimer(RenderPassStats::Timer::GpuExecution, *drawSceneInfo.commandBuffer);
	}
	get_cgame()->StopGPUProfilingStage(); // Scene

	get_cgame()->CallCallbacks<void, std::reference_wrapper<const DrawSceneInfo>>("RenderPostLightingPass", drawSceneInfo);

	// Particles
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::RenderParticlesGpu, *drawSceneInfo.commandBuffer);
	RenderParticles(*drawSceneInfo.commandBuffer, drawSceneInfo, false, drawSceneInfo.commandBuffer.get());
	if(drawSceneInfo.renderStats)
		(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::RenderParticlesGpu, *drawSceneInfo.commandBuffer);

	// Post processing
	if(drawSceneInfo.renderStats) {
		(*drawSceneInfo.renderStats)->BeginGpuTimer(RenderStats::RenderStage::PostProcessingGpu, *drawSceneInfo.commandBuffer);
		t = std::chrono::steady_clock::now();
	}
	get_cgame()->StartProfilingStage("PostProcessing");
	get_cgame()->StartGPUProfilingStage("PostProcessing");

	auto *renderer = scene.GetRenderer<CRendererComponent>();
	auto &postProcessing = renderer->GetPostProcessingEffects();
	auto applyToneMapped = !math::is_flag_set(drawSceneInfo.renderFlags, RenderFlags::HDR);
	for(auto &pp : postProcessing) {
		if(pp.render.IsValid()) {
			auto flags = pp.getFlags ? pp.getFlags() : PostProcessingEffectData::Flags::None;
			if(!applyToneMapped && math::is_flag_set(flags, PostProcessingEffectData::Flags::ToneMapped))
				break;
			pp.render.Call<void, const DrawSceneInfo &>(drawSceneInfo);
		}
	}

	if(!applyToneMapped) {
		// Don't bother resolving HDR; Just apply the barrier
		drawCmd->RecordImageBarrier(GetHDRInfo().sceneRenderTarget->GetTexture().GetImage(), prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::TransferSrcOptimal);
	}

	get_cgame()->CallCallbacks<void, std::reference_wrapper<const DrawSceneInfo>>("RenderPostProcessing", drawSceneInfo);
	get_cgame()->CallLuaCallbacks<void, const DrawSceneInfo *>("RenderPostProcessing", &drawSceneInfo);

	get_cgame()->StopGPUProfilingStage(); // PostProcessing
	get_cgame()->StopProfilingStage();    // PostProcessing
	if(drawSceneInfo.renderStats) {
		(*drawSceneInfo.renderStats)->SetTime(RenderStats::RenderStage::PostProcessingExecutionCpu, std::chrono::steady_clock::now() - t);
		(*drawSceneInfo.renderStats)->EndGpuTimer(RenderStats::RenderStage::PostProcessingGpu, *drawSceneInfo.commandBuffer);
	}
}
