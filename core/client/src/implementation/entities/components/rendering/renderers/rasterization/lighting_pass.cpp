// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :debug;
import :engine;
import :game;

using namespace pragma::rendering;

#define ENABLE_PARTICLE_RENDERING 1

static auto cvDrawParticles = pragma::console::get_client_con_var("render_draw_particles");
static auto cvDrawGlow = pragma::console::get_client_con_var("render_draw_glow");
static auto cvDrawTranslucent = pragma::console::get_client_con_var("render_draw_translucent");

void pragma::CRasterizationRendererComponent::RecordPrepass(const DrawSceneInfo &drawSceneInfo)
{
	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &prepass = GetPrepass();

	auto &shaderPrepass = GetPrepassShader();
	auto *prepassStats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr;
	auto &worldRenderQueues = sceneRenderDesc.GetWorldRenderQueues();
	m_prepassCommandBufferGroup->Record([this, &drawSceneInfo, &shaderPrepass, &worldRenderQueues, &sceneRenderDesc, prepassStats](prosper::ISecondaryCommandBuffer &cmd) {
		RenderPassDrawInfo renderPassDrawInfo {drawSceneInfo, cmd};
		DepthStageRenderProcessor rsys {
		  renderPassDrawInfo,
		  {} /* drawOrigin */,
		};

		CEPrepassStageData evDataPrepassStage {rsys, shaderPrepass};
		InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_PREPASS, evDataPrepassStage);
		rsys.BindShader(shaderPrepass, math::to_integral(ShaderPrepass::Pipeline::Opaque));
		// Render static world geometry
		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
			std::chrono::steady_clock::time_point t;
			if(drawSceneInfo.renderStats)
				t = std::chrono::steady_clock::now();
			sceneRenderDesc.WaitForWorldRenderQueues();
			if(drawSceneInfo.renderStats)
				drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass)->SetTime(RenderPassStats::Timer::RenderThreadWait, std::chrono::steady_clock::now() - t);
			for(auto i = decltype(worldRenderQueues.size()) {0u}; i < worldRenderQueues.size(); ++i)
				rsys.Render(*worldRenderQueues.at(i), RenderPass::Prepass, prepassStats, i);
		}

		// Note: The non-translucent render queues also include transparent (alpha masked) objects.
		// We don't care about translucent objects here.
		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
			rsys.Render(*sceneRenderDesc.GetRenderQueue(SceneRenderPass::World, false /* translucent */), RenderPass::Prepass, prepassStats);

			auto &queueTranslucent = *sceneRenderDesc.GetRenderQueue(SceneRenderPass::World, true /* translucent */);
			queueTranslucent.WaitForCompletion(prepassStats);
			if(queueTranslucent.queue.empty() == false) {
				// rsys.BindShader(shaderPrepass,pragma::math::to_integral(pragma::ShaderPrepass::Pipeline::AlphaTest));
				rsys.Render(queueTranslucent, RenderPass::Prepass, prepassStats);
			}
		}

		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::View) != RenderFlags::None) {
			auto &queue = *sceneRenderDesc.GetRenderQueue(SceneRenderPass::View, false /* translucent */);
			queue.WaitForCompletion(prepassStats);
			if(queue.queue.empty() == false) {
				rsys.UnbindShader();
				rsys.SetCameraType(BaseRenderProcessor::CameraType::View);
				rsys.BindShader(shaderPrepass, math::to_integral(ShaderPrepass::Pipeline::Opaque));
				// rsys.BindShader(shaderPrepass,pragma::math::to_integral(pragma::ShaderPrepass::Pipeline::Opaque));
				rsys.Render(queue, RenderPass::Prepass, prepassStats);
			}
		}
		rsys.UnbindShader();

		if((renderPassDrawInfo.drawSceneInfo.renderFlags & RenderFlags::Particles) != RenderFlags::None)
			RenderParticles(cmd, drawSceneInfo, true);
	});
}
void pragma::CRasterizationRendererComponent::UpdatePrepassRenderBuffers(const DrawSceneInfo &drawSceneInfo)
{
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	// Prepass and lighting pass are now being recorded in parallel on separate threads.
	// In the meantime, we can make use of the wait time by updating the entity buffers
	// for the entity we need for the prepass.
	if((drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
		CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(SceneRenderPass::World, false /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
		CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(SceneRenderPass::World, true /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
	}

	if((drawSceneInfo.renderFlags & RenderFlags::View) != RenderFlags::None) {
		CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(SceneRenderPass::View, false /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
		CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(SceneRenderPass::View, true /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::Prepass) : nullptr);
	}

	CEUpdateRenderBuffers evData {drawSceneInfo};
	InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_UPDATE_RENDER_BUFFERS, evData);

	get_cgame()->CallLuaCallbacks<void, const DrawSceneInfo *>("UpdateRenderBuffers", &drawSceneInfo);
	//
}
void pragma::CRasterizationRendererComponent::UpdateLightingPassRenderBuffers(const DrawSceneInfo &drawSceneInfo)
{
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(SceneRenderPass::Sky, false /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass) : nullptr);
	CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(SceneRenderPass::Sky, true /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass) : nullptr);
	//CSceneComponent::UpdateRenderBuffers(drawCmd,*sceneRenderDesc.GetRenderQueue(pragma::rendering::SceneRenderPass::World,true /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->lightingPass : nullptr);
	CSceneComponent::UpdateRenderBuffers(drawCmd, *sceneRenderDesc.GetRenderQueue(SceneRenderPass::View, true /* translucent */), drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass) : nullptr);
}
void pragma::CRasterizationRendererComponent::ExecutePrepass(const DrawSceneInfo &drawSceneInfo)
{
	if(math::is_flag_set(drawSceneInfo.flags, DrawSceneInfo::Flags::DisablePrepass))
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &hCam = scene.GetActiveCamera();
	// Pre-render depths and normals (if SSAO is enabled)
	get_cgame()->StartProfilingStage("Prepass");
	get_cgame()->StartGPUProfilingStage("Prepass");
	auto &prepass = GetPrepass();
	if(prepass.textureDepth->IsMSAATexture())
		static_cast<prosper::MSAATexture &>(*prepass.textureDepth).Reset();
	if(prepass.textureNormals != nullptr && prepass.textureNormals->IsMSAATexture())
		static_cast<prosper::MSAATexture &>(*prepass.textureNormals).Reset();

	// Entity instance buffer barrier
	auto &drawCmd = drawSceneInfo.commandBuffer;
	drawCmd->RecordBufferBarrier(*CRenderComponent::GetInstanceBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Entity bone buffer barrier
	drawCmd->RecordBufferBarrier(*get_instance_bone_buffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	// Camera buffer barrier
	drawCmd->RecordBufferBarrier(*scene.GetCameraBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	// View camera buffer barrier
	drawCmd->RecordBufferBarrier(*scene.GetViewCameraBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	CEDrawSceneInfo evData {drawSceneInfo};
	InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_PRE_PREPASS, evData);

	prepass.BeginRenderPass(drawSceneInfo, nullptr, true);

	InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_PRE_EXECUTE_PREPASS, evData);
	m_prepassCommandBufferGroup->ExecuteCommands(*drawCmd);
	InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_POST_EXECUTE_PREPASS, evData);

	prepass.EndRenderPass(drawSceneInfo);

	InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_POST_PREPASS, evData);

	get_cgame()->StopGPUProfilingStage(); // Prepass
	get_cgame()->StopProfilingStage();    // Prepass
}

