/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/renderers/rasterization/glow_data.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include <pragma/lua/luafunction_call.h>
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;


static void cl_render_ssao_callback(NetworkState*,ConVar*,bool,bool val)
{
	if(c_game == nullptr)
		return;
	auto *scene = c_game->GetScene();
	if(scene == nullptr)
		return;
	auto *renderer = dynamic_cast<RasterizationRenderer*>(scene->GetRenderer());
	if(renderer == nullptr)
		return;
	renderer->SetSSAOEnabled(*scene,val);
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_ssao,cl_render_ssao_callback);

static std::vector<RasterizationRenderer*> g_renderers {};
static std::weak_ptr<prosper::Texture> g_lightmap = {};
void RasterizationRenderer::UpdateLightmap(const std::shared_ptr<prosper::Texture> &lightMapTexture)
{
	for(auto *renderer : g_renderers)
		renderer->SetLightMap(lightMapTexture);
	g_lightmap = lightMapTexture;
}

RasterizationRenderer::RasterizationRenderer()
	: BaseRenderer{},m_hdrInfo{*this}
{
	g_renderers.push_back(this);
	m_whShaderWireframe = c_engine->GetShader("wireframe");

	InitializeLightDescriptorSets();

	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.size = sizeof(m_rendererData);
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	m_rendererBuffer = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo,&m_rendererData);

	auto *shaderPbr = static_cast<pragma::ShaderPBR*>(c_engine->GetShader("pbr").get());
	assert(shaderPbr);
	if(shaderPbr)
	{
		m_descSetGroupRenderer = shaderPbr->CreateDescriptorSetGroup(pragma::ShaderPBR::DESCRIPTOR_SET_RENDERER.setIndex);
		m_descSetGroupRenderer->GetDescriptorSet()->SetBindingUniformBuffer(*m_rendererBuffer,umath::to_integral(pragma::ShaderScene::RendererBinding::Renderer));
	}
}

RasterizationRenderer::~RasterizationRenderer()
{
	auto it = std::find(g_renderers.begin(),g_renderers.end(),this);
	if(it != g_renderers.end())
		g_renderers.erase(it);
}

void RasterizationRenderer::UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	drawCmd->RecordUpdateBuffer(*m_rendererBuffer,0ull,m_rendererData);
}

bool RasterizationRenderer::Initialize(uint32_t w,uint32_t h) {return true;}

prosper::IDescriptorSet *RasterizationRenderer::GetDepthDescriptorSet() const {return (m_hdrInfo.dsgSceneDepth != nullptr) ? m_hdrInfo.dsgSceneDepth->GetDescriptorSet() : nullptr;}
prosper::IDescriptorSet *RasterizationRenderer::GetRendererDescriptorSet() const {return m_descSetGroupRenderer->GetDescriptorSet();}

prosper::IDescriptorSet *RasterizationRenderer::GetLightSourceDescriptorSet() const {return m_dsgLights->GetDescriptorSet();}
prosper::IDescriptorSet *RasterizationRenderer::GetLightSourceDescriptorSetCompute() const {return m_dsgLightsCompute->GetDescriptorSet();}

void RasterizationRenderer::InitializeLightDescriptorSets()
{
	if(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_LIGHTS.IsValid())
	{
		auto &bufLightSources = pragma::CLightComponent::GetGlobalRenderBuffer();
		auto &bufShadowData = pragma::CLightComponent::GetGlobalShadowBuffer();
		m_dsgLights = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_LIGHTS);
		m_dsgLights->GetDescriptorSet()->SetBindingStorageBuffer(
			const_cast<prosper::IUniformResizableBuffer&>(bufLightSources),umath::to_integral(pragma::ShaderTextured3DBase::LightBinding::LightBuffers)
		);
		m_dsgLights->GetDescriptorSet()->SetBindingStorageBuffer(
			const_cast<prosper::IUniformResizableBuffer&>(bufShadowData),umath::to_integral(pragma::ShaderTextured3DBase::LightBinding::ShadowData)
		);

		m_dsgLightsCompute = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS);
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingStorageBuffer(
			const_cast<prosper::IUniformResizableBuffer&>(bufLightSources),umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::LightBuffers)
		);
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingStorageBuffer(
			const_cast<prosper::IUniformResizableBuffer&>(bufShadowData),umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::ShadowData)
		);
	}
}

