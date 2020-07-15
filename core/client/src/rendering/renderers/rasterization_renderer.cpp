/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_brute_force.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_bsp.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_chc.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_inert.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_octtree.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/renderers/rasterization/glow_data.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
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
	auto &scene = c_game->GetScene();
	if(scene == nullptr)
		return;
	auto *renderer = dynamic_cast<RasterizationRenderer*>(scene->GetRenderer());
	if(renderer == nullptr)
		return;
	renderer->SetSSAOEnabled(val);
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_ssao,cl_render_ssao_callback);

RasterizationRenderer::RasterizationRenderer(Scene &scene)
	: BaseRenderer{scene},m_hdrInfo{*this}
{
	m_whShaderWireframe = c_engine->GetShader("wireframe");

	InitializeLightDescriptorSets();
	ReloadOcclusionCullingHandler();

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
	m_occlusionCullingHandler = nullptr;
}

void RasterizationRenderer::UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	drawCmd->RecordUpdateBuffer(*m_rendererBuffer,0ull,m_rendererData);
}

bool RasterizationRenderer::Initialize() {return true;}

CulledMeshData *RasterizationRenderer::GetRenderInfo(RenderMode renderMode) const
{
	auto &renderMeshData = m_renderMeshCollectionHandler.GetRenderMeshData();
	auto it = renderMeshData.find(renderMode);
	if(it == renderMeshData.end())
		return nullptr;
	return it->second.get();
}

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
	// Occlusion Culling
	PerformOcclusionCulling();

	// Collect render objects
	c_game->CallCallbacks<void,RasterizationRenderer*>("OnPreRender",this);
	CollectRenderObjects(drawSceneInfo.renderFlags);
	c_game->CallLuaCallbacks<void,RasterizationRenderer*>("PrepareRendering",this);

	// Collect 3D skybox data
	m_3dSkyCameras.clear();
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CSkyCameraComponent>>();
	for(auto *ent : entIt)
	{
		if(static_cast<CBaseEntity*>(ent)->IsInScene(GetScene()) == false)
			continue;

		auto skyCamera = ent->GetComponent<pragma::CSkyCameraComponent>();
		auto &renderMeshes = skyCamera->UpdateRenderMeshes(*this,drawSceneInfo.renderFlags);
		m_3dSkyCameras.push_back(skyCamera);
	}
	//

	// Prepass
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::Scene);

	auto &drawCmd = drawSceneInfo.commandBuffer;
	if(drawSceneInfo.renderTarget == nullptr)
		RenderPrepass(drawSceneInfo);

	// SSAO
	RenderSSAO(drawSceneInfo);

	// Cull light sources
	CullLightSources(drawSceneInfo);

	// Lighting pass
	RenderLightingPass(drawSceneInfo);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::Scene);

	// Post processing
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::PostProcessing);
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessing);

	// Fog
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::PostProcessingFog);
	RenderSceneFog(drawSceneInfo);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessingFog);

	// Glow
	RenderGlowObjects(drawSceneInfo);
	c_game->CallCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("RenderPostProcessing",drawSceneInfo);
	c_game->CallLuaCallbacks<void,std::reference_wrapper<const util::DrawSceneInfo>>("RenderPostProcessing",drawSceneInfo);

	// Bloom
	RenderBloom(drawSceneInfo);

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

	// FXAA
	RenderFXAA(drawSceneInfo);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::PostProcessing);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::PostProcessing);
}

bool RasterizationRenderer::ReloadRenderTarget(uint32_t width,uint32_t height)
{
	auto &scene = GetScene();
	auto bSsao = IsSSAOEnabled();
	if(
		m_hdrInfo.Initialize(*this,width,height,m_sampleCount,bSsao) == false || 
		m_glowInfo.Initialize(width,height,m_hdrInfo) == false ||
		m_hdrInfo.InitializeDescriptorSets() == false
		)
		return false;

	auto &descSetHdrResolve = *m_hdrInfo.dsgBloomTonemapping->GetDescriptorSet();
	auto *resolvedGlowTex = &GetGlowInfo().renderTarget->GetTexture();
	if(resolvedGlowTex->IsMSAATexture())
		resolvedGlowTex = static_cast<prosper::MSAATexture*>(resolvedGlowTex)->GetResolvedTexture().get();
	descSetHdrResolve.SetBindingTexture(*resolvedGlowTex,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));

	auto &descSetCam = *scene.GetCameraDescriptorSetGraphics();
	auto &descSetCamView = *scene.GetViewCameraDescriptorSet();
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
	UpdateRenderSettings();
	return true;
}
void RasterizationRenderer::SetFrameDepthBufferSamplingRequired() {m_bFrameDepthBufferSamplingRequired = true;}
void RasterizationRenderer::EndRendering() {}
void RasterizationRenderer::BeginRendering(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	BaseRenderer::BeginRendering(drawCmd);
	umath::set_flag(m_stateFlags,StateFlags::DepthResolved | StateFlags::BloomResolved | StateFlags::RenderResolved,false);
}