void pragma::CRasterizationRendererComponent::ExecuteLightingPass(const DrawSceneInfo &drawSceneInfo)
{
	if(math::is_flag_set(drawSceneInfo.flags, DrawSceneInfo::Flags::DisablePrepass))
		return;
	get_cgame()->StartProfilingStage("ExecuteLightingPass");
	auto &scene = const_cast<CSceneComponent &>(*drawSceneInfo.scene);
	auto &cam = scene.GetActiveCamera();
	bool bShadows = (drawSceneInfo.renderFlags & RenderFlags::Shadows) == RenderFlags::Shadows;
	auto &drawCmd = drawSceneInfo.commandBuffer;

	//ScopeGuard sgDepthImg {};
	m_bFrameDepthBufferSamplingRequired = false;

	// Visible light tile index buffer
	drawCmd->RecordBufferBarrier(*GetForwardPlusInstance().GetTileVisLightIndexBuffer(), prosper::PipelineStageFlags::ComputeShaderBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::ShaderWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Entity instance buffer barrier
	drawCmd->RecordBufferBarrier(*CRenderComponent::GetInstanceBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Entity bone buffer barrier
	drawCmd->RecordBufferBarrier(*get_instance_bone_buffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	auto &bufLightSources = CLightComponent::GetGlobalRenderBuffer();
	auto &bufShadowData = CLightComponent::GetGlobalShadowBuffer();
	// Light source data barrier
	drawCmd->RecordBufferBarrier(bufLightSources, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	// Shadow data barrier
	drawCmd->RecordBufferBarrier(bufShadowData, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	auto &renderSettingsBufferData = get_cgame()->GetGlobalRenderSettingsBufferData();
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
	InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_PRE_LIGHTING_PASS, evData);

	BeginRenderPass(drawSceneInfo, nullptr, true);

	InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_PRE_EXECUTE_LIGHTING_PASS, evData);
	m_lightingCommandBufferGroup->ExecuteCommands(*drawCmd);
	InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_POST_EXECUTE_LIGHTING_PASS, evData);

	EndRenderPass(drawSceneInfo);

	InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_POST_LIGHTING_PASS, evData);

	get_cgame()->StopProfilingStage(); // ExecuteLightingPass
}
template<typename TCPPM>
TCPPM *pragma::CRasterizationRendererComponent::GetRendererComponent()
{
	return static_cast<TCPPM *>(m_rendererComponent);
}
template pragma::CRendererComponent *pragma::CRasterizationRendererComponent::GetRendererComponent();

template<typename TCPPM>
const TCPPM *pragma::CRasterizationRendererComponent::GetRendererComponent() const
{
	return const_cast<CRasterizationRendererComponent *>(this)->GetRendererComponent<CRendererComponent>();
}
template const pragma::CRendererComponent *pragma::CRasterizationRendererComponent::GetRendererComponent() const;

void pragma::CRasterizationRendererComponent::StartPrepassRecording(const DrawSceneInfo &drawSceneInfo)
{
	auto &prepass = GetPrepass();
	auto &prepassRt = *prepass.renderTarget;
	m_prepassCommandBufferGroup->StartRecording(prepassRt.GetRenderPass(), prepassRt.GetFramebuffer());

	if(!math::is_flag_set(drawSceneInfo.flags, DrawSceneInfo::Flags::DisablePrepass)) {
		RecordPrepass(drawSceneInfo);
		CEDrawSceneInfo evData {drawSceneInfo};
		InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS, evData);
	}

	m_prepassCommandBufferGroup->EndRecording();
}
void pragma::CRasterizationRendererComponent::StartLightingPassRecording(const DrawSceneInfo &drawSceneInfo)
{
	auto *rt = GetLightingPassRenderTarget(drawSceneInfo);
	if(rt == nullptr || drawSceneInfo.scene.expired())
		return;
	m_lightingCommandBufferGroup->StartRecording(rt->GetRenderPass(), rt->GetFramebuffer());

	if(!math::is_flag_set(drawSceneInfo.flags, DrawSceneInfo::Flags::DisableLightingPass)) {
		RecordLightingPass(drawSceneInfo);
		CEDrawSceneInfo evData {drawSceneInfo};
		InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_ON_RECORD_LIGHTING_PASS, evData);
	}

	m_lightingCommandBufferGroup->EndRecording();
}