void RasterizationRenderer::UpdateCSMDescriptorSet(pragma::CLightDirectionalComponent &lightSource)
{
	auto *dsLights = GetLightSourceDescriptorSet();
	if(dsLights == nullptr)
		return;
	auto *pShadowMap = lightSource.GetShadowMap();
	auto texture = pShadowMap ? pShadowMap->GetDepthTexture() : nullptr;
	if(texture == nullptr)
		return;
	auto numLayers = pShadowMap->GetLayerCount();
	for(auto i=decltype(numLayers){0};i<numLayers;++i)
	{
		dsLights->SetBindingArrayTexture(
			*texture,umath::to_integral(pragma::ShaderSceneLit::LightBinding::CSM),i,i
		);
	}
}

void RasterizationRenderer::SetFogOverride(const std::shared_ptr<prosper::IDescriptorSetGroup> &descSetGroup) {m_descSetGroupFogOverride = descSetGroup;}

void RasterizationRenderer::RenderGameScene(const util::DrawSceneInfo &drawSceneInfo)
{
	if(drawSceneInfo.scene.expired())
		return;
	static auto skipMode = 0;
	if(skipMode == 1)
		return;
	auto &scene = const_cast<pragma::CSceneComponent&>(*drawSceneInfo.scene);
	
	if(skipMode == 2)
		return;

	c_game->CallCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("OnPreRender",drawSceneInfo);
	c_game->CallLuaCallbacks<void,RasterizationRenderer*>("PrepareRendering",this);

	// scene.GetSceneRenderDesc().BuildRenderQueue(drawSceneInfo);
	
	if(skipMode == 4)
		return;
	// Prepass
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::Scene);
	
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto fUpdateRenderBuffers = [&drawSceneInfo,&drawCmd](const RenderQueue &renderQueue,RenderPassStats *stats=nullptr) {
		renderQueue.WaitForCompletion();
		CSceneComponent::GetEntityInstanceIndexBuffer()->UpdateBufferData(renderQueue);
		auto curEntity = std::numeric_limits<EntityIndex>::max();
		for(auto &item : renderQueue.queue)
		{
			if(item.entity == curEntity)
				continue;
			curEntity = item.entity;
			auto &ent = static_cast<CBaseEntity&>(*c_game->GetEntityByLocalIndex(item.entity));
			auto &renderC = *ent.GetRenderComponent();
			if(stats && umath::is_flag_set(renderC.GetStateFlags(),CRenderComponent::StateFlags::RenderBufferDirty))
				++stats->numEntityBufferUpdates;
			renderC.UpdateRenderBuffers(drawCmd);
		}
	};

	auto &sceneRenderDesc = drawSceneInfo.scene->GetSceneRenderDesc();
	if(drawSceneInfo.renderTarget == nullptr)
	{
		auto prepassMode = GetPrepassMode();
		if(prepassMode == PrepassMode::NoPrepass || drawSceneInfo.scene.expired())
			return;
		auto &scene = *drawSceneInfo.scene;
		auto &hCam = scene.GetActiveCamera();
		// Pre-render depths and normals (if SSAO is enabled)
		c_game->StartProfilingStage(CGame::CPUProfilingPhase::Prepass);
		c_game->StartProfilingStage(CGame::GPUProfilingPhase::Prepass);
		auto &prepass = GetPrepass();
		if(prepass.textureDepth->IsMSAATexture())
			static_cast<prosper::MSAATexture&>(*prepass.textureDepth).Reset();
		if(prepass.textureNormals != nullptr && prepass.textureNormals->IsMSAATexture())
			static_cast<prosper::MSAATexture&>(*prepass.textureNormals).Reset();

		// Entity instance buffer barrier
		auto &drawCmd = drawSceneInfo.commandBuffer;
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

		// Camera buffer barrier
		drawCmd->RecordBufferBarrier(
			*scene.GetCameraBuffer(),
			prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit,
			prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
		);

		// View camera buffer barrier
		drawCmd->RecordBufferBarrier(
			*scene.GetViewCameraBuffer(),
			prosper::PipelineStageFlags::TransferBit,prosper::PipelineStageFlags::FragmentShaderBit,
			prosper::AccessFlags::TransferWriteBit,prosper::AccessFlags::ShaderReadBit
		);

		// We still have to update entity buffers *before* we start the render pass (since buffer updates
		// are not allowed during a render pass).
		auto &worldRenderQueues = sceneRenderDesc.GetWorldRenderQueues();
		if((drawSceneInfo.renderFlags &FRender::World) != FRender::None)
		{
			std::chrono::steady_clock::time_point t;
			if(drawSceneInfo.renderStats)
				t = std::chrono::steady_clock::now();
			sceneRenderDesc.WaitForWorldRenderQueues();
			if(drawSceneInfo.renderStats)
				drawSceneInfo.renderStats->prepass.renderThreadWaitTime += std::chrono::steady_clock::now() -t;

			for(auto &renderQueue : worldRenderQueues)
				fUpdateRenderBuffers(*renderQueue,drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->prepass : nullptr);
		}

		// If we're lucky, the render queues for everything else have already been built
		// as well, so we can update them right now and do everything in one render pass.
		// Otherwise we have to split the pass into two and update the remaining render buffers
		// inbetween both passes.
		auto &worldObjectsRenderQueue = *sceneRenderDesc.GetRenderQueue(RenderMode::World,false /* translucent */);
		auto worldObjectRenderQueueReady = worldObjectsRenderQueue.IsComplete();
		if(worldObjectRenderQueueReady)
		{
			if((drawSceneInfo.renderFlags &FRender::World) != FRender::None)
				fUpdateRenderBuffers(worldObjectsRenderQueue,drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->prepass : nullptr);

			if((drawSceneInfo.renderFlags &FRender::View) != FRender::None)
				fUpdateRenderBuffers(*sceneRenderDesc.GetRenderQueue(RenderMode::View,false /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->prepass : nullptr);
		}

		prepass.BeginRenderPass(drawSceneInfo);

		auto &shaderPrepass = GetPrepassShader();
		auto *prepassStats = drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->prepass : nullptr;
		pragma::rendering::DepthStageRenderProcessor rsys {drawSceneInfo,RenderFlags::None,{} /* drawOrigin */,};
		if(rsys.BindShader(shaderPrepass))
		{
			// Render static world geometry
			if((drawSceneInfo.renderFlags &FRender::World) != FRender::None)
			{
				for(auto i=decltype(worldRenderQueues.size()){0u};i<worldRenderQueues.size();++i)
					rsys.Render(*worldRenderQueues.at(i),prepassStats,i);
			}

			if(worldObjectRenderQueueReady == false)
			{
				// We have to suspend the render pass temporarily to update the remaining render buffers
				rsys.UnbindShader();
				prepass.EndRenderPass(drawSceneInfo);

				if((drawSceneInfo.renderFlags &FRender::World) != FRender::None)
					fUpdateRenderBuffers(worldObjectsRenderQueue,drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->prepass : nullptr);

				if((drawSceneInfo.renderFlags &FRender::View) != FRender::None)
					fUpdateRenderBuffers(*sceneRenderDesc.GetRenderQueue(RenderMode::View,false /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->prepass : nullptr);

				prepass.BeginRenderPass(drawSceneInfo,prepass.subsequentRenderPass.get());
				rsys.BindShader(shaderPrepass);
			}

			// Note: The non-translucent render queues also include transparent (alpha masked) objects.
			// We don't care about translucent objects here.
			if((drawSceneInfo.renderFlags &FRender::World) != FRender::None)
				rsys.Render(*sceneRenderDesc.GetRenderQueue(RenderMode::World,false /* translucent */),prepassStats);

			if((drawSceneInfo.renderFlags &FRender::View) != FRender::None)
			{
				rsys.SetCameraType(pragma::rendering::BaseRenderProcessor::CameraType::View);
				rsys.Render(*sceneRenderDesc.GetRenderQueue(RenderMode::View,false /* translucent */),prepassStats);
			}

			c_game->CallCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>,std::reference_wrapper<pragma::rendering::DepthStageRenderProcessor>>("RenderPrepass",drawSceneInfo,rsys);
			c_game->CallLuaCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("RenderPrepass",drawSceneInfo);
		}
		rsys.UnbindShader();

		prepass.EndRenderPass(drawSceneInfo);



		c_game->StopProfilingStage(CGame::GPUProfilingPhase::Prepass);
		c_game->StopProfilingStage(CGame::CPUProfilingPhase::Prepass);

		drawSceneInfo.scene->InvokeEventCallbacks(CSceneComponent::EVENT_POST_RENDER_PREPASS,pragma::CEDrawSceneInfo{drawSceneInfo});
	}
	
	if(skipMode == 5)
		return;
	// SSAO
	RenderSSAO(drawSceneInfo);
	
	if(skipMode == 6)
		return;
	// Cull light sources
	CullLightSources(drawSceneInfo);


	
	if(skipMode == 7)
		return;
	
	// We still need to update the render buffers for some entities
	// (All others have already been updated in the prepass)
	fUpdateRenderBuffers(*sceneRenderDesc.GetRenderQueue(RenderMode::Skybox,false /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->lightingPass : nullptr);
	fUpdateRenderBuffers(*sceneRenderDesc.GetRenderQueue(RenderMode::Skybox,true /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->lightingPass : nullptr);
	fUpdateRenderBuffers(*sceneRenderDesc.GetRenderQueue(RenderMode::World,true /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->lightingPass : nullptr);
	fUpdateRenderBuffers(*sceneRenderDesc.GetRenderQueue(RenderMode::View,true /* translucent */),drawSceneInfo.renderStats ? &drawSceneInfo.renderStats->lightingPass : nullptr);

	// Lighting pass
	RenderLightingPass(drawSceneInfo);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::Scene);


	
	if(skipMode == 8)
		return;
	// Post processing
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::PostProcessing);
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessing);

	// Fog
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingFog);
	RenderSceneFog(drawSceneInfo);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingFog);
	
	if(skipMode == 9)
		return;
	// Glow
	// RenderGlowObjects(drawSceneInfo);
	c_game->CallCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("RenderPostProcessing",drawSceneInfo);
	c_game->CallLuaCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("RenderPostProcessing",drawSceneInfo);

	// Bloom
	// RenderBloom(drawSceneInfo);
	
	if(skipMode == 10)
		return;
	// Tone mapping
	if(umath::is_flag_set(drawSceneInfo.renderFlags,FRender::HDR))
	{
		// Don't bother resolving HDR; Just apply the barrier
		drawCmd->RecordImageBarrier(
			GetHDRInfo().sceneRenderTarget->GetTexture().GetImage(),
			prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal
		);
		return;
	}
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingHDR);
	auto &dsgBloomTonemapping = GetHDRInfo().dsgBloomTonemapping;
	RenderToneMapping(drawSceneInfo,*dsgBloomTonemapping->GetDescriptorSet());
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingHDR);
	
	if(skipMode == 11)
		return;
	// FXAA
	RenderFXAA(drawSceneInfo);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessing);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::PostProcessing);
}

