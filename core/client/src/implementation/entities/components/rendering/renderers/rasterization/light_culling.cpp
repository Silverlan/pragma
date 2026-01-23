// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.rasterization_renderer;
import :client_state;
import :game;

using namespace pragma::rendering;

void pragma::CRasterizationRendererComponent::CullLightSources(const DrawSceneInfo &drawSceneInfo)
{
	auto &shaderSettings = get_client_state()->GetGameWorldShaderSettings();
	if(drawSceneInfo.scene.expired() || shaderSettings.dynamicLightingEnabled == false)
		return;
	auto &scene = *drawSceneInfo.scene;
	auto &prepass = GetPrepass();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	{
		get_cgame()->StartProfilingStage("CullLightSources");
		get_cgame()->StartGPUProfilingStage("CullLightSources");
		auto depthTex = prepass.textureDepth;
		auto bMultisampled = depthTex->IsMSAATexture();
		if(depthTex->IsMSAATexture()) {
			depthTex = static_cast<prosper::MSAATexture &>(*depthTex).Resolve(*drawCmd, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);
		}
		else
			drawCmd->RecordImageBarrier(depthTex->GetImage(), prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

		drawCmd->RecordImageBarrier(depthTex->GetImage(), {prosper::PipelineStageFlags::LateFragmentTestsBit, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit},
		  {prosper::PipelineStageFlags::ComputeShaderBit, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::AccessFlags::ShaderReadBit});

		m_visLightSources.clear();
		m_visShadowedLights.clear();

		auto &fp = GetForwardPlusInstance();

		// Camera buffer
		drawCmd->RecordBufferBarrier(*scene.GetCameraBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

		// Render settings buffer
		drawCmd->RecordBufferBarrier(*scene.GetRenderSettingsBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

		auto *worldEnv = scene.GetWorldEnvironment();
		if(worldEnv && worldEnv->IsUnlit() == false) {
			std::chrono::steady_clock::time_point t;
			if(drawSceneInfo.renderStats)
				t = std::chrono::steady_clock::now();

			fp.Compute(*drawCmd, const_cast<CSceneComponent &>(scene), depthTex->GetImage(), *scene.GetCameraDescriptorSetCompute());
			auto &lightBits = fp.GetShadowLightBits();
			for(auto i = decltype(lightBits.size()) {0}; i < lightBits.size(); ++i) {
				auto &intVal = lightBits.at(i);
				const auto numBits = 32u;
				for(auto j = 0u; j < numBits; ++j) {
					if(!(intVal & (1 << j))) // If bit is set, this light is visible on screen
						continue;
					auto shadowIdx = i * numBits + j;
					auto *l = CLightComponent::GetLightByShadowBufferIndex(shadowIdx);
					if(l == nullptr || static_cast<ecs::CBaseEntity &>(l->GetEntity()).IsInScene(scene) == false)
						continue;
					m_visLightSources.push_back(l);

					auto &renderBuffer = l->GetRenderBuffer();
					if(renderBuffer) {
						drawCmd->RecordBufferBarrier(*renderBuffer, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
					}
					auto &shadowBuffer = l->GetShadowBuffer();
					if(shadowBuffer) {
						drawCmd->RecordBufferBarrier(*shadowBuffer, prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

						// Determine light sources that should actually cast shadows
						if(l->ShouldCastShadows()) {
							auto *shadowC = l->GetShadowComponent<CShadowComponent>();
							auto hSm = l->GetShadowMap<CShadowComponent>(ShadowMapType::Dynamic);
							if(hSm.valid() && hSm->HasRenderTarget()) {
								// Request render target for light sources that already had one before.
								// This will make sure the shadow map is the same as before, which increases the likelihood
								// we don't actually have to re-render the shadows for this light.
								hSm->RequestRenderTarget();
							}
							if(shadowC->GetRenderer().GetRenderState() != LightShadowRenderer::RenderState::NoRenderRequired)
								m_visShadowedLights.push_back(l->GetHandle<CLightComponent>());
						}
					}
				}
			}

			if(drawSceneInfo.renderStats)
				(*drawSceneInfo.renderStats)->SetTime(RenderStats::RenderStage::LightCullingCpu, std::chrono::steady_clock::now() - t);
		}
	}
}
void pragma::CRasterizationRendererComponent::RenderShadows(const DrawSceneInfo &drawSceneInfo)
{
	auto &shaderSettings = get_client_state()->GetGameWorldShaderSettings();
	if(drawSceneInfo.scene.expired() || shaderSettings.dynamicLightingEnabled == false)
		return;
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto &scene = *drawSceneInfo.scene;
	auto &prepass = GetPrepass();
	auto depthTex = prepass.textureDepth;
	auto bMultisampled = depthTex->IsMSAATexture();
	// Don't write to depth image until compute shader has completed reading from it
	if(!bMultisampled)
		drawCmd->RecordImageBarrier(depthTex->GetImage(), prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::DepthStencilAttachmentOptimal);

	drawCmd->RecordImageBarrier(depthTex->GetImage(), {prosper::PipelineStageFlags::ComputeShaderBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::ShaderReadBit},
	  {prosper::PipelineStageFlags::EarlyFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit});

	get_cgame()->StopGPUProfilingStage(); // CullLightSources
	get_cgame()->StopProfilingStage();    // CullLightSources

	get_cgame()->StartProfilingStage("Shadows");
	get_cgame()->StartGPUProfilingStage("Shadows");
	// Update shadows
	//pragma::get_cengine()->StartGPUTimer(GPUTimerEvent::Shadow); // TODO: Only for main scene // prosper TODO

	// Entity instance buffer barrier
	drawCmd->RecordBufferBarrier(*CRenderComponent::GetInstanceBuffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Entity bone buffer barrier
	drawCmd->RecordBufferBarrier(*get_instance_bone_buffer(), prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::TransferWriteBit,
	  prosper::AccessFlags::ShaderReadBit);

	auto *worldEnv = scene.GetWorldEnvironment();
	if(worldEnv && worldEnv->IsUnlit() == false && shaderSettings.dynamicShadowsEnabled) {
		std::queue<uint32_t> lightSourcesReadyForShadowRendering;
		std::queue<uint32_t> lightSourcesWaitingForRenderQueues;
		for(auto i = decltype(m_visShadowedLights.size()) {0u}; i < m_visShadowedLights.size(); ++i) {
			auto *l = m_visShadowedLights[i].get();
			auto hSm = l->GetShadowMap<CShadowComponent>(ShadowMapType::Dynamic);
			if(hSm.valid() && hSm->HasRenderTarget() == false)
				hSm->RequestRenderTarget();

			if(hSm.expired() || hSm->HasRenderTarget() == false)
				continue; // No render target available for this light; No shadows will be rendered

			// Note: At this point the engine has already initiated render queue generation
			// for shadowed light sources that were visible in the previous frame.
			// If these light sources are still visible this frame, their render queues are likely
			// to be complete already, in which case we can start rendering shadows immediately.
			// In the meantime, any shadowed lights that don't have a render queue ready yet
			// will start generating one now, which should be complete by the time the other
			// light sources have completed rendering their shadow maps.
			auto *shadowC = l->GetShadowComponent<CShadowComponent>();
			auto &renderer = shadowC->GetRenderer();
			if(renderer.IsRenderQueueComplete())
				lightSourcesReadyForShadowRendering.push(i);
			else {
				// Render queue creation has either not been initiated yet, or is still processing
				lightSourcesWaitingForRenderQueues.push(i);
				if(renderer.DoesRenderQueueRequireBuilding())
					renderer.BuildRenderQueues(drawSceneInfo);
			}
		}
		get_cgame()->GetRenderQueueBuilder().SetReadyForCompletion();

		while(lightSourcesReadyForShadowRendering.empty() == false) {
			auto idx = lightSourcesReadyForShadowRendering.front();
			lightSourcesReadyForShadowRendering.pop();

			// TODO: Compare render queue hash to determine if we need to re-render
			// Note: Always have to re-render if render target has changed!!
			// Also do the same below
			auto &lightC = m_visShadowedLights.at(idx);
			auto &renderer = lightC->GetShadowComponent<CShadowComponent>()->GetRenderer();
			renderer.Render(drawSceneInfo);
		}

		while(lightSourcesWaitingForRenderQueues.empty() == false) {
			auto idx = lightSourcesWaitingForRenderQueues.front();
			lightSourcesWaitingForRenderQueues.pop();

			// Render remaining light sources. If their render queues are still not
			// completed, we'll have no choice but to wait.
			auto &lightC = m_visShadowedLights.at(idx);
			auto &renderer = lightC->GetShadowComponent<CShadowComponent>()->GetRenderer();
			renderer.Render(drawSceneInfo);
		}

		const_cast<CSceneComponent &>(*drawSceneInfo.scene).SwapPreviouslyVisibleLights(std::move(m_visShadowedLights));
	}
	//pragma::get_cengine()->StopGPUTimer(GPUTimerEvent::Shadow); // prosper TODO
	//drawCmd->SetViewport(w,h); // Reset the viewport

	//auto &imgDepth = textureDepth->GetImage(); // prosper TODO
	//imgDepth->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal); // prosper TODO
	get_cgame()->StopGPUProfilingStage(); // Shadows
	get_cgame()->StopProfilingStage();    // Shadows
}