void pragma::CRasterizationRendererComponent::RecordLightingPass(const DrawSceneInfo &drawSceneInfo)
{
	auto &scene = const_cast<CSceneComponent &>(*drawSceneInfo.scene);
	auto &cam = scene.GetActiveCamera();
	m_lightingCommandBufferGroup->Record([this, &scene, &cam, &drawSceneInfo](prosper::ISecondaryCommandBuffer &cmd) mutable {
		get_cgame()->StartProfilingStage("RecordLightingPass");
		auto pcmd = cmd.shared_from_this();
		auto bGlow = cvDrawGlow->GetBool();
		auto bTranslucent = cvDrawTranslucent->GetBool();

		auto *lightingStageStats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPass) : nullptr;
		auto *lightingStageTranslucentStats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->GetPassStats(RenderStats::RenderPass::LightingPassTranslucent) : nullptr;
		RenderPassDrawInfo rpDrawInfo {drawSceneInfo, cmd};
		LightingStageRenderProcessor rsys {rpDrawInfo, {} /* drawOrigin */};
		auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();

		CELightingStageData evDataLightingStage {rsys};
		if((drawSceneInfo.renderFlags & RenderFlags::Skybox) != RenderFlags::None) {
			get_cgame()->StartProfilingStage("Skybox");
			get_cgame()->StartGPUProfilingStage("Skybox");
			InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_SKYBOX, evDataLightingStage);

			rsys.Render(*sceneRenderDesc.GetRenderQueue(SceneRenderPass::Sky, false /* translucent */), lightingStageStats);
			rsys.Render(*sceneRenderDesc.GetRenderQueue(SceneRenderPass::Sky, true /* translucent */), lightingStageTranslucentStats);

			InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_MT_END_RECORD_SKYBOX, evDataLightingStage);
			get_cgame()->StopGPUProfilingStage(); // Skybox
			get_cgame()->StopProfilingStage();    // Skybox
		}

		// Render static world geometry