bool RasterizationRenderer::ReloadRenderTarget(pragma::CSceneComponent &scene,uint32_t width,uint32_t height)
{
	auto bSsao = IsSSAOEnabled();
	if(
		m_hdrInfo.Initialize(scene,*this,width,height,m_sampleCount,bSsao) == false || 
		m_glowInfo.Initialize(width,height,m_hdrInfo) == false ||
		m_hdrInfo.InitializeDescriptorSets() == false
		)
		return false;

	auto &descSetHdrResolve = *m_hdrInfo.dsgBloomTonemapping->GetDescriptorSet();
	auto *resolvedGlowTex = &GetGlowInfo().renderTarget->GetTexture();
	if(resolvedGlowTex->IsMSAATexture())
		resolvedGlowTex = static_cast<prosper::MSAATexture*>(resolvedGlowTex)->GetResolvedTexture().get();
	descSetHdrResolve.SetBindingTexture(*resolvedGlowTex,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));

	if(bSsao == true)
	{
		auto &ssaoInfo = GetSSAOInfo();
		auto *ssaoBlurTexResolved = &ssaoInfo.renderTargetBlur->GetTexture();
		if(ssaoBlurTexResolved->IsMSAATexture())
			ssaoBlurTexResolved = static_cast<prosper::MSAATexture*>(ssaoBlurTexResolved)->GetResolvedTexture().get();

		auto &ds = *m_descSetGroupRenderer->GetDescriptorSet();
		ds.SetBindingTexture(*ssaoBlurTexResolved,umath::to_integral(pragma::ShaderScene::RendererBinding::SSAOMap));
	}
	auto &dummyTex = c_engine->GetRenderContext().GetDummyTexture();
	auto &ds = *m_descSetGroupRenderer->GetDescriptorSet();
	ds.SetBindingTexture(*dummyTex,umath::to_integral(pragma::ShaderScene::RendererBinding::LightMap));

	m_lightMapInfo.lightMapTexture = nullptr;
	if(g_lightmap.expired() == false)
		SetLightMap(g_lightmap.lock());

	UpdateRenderSettings();
	return true;
}
void RasterizationRenderer::SetFrameDepthBufferSamplingRequired() {m_bFrameDepthBufferSamplingRequired = true;}
void RasterizationRenderer::EndRendering() {}
void RasterizationRenderer::BeginRendering(const util::DrawSceneInfo &drawSceneInfo)
{
	BaseRenderer::BeginRendering(drawSceneInfo);
	umath::set_flag(m_stateFlags,StateFlags::DepthResolved | StateFlags::BloomResolved | StateFlags::RenderResolved,false);
}