bool RasterizationRenderer::IsSSAOEnabled() const {return umath::is_flag_set(m_stateFlags,StateFlags::SSAOEnabled);}
void RasterizationRenderer::SetSSAOEnabled(bool b)
{
	umath::set_flag(m_stateFlags,StateFlags::SSAOEnabled,b);
	UpdateRenderSettings();
	ReloadRenderTarget(GetWidth(),GetHeight());
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
void RasterizationRenderer::UpdateCameraData(pragma::CameraData &cameraData)
{
	UpdateFrustumPlanes();
}
void RasterizationRenderer::UpdateRenderSettings()
{
	auto &scene = GetScene();
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
pragma::ShaderTextured3DBase *RasterizationRenderer::GetShaderOverride(pragma::ShaderTextured3DBase *srcShader)
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

const std::vector<pragma::OcclusionMeshInfo> &RasterizationRenderer::GetCulledMeshes() const {return m_renderMeshCollectionHandler.GetOcclusionFilteredMeshes();}
std::vector<pragma::OcclusionMeshInfo> &RasterizationRenderer::GetCulledMeshes() {return m_renderMeshCollectionHandler.GetOcclusionFilteredMeshes();}
const std::vector<pragma::CParticleSystemComponent*> &RasterizationRenderer::GetCulledParticles() const {return m_renderMeshCollectionHandler.GetOcclusionFilteredParticleSystems();}
std::vector<pragma::CParticleSystemComponent*> &RasterizationRenderer::GetCulledParticles() {return m_renderMeshCollectionHandler.GetOcclusionFilteredParticleSystems();}

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
GlowData &RasterizationRenderer::GetGlowInfo() {return m_glowInfo;}
SSAOInfo &RasterizationRenderer::GetSSAOInfo() {return m_hdrInfo.ssaoInfo;}
pragma::rendering::Prepass &RasterizationRenderer::GetPrepass() {return m_hdrInfo.prepass;}
const pragma::rendering::ForwardPlusInstance &RasterizationRenderer::GetForwardPlusInstance() const {return const_cast<RasterizationRenderer*>(this)->GetForwardPlusInstance();}
pragma::rendering::ForwardPlusInstance &RasterizationRenderer::GetForwardPlusInstance() {return m_hdrInfo.forwardPlusInstance;}

Float RasterizationRenderer::GetHDRExposure() const {return m_hdrInfo.exposure;}
Float RasterizationRenderer::GetMaxHDRExposure() const {return m_hdrInfo.max_exposure;}
void RasterizationRenderer::SetMaxHDRExposure(Float exposure) {m_hdrInfo.max_exposure = exposure;}

const pragma::OcclusionCullingHandler &RasterizationRenderer::GetOcclusionCullingHandler() const {return const_cast<RasterizationRenderer*>(this)->GetOcclusionCullingHandler();}
pragma::OcclusionCullingHandler &RasterizationRenderer::GetOcclusionCullingHandler() {return *m_occlusionCullingHandler;}
void RasterizationRenderer::SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler) {m_occlusionCullingHandler = handler;}
void RasterizationRenderer::ReloadOcclusionCullingHandler()
{
	auto occlusionCullingMode = c_game->GetConVarInt("cl_render_occlusion_culling");
	switch(occlusionCullingMode)
	{
	case 1: /* Brute-force */
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerBruteForce>();
		break;
	case 2: /* CHC++ */
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerCHC>();
		break;
	case 4: /* BSP */
	{
		auto *world = c_game->GetWorld();
		if(world)
		{
			auto &entWorld = world->GetEntity();
			auto pWorldComponent = entWorld.GetComponent<pragma::CWorldComponent>();
			auto bspTree = pWorldComponent.valid() ? pWorldComponent->GetBSPTree() : nullptr;
			if(bspTree != nullptr && bspTree->GetNodes().size() > 1u)
			{
				m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerBSP>(bspTree);
				break;
			}
		}
	}
	case 3: /* Octtree */
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerOctTree>();
		break;
	case 0: /* Off */
	default:
		m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerInert>();
		break;
	}
	m_occlusionCullingHandler->Initialize();
}

const std::vector<Plane> &RasterizationRenderer::GetFrustumPlanes() const {return m_frustumPlanes;}
const std::vector<Plane> &RasterizationRenderer::GetClippedFrustumPlanes() const {return m_clippedFrustumPlanes;}

void RasterizationRenderer::UpdateFrustumPlanes()
{
	m_frustumPlanes.clear();
	m_clippedFrustumPlanes.clear();
	auto &scene = GetScene();
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
RenderMeshCollectionHandler &RasterizationRenderer::GetRenderMeshCollectionHandler() {return m_renderMeshCollectionHandler;}
const RenderMeshCollectionHandler &RasterizationRenderer::GetRenderMeshCollectionHandler() const {return const_cast<RasterizationRenderer*>(this)->GetRenderMeshCollectionHandler();}

prosper::Shader *RasterizationRenderer::GetWireframeShader() {return m_whShaderWireframe.get();}