#if DEBUG_RENDER_PERFORMANCE_TEST_ENABLED == 1
		if(g_dbgMode == 3 || g_dbgMode == 5) {
#endif
			if((drawSceneInfo.renderFlags & RenderFlags::World) != RenderFlags::None) {
				get_cgame()->StartProfilingStage("World");
				get_cgame()->StartGPUProfilingStage("World");
				InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_WORLD, evDataLightingStage);

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
				rsys.Render(*sceneRenderDesc.GetRenderQueue(SceneRenderPass::World, false /* translucent */), lightingStageStats);
				rsys.Render(*sceneRenderDesc.GetRenderQueue(SceneRenderPass::World, true /* translucent */), lightingStageTranslucentStats);

				InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_MT_END_RECORD_WORLD, evDataLightingStage);
				get_cgame()->StopGPUProfilingStage(); // World
				get_cgame()->StopProfilingStage();    // World
			}
#if DEBUG_RENDER_PERFORMANCE_TEST_ENABLED == 1
		}
#endif
#if 0
		if(bShouldDrawParticles)
		{
			//pragma::get_cgame()->StartProfilingStage(pragma::CGame::GPUProfilingPhase::Particles);
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
			// pragma::get_cgame()->StopProfilingStage(pragma::CGame::GPUProfilingPhase::Particles);
		}
#endif
		//pragma::get_cengine()->StopGPUTimer(GPUTimerEvent::Particles); // prosper TODO

		// pragma::get_cgame()->CallCallbacks<void,std::reference_wrapper<const pragma::rendering::DrawSceneInfo>>("Render",std::ref(drawSceneInfo));
		// pragma::get_cgame()->CallLuaCallbacks<void,const pragma::rendering::DrawSceneInfo*>("Render",&drawSceneInfo);

		if((drawSceneInfo.renderFlags & RenderFlags::Debug) == RenderFlags::Debug) {
			get_cgame()->StartProfilingStage("Debug");
			get_cgame()->StartGPUProfilingStage("Debug");
			InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_DEBUG, evDataLightingStage);

			if(cam.valid())
				debug::DebugRenderer::Render(pcmd, *cam);
			get_cgame()->RenderDebugPhysics(pcmd, *cam);