bool RasterizationRenderer::IsSSAOEnabled() const {return umath::is_flag_set(m_stateFlags,StateFlags::SSAOEnabled);}
void RasterizationRenderer::SetSSAOEnabled(pragma::CSceneComponent &scene,bool b)
{
	umath::set_flag(m_stateFlags,StateFlags::SSAOEnabled,b);
	UpdateRenderSettings();
	ReloadRenderTarget(scene,GetWidth(),GetHeight());
	/*m_hdrInfo.prepass.SetUseExtendedPrepass(b);
	if(b == true)
	{
	auto &context = c_engine->GetRenderContext();
	m_hdrInfo.ssaoInfo.Initialize(context,GetWidth(),GetHeight(),static_cast<Anvil::SampleCountFlagBits>(c_game->GetMSAASampleCount()),m_hdrInfo.prepass.texturePositions,m_hdrInfo.prepass.textureNormals,m_hdrInfo.prepass.textureDepth);
	}
	else
	m_hdrInfo.ssaoInfo.Clear();
	UpdateRenderSettings();*/
}
void RasterizationRenderer::UpdateCameraData(pragma::CSceneComponent &scene,pragma::CameraData &cameraData)
{
	UpdateFrustumPlanes(scene);
}
void RasterizationRenderer::UpdateRenderSettings()
{
	auto &tileInfo = m_rendererData.tileInfo;
	tileInfo = static_cast<uint32_t>(pragma::ShaderForwardPLightCulling::TILE_SIZE)<<16;
	tileInfo |= static_cast<uint32_t>(pragma::rendering::ForwardPlusInstance::CalcWorkGroupCount(GetWidth(),GetHeight()).first);
	m_rendererData.SetResolution(GetWidth(),GetHeight());

	if(IsSSAOEnabled() == true)
		m_rendererData.flags |= pragma::rendering::RendererData::Flags::SSAOEnabled;
}
void RasterizationRenderer::SetShaderOverride(const std::string &srcShaderId,const std::string &shaderOverrideId)
{
	auto hSrcShader = c_engine->GetShader(srcShaderId);
	if(hSrcShader.get()->GetBaseTypeHashCode() != pragma::ShaderTextured3DBase::HASH_TYPE)
		return;
	auto *srcShader = dynamic_cast<pragma::ShaderTextured3DBase*>(hSrcShader.get());
	if(srcShader == nullptr)
		return;
	auto hDstShader = c_engine->GetShader(shaderOverrideId);
	auto dstShader = dynamic_cast<pragma::ShaderTextured3DBase*>(hDstShader.get());
	if(dstShader == nullptr)
		return;
	m_shaderOverrides[typeid(*srcShader).hash_code()] = dstShader->GetHandle();
}
pragma::ShaderTextured3DBase *RasterizationRenderer::GetShaderOverride(pragma::ShaderTextured3DBase *srcShader) const
{
	if(srcShader == nullptr)
		return nullptr;
	auto it = m_shaderOverrides.find(typeid(*srcShader).hash_code());
	if(it == m_shaderOverrides.end())
		return srcShader;
	return static_cast<pragma::ShaderTextured3DBase*>(it->second.get());
}
void RasterizationRenderer::ClearShaderOverride(const std::string &srcShaderId)
{
	auto hSrcShader = c_engine->GetShader(srcShaderId);
	auto *srcShader = dynamic_cast<pragma::ShaderTextured3DBase*>(hSrcShader.get());
	if(srcShader == nullptr)
		return;
	auto it = m_shaderOverrides.find(typeid(*srcShader).hash_code());
	if(it == m_shaderOverrides.end())
		return;
	m_shaderOverrides.erase(it);
}

void RasterizationRenderer::SetPrepassMode(PrepassMode mode)
{
	auto &prepass = GetPrepass();
	switch(static_cast<PrepassMode>(mode))
	{
	case PrepassMode::NoPrepass:
		umath::set_flag(m_stateFlags,StateFlags::PrepassEnabled,false);
		break;
	case PrepassMode::DepthOnly:
		umath::set_flag(m_stateFlags,StateFlags::PrepassEnabled,true);
		prepass.SetUseExtendedPrepass(false);
		break;
	case PrepassMode::Extended:
		umath::set_flag(m_stateFlags,StateFlags::PrepassEnabled,true);
		prepass.SetUseExtendedPrepass(true);
		break;
	}
}
RasterizationRenderer::PrepassMode RasterizationRenderer::GetPrepassMode() const
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::PrepassEnabled) == false)
		return PrepassMode::NoPrepass;
	auto &prepass = const_cast<RasterizationRenderer*>(this)->GetPrepass();
	return prepass.IsExtended() ? PrepassMode::Extended : PrepassMode::DepthOnly;
}

pragma::ShaderPrepassBase &RasterizationRenderer::GetPrepassShader() const {return const_cast<RasterizationRenderer*>(this)->GetPrepass().GetShader();}

HDRData &RasterizationRenderer::GetHDRInfo() {return m_hdrInfo;}
const HDRData &RasterizationRenderer::GetHDRInfo() const {return const_cast<RasterizationRenderer*>(this)->GetHDRInfo();}
GlowData &RasterizationRenderer::GetGlowInfo() {return m_glowInfo;}
SSAOInfo &RasterizationRenderer::GetSSAOInfo() {return m_hdrInfo.ssaoInfo;}
pragma::rendering::Prepass &RasterizationRenderer::GetPrepass() {return m_hdrInfo.prepass;}
const pragma::rendering::ForwardPlusInstance &RasterizationRenderer::GetForwardPlusInstance() const {return const_cast<RasterizationRenderer*>(this)->GetForwardPlusInstance();}
pragma::rendering::ForwardPlusInstance &RasterizationRenderer::GetForwardPlusInstance() {return m_hdrInfo.forwardPlusInstance;}