#if DEBUG_RENDER_PERFORMANCE_TEST_ENABLED == 1
			pragma::ecs::EntityIterator entIt {*pragma::get_cgame()};
			entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CPropDynamicComponent>>();
			auto it = entIt.begin();
			static std::shared_ptr<prosper::IBuffer> dbgBuffer = nullptr;
			static std::shared_ptr<prosper::IBuffer> instBuffer = nullptr;
			static uint32_t vertCount = 0;
			static std::shared_ptr<pragma::geometry::ModelSubMesh> mesh = nullptr;
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
					dbgBuffer = pragma::get_cengine()->GetRenderContext().CreateBuffer(createInfo, verts.data());
					vertCount = verts.size();
					mesh = subMesh;

					std::vector<uint16_t> indices;
					indices.resize(verts.size());
					for(auto i = decltype(indices.size()) {0u}; i < indices.size(); ++i)
						indices[i] = i;
					createInfo.size = indices.size() * sizeof(indices.front());
					createInfo.usageFlags = prosper::BufferUsageFlags::IndexBufferBit;
					createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
					instBuffer = pragma::get_cengine()->GetRenderContext().CreateBuffer(createInfo, indices.data());
				}
			}

			if(dbgBuffer) {
				auto shader = pragma::get_cengine()->GetShader("test");
				if(shader.valid()) {
					if(g_dbgMode == 0) {
						auto *test = static_cast<pragma::ShaderTest *>(shader.get());
						if(test->BeginDraw(pcmd, {})) // && test->BindEntity(static_cast<pragma::ecs::CBaseEntity&>(*ent)))
						{
							test->BindSceneCamera(scene, static_cast<pragma::CRasterizationRendererComponent &>(*scene.GetRenderer<pragma::CRendererComponent>()), false);
							auto instanceBuffer = CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer();
							//test->Draw(static_cast<pragma::geometry::CModelSubMesh&>(*mesh),0u,*instanceBuffer);
							test->DrawTest(*dbgBuffer, *instBuffer, vertCount);
							//test->DrawTest(*dbgBuffer,vertCount);
							test->EndDraw();
						}
					}
					else if(g_dbgMode == 1) {
						auto &whDebugShader = pragma::get_cgame()->GetGameShader(pragma::CGame::GameShader::Debug);
						auto *shader = static_cast<pragma::ShaderDebug *>(whDebugShader.get());

						auto m = umat::identity();
						const Vector2 scale {2.f, 2.f};
						const Vector3 matScale {-scale.x, -scale.y, -1.f};
						m = glm::gtc::scale(m, matScale);
						m = cam->GetProjectionMatrix() * cam->GetViewMatrix() * m;

						pragma::ShaderDebug::PushConstants pushConstants {m, Vector4 {1.f, 1.f, 1.f, 1.f}};
						shader->BeginDraw(pcmd);
						shader->Draw(*dbgBuffer, vertCount, m);
						shader->EndDraw();
					}
					else if(g_dbgMode == 2) {
						/*auto shader = pragma::get_cengine()->GetShader("pbr");
				auto *test = static_cast<pragma::ShaderPBR*>(shader.get());
				if(test->BeginDraw(drawCmd,{}) && test->BindEntity(static_cast<pragma::ecs::CBaseEntity&>(*ent)))
				{
					test->BindScene();
					test->BindEntity();
					test->BindLights();
					test->BindMaterial();
					test->BindSceneCamera(scene,static_cast<pragma::CRasterizationRendererComponent&>(*scene.GetRenderer<pragma::CRendererComponent>()),false);
					auto instanceBuffer = CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer();
					test->Draw(static_cast<pragma::geometry::CModelSubMesh&>(*subMesh),0u,*instanceBuffer);
					//test->DrawTest(*dbgBuffer,vertCount);
					test->EndDraw();
				}*/
					}
				}
			}
#endif

			InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_MT_END_RECORD_DEBUG, evDataLightingStage);
			get_cgame()->StopGPUProfilingStage(); // Debug
			get_cgame()->StopProfilingStage();    // Debug
		}

		if((drawSceneInfo.renderFlags & RenderFlags::View) != RenderFlags::None) {
			get_cgame()->StartGPUProfilingStage("View");
			get_cgame()->StartProfilingStage("View");
			InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_VIEW, evDataLightingStage);

			rsys.SetCameraType(BaseRenderProcessor::CameraType::View);
			rsys.Render(*sceneRenderDesc.GetRenderQueue(SceneRenderPass::View, false /* translucent */), lightingStageStats);
			rsys.Render(*sceneRenderDesc.GetRenderQueue(SceneRenderPass::View, true /* translucent */), lightingStageTranslucentStats);

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

			InvokeEventCallbacks(cRasterizationRendererComponent::EVENT_MT_END_RECORD_VIEW, evDataLightingStage);
			get_cgame()->StopGPUProfilingStage(); // View
			get_cgame()->StopProfilingStage();    // View
		}
		get_cgame()->StopProfilingStage(); // RecordLightingPass
	});
}