Float RasterizationRenderer::GetHDRExposure() const {return m_hdrInfo.exposure;}
Float RasterizationRenderer::GetMaxHDRExposure() const {return m_hdrInfo.max_exposure;}
void RasterizationRenderer::SetMaxHDRExposure(Float exposure) {m_hdrInfo.max_exposure = exposure;}

const std::vector<Plane> &RasterizationRenderer::GetFrustumPlanes() const {return m_frustumPlanes;}
const std::vector<Plane> &RasterizationRenderer::GetClippedFrustumPlanes() const {return m_clippedFrustumPlanes;}

void RasterizationRenderer::UpdateFrustumPlanes(pragma::CSceneComponent &scene)
{
	m_frustumPlanes.clear();
	m_clippedFrustumPlanes.clear();
	auto &cam = scene.GetActiveCamera();
	if(cam.expired())
		return;
	cam->GetFrustumPlanes(m_frustumPlanes);
	m_clippedFrustumPlanes = m_frustumPlanes;
/*	auto forward = camera->GetForward();
	auto up = camera->GetUp();
	auto rot = camera->GetRotation();
	auto pos = camera->GetPos();
	camera->SetForward(uvec::FORWARD);
	camera->SetUp(uvec::UP);
	camera->SetPos(Vector3{});

	std::vector<Vector3> frustumPoints {};
	camera->GetFrustumPoints(frustumPoints);
	for(auto &p : frustumPoints)
	{
		uvec::rotate(&p,rot);
		p += pos;
	}
	camera->GetFrustumPlanes(frustumPoints,m_frustumPlanes);
	m_clippedFrustumPlanes = m_frustumPlanes;

	camera->SetForward(forward);
	camera->SetUp(up);
	camera->SetPos(pos);*/


	/*if(FogController::IsFogEnabled() == true)
	{
		float fogDist = FogController::GetFarDistance();
		float farZ = cam->GetZFar();
		if(fogDist < farZ)
			farZ = fogDist;
		Plane &farPlane = planesClipped[static_cast<int>(FrustumPlane::Far)];
		Vector3 &start = planesClipped[static_cast<int>(FrustumPlane::Near)].GetCenterPos();
		Vector3 dir = farPlane.GetCenterPos() -start;
		uvec::normalize(&dir);
		farPlane.MoveToPos(start +dir *farZ); // TODO Checkme
	}*/
}

void RasterizationRenderer::SetLightMap(const std::shared_ptr<prosper::Texture> &lightMapTexture)
{
	if(lightMapTexture == m_lightMapInfo.lightMapTexture)
		return;
	m_lightMapInfo.lightMapTexture = lightMapTexture;

	auto &ds = *m_descSetGroupRenderer->GetDescriptorSet();
	ds.SetBindingTexture(*lightMapTexture,umath::to_integral(pragma::ShaderScene::RendererBinding::LightMap));
}
const std::shared_ptr<prosper::Texture> &RasterizationRenderer::GetLightMap() const {return m_lightMapInfo.lightMapTexture;}
prosper::Texture *RasterizationRenderer::GetSceneTexture() {return m_hdrInfo.sceneRenderTarget ? &m_hdrInfo.sceneRenderTarget->GetTexture() : nullptr;}
prosper::Texture *RasterizationRenderer::GetPresentationTexture() {return m_hdrInfo.toneMappedRenderTarget ? &m_hdrInfo.toneMappedRenderTarget->GetTexture() : nullptr;}
prosper::Texture *RasterizationRenderer::GetHDRPresentationTexture() {return m_hdrInfo.sceneRenderTarget ? &m_hdrInfo.sceneRenderTarget->GetTexture() : nullptr;}
bool RasterizationRenderer::IsRasterizationRenderer() const {return true;}

prosper::SampleCountFlags RasterizationRenderer::GetSampleCount() const {return const_cast<RasterizationRenderer*>(this)->GetHDRInfo().sceneRenderTarget->GetTexture().GetImage().GetSampleCount();}
bool RasterizationRenderer::IsMultiSampled() const {return GetSampleCount() != prosper::SampleCountFlags::e1Bit;}

bool RasterizationRenderer::BeginRenderPass(const util::DrawSceneInfo &drawSceneInfo,prosper::IRenderPass *customRenderPass)
{
	auto &hdrInfo = GetHDRInfo();
	auto &rt = hdrInfo.GetRenderTarget(drawSceneInfo);
	auto &tex = rt.GetTexture();
	if(tex.IsMSAATexture())
		static_cast<prosper::MSAATexture&>(tex).Reset();
	return hdrInfo.BeginRenderPass(drawSceneInfo,customRenderPass);
}
bool RasterizationRenderer::EndRenderPass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &hdrInfo = GetHDRInfo();
	return hdrInfo.EndRenderPass(drawSceneInfo);
}
bool RasterizationRenderer::ResolveRenderPass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &hdrInfo = GetHDRInfo();
	return hdrInfo.ResolveRenderPass(drawSceneInfo);
}

prosper::Shader *RasterizationRenderer::GetWireframeShader() const {return m_whShaderWireframe.get();}
