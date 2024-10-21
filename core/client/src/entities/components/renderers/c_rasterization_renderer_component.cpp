/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/lighting/c_light_data_buffer_manager.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/entities/environment/lights/c_env_shadow_csm.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <image/prosper_msaa_texture.hpp>
#include <pragma/logging.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

LINK_ENTITY_TO_CLASS(rasterization_renderer, CRasterizationRenderer);

ComponentEventId CRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_ON_RECORD_LIGHTING_PASS = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_PRE_EXECUTE_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_POST_EXECUTE_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_PRE_EXECUTE_LIGHTING_PASS = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_POST_EXECUTE_LIGHTING_PASS = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_PRE_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_POST_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_PRE_LIGHTING_PASS = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_POST_LIGHTING_PASS = INVALID_COMPONENT_ID;

ComponentEventId CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_SKYBOX = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_END_RECORD_SKYBOX = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_WORLD = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_END_RECORD_WORLD = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_PARTICLES = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_END_RECORD_PARTICLES = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_DEBUG = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_END_RECORD_DEBUG = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_WATER = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_END_RECORD_WATER = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_VIEW = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_END_RECORD_VIEW = INVALID_COMPONENT_ID;
ComponentEventId CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_PREPASS = INVALID_COMPONENT_ID;
void CRasterizationRendererComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_RECORD_PREPASS = registerEvent("ON_RECORD_PREPASS", ComponentEventInfo::Type::Explicit);
	EVENT_ON_RECORD_LIGHTING_PASS = registerEvent("ON_RECORD_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);
	EVENT_PRE_EXECUTE_PREPASS = registerEvent("PRE_EXECUTE_PREPASS", ComponentEventInfo::Type::Explicit);
	EVENT_POST_EXECUTE_PREPASS = registerEvent("POST_EXECUTE_PREPASS", ComponentEventInfo::Type::Explicit);
	EVENT_PRE_EXECUTE_LIGHTING_PASS = registerEvent("PRE_EXECUTE_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);
	EVENT_POST_EXECUTE_LIGHTING_PASS = registerEvent("POST_EXECUTE_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);
	EVENT_PRE_PREPASS = registerEvent("PRE_PREPASS", ComponentEventInfo::Type::Explicit);
	EVENT_POST_PREPASS = registerEvent("POST_PREPASS", ComponentEventInfo::Type::Explicit);
	EVENT_PRE_LIGHTING_PASS = registerEvent("PRE_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);
	EVENT_POST_LIGHTING_PASS = registerEvent("POST_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);

	EVENT_MT_BEGIN_RECORD_SKYBOX = registerEvent("MT_BEGIN_RECORD_SKYBOX", ComponentEventInfo::Type::Explicit);
	EVENT_MT_END_RECORD_SKYBOX = registerEvent("MT_END_RECORD_SKYBOX", ComponentEventInfo::Type::Explicit);
	EVENT_MT_BEGIN_RECORD_WORLD = registerEvent("MT_BEGIN_RECORD_WORLD", ComponentEventInfo::Type::Explicit);
	EVENT_MT_END_RECORD_WORLD = registerEvent("MT_END_RECORD_WORLD", ComponentEventInfo::Type::Explicit);
	EVENT_MT_BEGIN_RECORD_PARTICLES = registerEvent("MT_BEGIN_RECORD_PARTICLES", ComponentEventInfo::Type::Explicit);
	EVENT_MT_END_RECORD_PARTICLES = registerEvent("MT_END_RECORD_PARTICLES", ComponentEventInfo::Type::Explicit);
	EVENT_MT_BEGIN_RECORD_DEBUG = registerEvent("MT_BEGIN_RECORD_DEBUG", ComponentEventInfo::Type::Explicit);
	EVENT_MT_END_RECORD_DEBUG = registerEvent("MT_END_RECORD_DEBUG", ComponentEventInfo::Type::Explicit);
	EVENT_MT_BEGIN_RECORD_WATER = registerEvent("MT_BEGIN_RECORD_WATER", ComponentEventInfo::Type::Explicit);
	EVENT_MT_END_RECORD_WATER = registerEvent("MT_END_RECORD_WATER", ComponentEventInfo::Type::Explicit);
	EVENT_MT_BEGIN_RECORD_VIEW = registerEvent("MT_BEGIN_RECORD_VIEW", ComponentEventInfo::Type::Explicit);
	EVENT_MT_END_RECORD_VIEW = registerEvent("MT_END_RECORD_VIEW", ComponentEventInfo::Type::Explicit);
	EVENT_MT_BEGIN_RECORD_PREPASS = registerEvent("MT_BEGIN_RECORD_PREPASS", ComponentEventInfo::Type::Explicit);
}

void CRasterizationRendererComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

static pragma::ComponentHandle<pragma::CLightMapComponent> g_lightmapC = {};
void CRasterizationRendererComponent::UpdateLightmap(CLightMapComponent &lightMapC)
{
	if(!lightMapC.HasValidLightMap()) {
		CLightMapComponent::LOGGER.warn("Lightmap has no valid lightmap texture!");
		return;
	}
	for(auto &renderer : EntityCIterator<CRasterizationRendererComponent> {*c_game})
		renderer.SetLightMap(lightMapC);
	g_lightmapC = lightMapC.GetHandle<CLightMapComponent>();
}
void CRasterizationRendererComponent::UpdateLightmap()
{
	if(g_lightmapC.expired()) {
		CLightMapComponent::LOGGER.warn("No lightmap component found!");
		return;
	}
	UpdateLightmap(*g_lightmapC);
}

CRasterizationRendererComponent::CRasterizationRendererComponent(BaseEntity &ent) : BaseEntityComponent {ent}, m_hdrInfo {*this}
{
	m_whShaderWireframe = c_engine->GetShader("wireframe");

	InitializeLightDescriptorSets();
	InitializeCommandBufferGroups();

	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.size = sizeof(m_rendererData);
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	m_rendererBuffer = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo, &m_rendererData);

	auto *shaderPbr = static_cast<pragma::ShaderPBR *>(c_engine->GetShader("pbr").get());
	assert(shaderPbr);
	if(shaderPbr) {
		m_descSetGroupRenderer = shaderPbr->CreateDescriptorSetGroup(pragma::ShaderPBR::DESCRIPTOR_SET_RENDERER.setIndex);
		m_descSetGroupRenderer->GetDescriptorSet()->SetBindingUniformBuffer(*m_rendererBuffer, umath::to_integral(pragma::ShaderScene::RendererBinding::Renderer));
	}
}

CRasterizationRendererComponent::~CRasterizationRendererComponent()
{
	auto &renderContext = c_engine->GetRenderContext();
	renderContext.KeepResourceAliveUntilPresentationComplete(m_prepassCommandBufferGroup);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_shadowCommandBufferGroup);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_lightingCommandBufferGroup);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_descSetGroupFogOverride);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_dsgLights);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_dsgLightsCompute);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_rendererBuffer);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_descSetGroupRenderer);
}

void CRasterizationRendererComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(CRendererComponent::EVENT_RELOAD_RENDER_TARGET, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &reloadRenderTargetEv = static_cast<CEReloadRenderTarget &>(evData.get());
		reloadRenderTargetEv.resultSuccess = ReloadRenderTarget(reloadRenderTargetEv.width, reloadRenderTargetEv.height);
	});
	BindEventUnhandled(CRendererComponent::EVENT_RELOAD_BLOOM_RENDER_TARGET, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { m_hdrInfo.ReloadBloomRenderTarget(static_cast<CEReloadBloomRenderTarget &>(evData.get()).width); });
	BindEventUnhandled(CRendererComponent::EVENT_BEGIN_RENDERING, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { BeginRendering(static_cast<CEBeginRendering &>(evData.get()).drawSceneInfo); });
	BindEventUnhandled(CRendererComponent::EVENT_END_RENDERING, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { EndRendering(); });
	BindEventUnhandled(CRendererComponent::EVENT_UPDATE_CAMERA_DATA, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateFrustumPlanes(static_cast<CEUpdateCameraData &>(evData.get()).scene); });
	BindEvent(CRendererComponent::EVENT_GET_SCENE_TEXTURE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		static_cast<pragma::CEGetSceneTexture &>(evData.get()).resultTexture = m_hdrInfo.sceneRenderTarget ? &m_hdrInfo.sceneRenderTarget->GetTexture() : nullptr;
		return util::EventReply::Handled;
	});
	BindEvent(CRendererComponent::EVENT_GET_PRESENTATION_TEXTURE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		static_cast<pragma::CEGetPresentationTexture &>(evData.get()).resultTexture = m_hdrInfo.toneMappedRenderTarget ? &m_hdrInfo.toneMappedRenderTarget->GetTexture() : nullptr;
		return util::EventReply::Handled;
	});
	BindEvent(CRendererComponent::EVENT_GET_HDR_PRESENTATION_TEXTURE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		static_cast<pragma::CEGetHdrPresentationTexture &>(evData.get()).resultTexture = m_hdrInfo.sceneRenderTarget ? &m_hdrInfo.sceneRenderTarget->GetTexture() : nullptr;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(CRendererComponent::EVENT_RECORD_COMMAND_BUFFERS, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { RecordCommandBuffers(static_cast<CEDrawSceneInfo &>(evData.get()).drawSceneInfo); });
	BindEventUnhandled(CRendererComponent::EVENT_RENDER, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { Render(static_cast<CERender &>(evData.get()).drawSceneInfo); });
	BindEventUnhandled(CRendererComponent::EVENT_UPDATE_RENDER_SETTINGS, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateRenderSettings(); });
	BindEventUnhandled(CRendererComponent::EVENT_UPDATE_RENDERER_BUFFER, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateRendererBuffer(static_cast<CEUpdateRendererBuffer &>(evData.get()).drawCommandBuffer); });

	auto &ent = GetEntity();
	m_rendererComponent = ent.AddComponent<CRendererComponent>().get();

	UpdateLightmap();
}

void CRasterizationRendererComponent::UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) { drawCmd->RecordUpdateBuffer(*m_rendererBuffer, 0ull, m_rendererData); }

prosper::IDescriptorSet *CRasterizationRendererComponent::GetDepthDescriptorSet() const { return (m_hdrInfo.dsgSceneDepth != nullptr) ? m_hdrInfo.dsgSceneDepth->GetDescriptorSet() : nullptr; }
prosper::IDescriptorSet *CRasterizationRendererComponent::GetRendererDescriptorSet() const { return m_descSetGroupRenderer->GetDescriptorSet(); }

prosper::IDescriptorSet *CRasterizationRendererComponent::GetLightSourceDescriptorSet() const { return m_dsgLights->GetDescriptorSet(); }
prosper::IDescriptorSet *CRasterizationRendererComponent::GetLightSourceDescriptorSetCompute() const { return m_dsgLightsCompute->GetDescriptorSet(); }

void CRasterizationRendererComponent::InitializeCommandBufferGroups()
{
	auto &context = c_engine->GetRenderContext();
	auto &window = context.GetWindow();
	m_prepassCommandBufferGroup = context.CreateSwapCommandBufferGroup(window);
	m_shadowCommandBufferGroup = context.CreateSwapCommandBufferGroup(window);
	m_lightingCommandBufferGroup = context.CreateSwapCommandBufferGroup(window);
}

void CRasterizationRendererComponent::InitializeLightDescriptorSets()
{
	if(pragma::ShaderGameWorldLightingPass::DESCRIPTOR_SET_LIGHTS.IsValid()) {
		auto &bufLightSources = pragma::CLightComponent::GetGlobalRenderBuffer();
		auto &bufShadowData = pragma::CLightComponent::GetGlobalShadowBuffer();
		m_dsgLights = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderGameWorldLightingPass::DESCRIPTOR_SET_LIGHTS);
#if USE_LIGHT_SOURCE_UNIFORM_BUFFER == 1
		m_dsgLights->GetDescriptorSet()->SetBindingUniformBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufLightSources), umath::to_integral(pragma::ShaderGameWorldLightingPass::LightBinding::LightBuffers));
		m_dsgLights->GetDescriptorSet()->SetBindingUniformBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufShadowData), umath::to_integral(pragma::ShaderGameWorldLightingPass::LightBinding::ShadowData));
#else
		m_dsgLights->GetDescriptorSet()->SetBindingStorageBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufLightSources), umath::to_integral(pragma::ShaderGameWorldLightingPass::LightBinding::LightBuffers));
		m_dsgLights->GetDescriptorSet()->SetBindingStorageBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufShadowData), umath::to_integral(pragma::ShaderGameWorldLightingPass::LightBinding::ShadowData));
#endif

		m_dsgLightsCompute = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS);
#if USE_LIGHT_SOURCE_UNIFORM_BUFFER == 1
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingUniformBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufLightSources), umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::LightBuffers));
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingUniformBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufShadowData), umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::ShadowData));
#else
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingStorageBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufLightSources), umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::LightBuffers));
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingStorageBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufShadowData), umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::ShadowData));
#endif
	}
}

void CRasterizationRendererComponent::UpdateCSMDescriptorSet(pragma::CLightDirectionalComponent &lightSource)
{
	auto *dsLights = GetLightSourceDescriptorSet();
	if(dsLights == nullptr)
		return;
	auto *pShadowMap = lightSource.GetShadowMap();
	auto texture = pShadowMap ? pShadowMap->GetDepthTexture() : nullptr;
	if(texture == nullptr)
		return;
	auto numLayers = pShadowMap->GetLayerCount();
	for(auto i = decltype(numLayers) {0}; i < numLayers; ++i) {
		dsLights->SetBindingArrayTexture(*texture, umath::to_integral(pragma::ShaderSceneLit::LightBinding::CSM), i, i);
	}
}

void CRasterizationRendererComponent::SetFogOverride(const std::shared_ptr<prosper::IDescriptorSetGroup> &descSetGroup) { m_descSetGroupFogOverride = descSetGroup; }
const std::shared_ptr<prosper::IDescriptorSetGroup> &CRasterizationRendererComponent::GetFogOverride() const { return m_descSetGroupFogOverride; }

// TODO: Remove this
#if DEBUG_RENDER_PERFORMANCE_TEST_ENABLED == 1
extern int g_dbgMode;
#endif

bool CRasterizationRendererComponent::ReloadRenderTarget(uint32_t width, uint32_t height)
{
	auto bSsao = IsSSAOEnabled();
	if(m_hdrInfo.Initialize(width, height, m_sampleCount, bSsao) == false ||
	  //m_glowInfo.Initialize(width,height,m_hdrInfo) == false ||
	  m_hdrInfo.InitializeDescriptorSets() == false)
		return false;

	auto &descSetHdrResolve = *m_hdrInfo.dsgBloomTonemapping->GetDescriptorSet();
	//auto *resolvedGlowTex = &GetGlowInfo().renderTarget->GetTexture();
	//if(resolvedGlowTex->IsMSAATexture())
	//	resolvedGlowTex = static_cast<prosper::MSAATexture*>(resolvedGlowTex)->GetResolvedTexture().get();
	//descSetHdrResolve.SetBindingTexture(*resolvedGlowTex,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));

	if(bSsao == true) {
		auto &ssaoInfo = GetSSAOInfo();
		auto *ssaoBlurTexResolved = &ssaoInfo.renderTargetBlur->GetTexture();
		if(ssaoBlurTexResolved->IsMSAATexture())
			ssaoBlurTexResolved = static_cast<prosper::MSAATexture *>(ssaoBlurTexResolved)->GetResolvedTexture().get();

		auto &ds = *m_descSetGroupRenderer->GetDescriptorSet();
		ds.SetBindingTexture(*ssaoBlurTexResolved, umath::to_integral(pragma::ShaderScene::RendererBinding::SSAOMap));
	}
	auto &dummyTex = c_engine->GetRenderContext().GetDummyTexture();
	auto &ds = *m_descSetGroupRenderer->GetDescriptorSet();
	ds.SetBindingTexture(*dummyTex, umath::to_integral(pragma::ShaderScene::RendererBinding::LightMapDiffuse));
	ds.SetBindingTexture(*dummyTex, umath::to_integral(pragma::ShaderScene::RendererBinding::LightMapDiffuseIndirect));
	ds.SetBindingTexture(*dummyTex, umath::to_integral(pragma::ShaderScene::RendererBinding::LightMapDominantDirection));

	m_lightMapInfo.lightMapTexture = nullptr;
	if(g_lightmapC.expired() == false)
		SetLightMap(*g_lightmapC);

	UpdateRenderSettings();
	return true;
}
void CRasterizationRendererComponent::SetFrameDepthBufferSamplingRequired() { m_bFrameDepthBufferSamplingRequired = true; }
void CRasterizationRendererComponent::EndRendering() {}
void CRasterizationRendererComponent::BeginRendering(const util::DrawSceneInfo &drawSceneInfo) { umath::set_flag(m_stateFlags, StateFlags::DepthResolved | StateFlags::BloomResolved | StateFlags::RenderResolved, false); }

void CRasterizationRendererComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CRendererComponent))
		m_rendererComponent = static_cast<CRendererComponent *>(&component);
}
void CRasterizationRendererComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(CRendererComponent))
		m_rendererComponent = nullptr;
}

bool CRasterizationRendererComponent::IsSSAOEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::SSAOEnabled) && client->GetGameWorldShaderSettings().ssaoEnabled; }
void CRasterizationRendererComponent::SetSSAOEnabled(bool b)
{
	umath::set_flag(m_stateFlags, StateFlags::SSAOEnabled, b);
	UpdateRenderSettings();
	if(GetWidth() > 0 && GetHeight() > 0)
		ReloadRenderTarget(GetWidth(), GetHeight());
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
uint32_t CRasterizationRendererComponent::GetWidth() const { return m_rendererComponent ? m_rendererComponent->GetWidth() : 0; }
uint32_t CRasterizationRendererComponent::GetHeight() const { return m_rendererComponent ? m_rendererComponent->GetHeight() : 0; }
void CRasterizationRendererComponent::UpdateRenderSettings()
{
	auto &tileInfo = m_rendererData.tileInfo;
	tileInfo = static_cast<uint32_t>(pragma::ShaderForwardPLightCulling::TILE_SIZE) << 16;
	tileInfo |= static_cast<uint32_t>(pragma::rendering::ForwardPlusInstance::CalcWorkGroupCount(GetWidth(), GetHeight()).first);
	m_rendererData.SetResolution(GetWidth(), GetHeight());

	if(IsSSAOEnabled() == true)
		m_rendererData.flags |= pragma::RendererData::Flags::SSAOEnabled;
}
void CRasterizationRendererComponent::SetShaderOverride(const std::string &srcShaderId, const std::string &shaderOverrideId)
{
	auto hSrcShader = c_engine->GetShader(srcShaderId);
	if(hSrcShader.get()->GetBaseTypeHashCode() != pragma::ShaderGameWorldLightingPass::HASH_TYPE)
		return;
	auto *srcShader = dynamic_cast<pragma::ShaderGameWorldLightingPass *>(hSrcShader.get());
	if(srcShader == nullptr)
		return;
	auto hDstShader = c_engine->GetShader(shaderOverrideId);
	auto dstShader = dynamic_cast<pragma::ShaderGameWorldLightingPass *>(hDstShader.get());
	if(dstShader == nullptr)
		return;
	m_shaderOverrides[typeid(*srcShader).hash_code()] = dstShader->GetHandle();
}
pragma::ShaderGameWorldLightingPass *CRasterizationRendererComponent::GetShaderOverride(pragma::ShaderGameWorldLightingPass *srcShader) const
{
	if(srcShader == nullptr)
		return nullptr;
	auto it = m_shaderOverrides.find(typeid(*srcShader).hash_code());
	if(it == m_shaderOverrides.end())
		return srcShader;
	return static_cast<pragma::ShaderGameWorldLightingPass *>(it->second.get());
}
void CRasterizationRendererComponent::ClearShaderOverride(const std::string &srcShaderId)
{
	auto hSrcShader = c_engine->GetShader(srcShaderId);
	auto *srcShader = dynamic_cast<pragma::ShaderGameWorldLightingPass *>(hSrcShader.get());
	if(srcShader == nullptr)
		return;
	auto it = m_shaderOverrides.find(typeid(*srcShader).hash_code());
	if(it == m_shaderOverrides.end())
		return;
	m_shaderOverrides.erase(it);
}

void CRasterizationRendererComponent::SetPrepassMode(PrepassMode mode)
{
	auto &prepass = GetPrepass();
	switch(static_cast<PrepassMode>(mode)) {
	case PrepassMode::NoPrepass:
		umath::set_flag(m_stateFlags, StateFlags::PrepassEnabled, false);
		break;
	case PrepassMode::DepthOnly:
		umath::set_flag(m_stateFlags, StateFlags::PrepassEnabled, true);
		prepass.SetUseExtendedPrepass(false);
		break;
	case PrepassMode::Extended:
		umath::set_flag(m_stateFlags, StateFlags::PrepassEnabled, true);
		prepass.SetUseExtendedPrepass(true);
		break;
	}
}
CRasterizationRendererComponent::PrepassMode CRasterizationRendererComponent::GetPrepassMode() const
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::PrepassEnabled) == false)
		return PrepassMode::NoPrepass;
	auto &prepass = const_cast<CRasterizationRendererComponent *>(this)->GetPrepass();
	return prepass.IsExtended() ? PrepassMode::Extended : PrepassMode::DepthOnly;
}

pragma::ShaderPrepassBase &CRasterizationRendererComponent::GetPrepassShader() const { return const_cast<CRasterizationRendererComponent *>(this)->GetPrepass().GetShader(); }

pragma::rendering::HDRData &CRasterizationRendererComponent::GetHDRInfo() { return m_hdrInfo; }
const pragma::rendering::HDRData &CRasterizationRendererComponent::GetHDRInfo() const { return const_cast<CRasterizationRendererComponent *>(this)->GetHDRInfo(); }
// GlowData &CRasterizationRendererComponent::GetGlowInfo() {return m_glowInfo;}
SSAOInfo &CRasterizationRendererComponent::GetSSAOInfo() { return m_hdrInfo.ssaoInfo; }
pragma::rendering::Prepass &CRasterizationRendererComponent::GetPrepass() { return m_hdrInfo.prepass; }
const pragma::rendering::ForwardPlusInstance &CRasterizationRendererComponent::GetForwardPlusInstance() const { return const_cast<CRasterizationRendererComponent *>(this)->GetForwardPlusInstance(); }
pragma::rendering::ForwardPlusInstance &CRasterizationRendererComponent::GetForwardPlusInstance() { return m_hdrInfo.forwardPlusInstance; }

void CRasterizationRendererComponent::SetBloomThreshold(float threshold) { m_rendererData.bloomThreshold = threshold; }
float CRasterizationRendererComponent::GetBloomThreshold() const { return m_rendererData.bloomThreshold; }

Float CRasterizationRendererComponent::GetHDRExposure() const { return m_hdrInfo.exposure; }
Float CRasterizationRendererComponent::GetMaxHDRExposure() const { return m_hdrInfo.max_exposure; }
void CRasterizationRendererComponent::SetMaxHDRExposure(Float exposure) { m_hdrInfo.max_exposure = exposure; }

const std::vector<umath::Plane> &CRasterizationRendererComponent::GetFrustumPlanes() const { return m_frustumPlanes; }
const std::vector<umath::Plane> &CRasterizationRendererComponent::GetClippedFrustumPlanes() const { return m_clippedFrustumPlanes; }

void CRasterizationRendererComponent::UpdateFrustumPlanes(pragma::CSceneComponent &scene)
{
	m_frustumPlanes.clear();
	m_clippedFrustumPlanes.clear();
	auto &cam = scene.GetActiveCamera();
	if(cam.expired())
		return;
	cam->UpdateFrustumPlanes();
	m_frustumPlanes = cam->GetFrustumPlanes();
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

void CRasterizationRendererComponent::SetLightMap(pragma::CLightMapComponent &lightMapC)
{
	m_rendererData.lightmapExposurePow = lightMapC.CalcLightMapPowExposurePow();
	/*if(
		m_lightMapInfo.lightMapComponent.get() == &lightMapC &&
		lightMapC.GetLightMap() == m_lightMapInfo.lightMapTexture &&
		lightMapC.GetDirectionalLightMap() == m_lightMapInfo.directionalLightMapTexture
	)
		return;*/
	m_lightMapInfo.lightMapTexture = lightMapC.GetTexture(pragma::CLightMapComponent::Texture::DiffuseMap);
	if(!m_lightMapInfo.lightMapTexture) {
		m_lightMapInfo.lightMapTexture = lightMapC.GetTexture(pragma::CLightMapComponent::Texture::DiffuseDirectMap);
		m_lightMapInfo.lightMapIndirectTexture = lightMapC.GetTexture(pragma::CLightMapComponent::Texture::DiffuseIndirectMap);
		m_lightMapInfo.lightMapDominantDirectionTexture = lightMapC.GetTexture(pragma::CLightMapComponent::Texture::DominantDirectionMap);
	}
	m_lightMapInfo.lightMapComponent = lightMapC.GetHandle<CLightMapComponent>();
	if(m_lightMapInfo.cbExposure.IsValid())
		m_lightMapInfo.cbExposure.Remove();
	m_lightMapInfo.cbExposure = lightMapC.GetLightMapExposureProperty()->AddCallback([this, &lightMapC](std::reference_wrapper<const float> oldValue, std::reference_wrapper<const float> newValue) { m_rendererData.lightmapExposurePow = lightMapC.CalcLightMapPowExposurePow(); });
	if(m_lightMapInfo.lightMapTexture == nullptr) {
		CLightMapComponent::LOGGER.warn("Lightmap component has no light map texture!");
		return;
	}
	auto &ds = *m_descSetGroupRenderer->GetDescriptorSet();

	auto &dummyTex = c_engine->GetRenderContext().GetDummyTexture();
	auto getTex = [&dummyTex](const std::shared_ptr<prosper::Texture> &tex) { return tex ? tex : dummyTex; };
	ds.SetBindingTexture(*getTex(m_lightMapInfo.lightMapTexture), umath::to_integral(pragma::ShaderScene::RendererBinding::LightMapDiffuse));
	ds.SetBindingTexture(*getTex(m_lightMapInfo.lightMapIndirectTexture), umath::to_integral(pragma::ShaderScene::RendererBinding::LightMapDiffuseIndirect));
	ds.SetBindingTexture(*getTex(m_lightMapInfo.lightMapDominantDirectionTexture), umath::to_integral(pragma::ShaderScene::RendererBinding::LightMapDominantDirection));
}
const pragma::ComponentHandle<pragma::CLightMapComponent> &CRasterizationRendererComponent::GetLightMap() const { return m_lightMapInfo.lightMapComponent; }
bool CRasterizationRendererComponent::HasIndirectLightmap() const { return m_lightMapInfo.lightMapIndirectTexture != nullptr; }
bool CRasterizationRendererComponent::HasDirectionalLightmap() const { return m_lightMapInfo.lightMapDominantDirectionTexture != nullptr; }

void CRasterizationRendererComponent::ReloadPresentationRenderTarget() { m_hdrInfo.ReloadPresentationRenderTarget(GetWidth(), GetHeight(), GetSampleCount()); }

prosper::SampleCountFlags CRasterizationRendererComponent::GetSampleCount() const { return const_cast<CRasterizationRendererComponent *>(this)->GetHDRInfo().sceneRenderTarget->GetTexture().GetImage().GetSampleCount(); }
bool CRasterizationRendererComponent::IsMultiSampled() const { return GetSampleCount() != prosper::SampleCountFlags::e1Bit; }

prosper::RenderTarget *CRasterizationRendererComponent::GetPrepassRenderTarget(const util::DrawSceneInfo &drawSceneInfo) { return GetPrepass().renderTarget.get(); }
prosper::RenderTarget *CRasterizationRendererComponent::GetLightingPassRenderTarget(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &hdrInfo = GetHDRInfo();
	auto &rt = hdrInfo.GetRenderTarget(drawSceneInfo);
	return &rt;
}

prosper::RenderTarget *CRasterizationRendererComponent::BeginRenderPass(const util::DrawSceneInfo &drawSceneInfo, prosper::IRenderPass *customRenderPass, bool secondaryCommandBuffers)
{
	auto *rt = GetLightingPassRenderTarget(drawSceneInfo);
	if(rt == nullptr)
		return nullptr;
	auto &tex = rt->GetTexture();
	if(tex.IsMSAATexture())
		static_cast<prosper::MSAATexture &>(tex).Reset();
	auto result = GetHDRInfo().BeginRenderPass(drawSceneInfo, customRenderPass, secondaryCommandBuffers);
	return result ? rt : nullptr;
	;
}
bool CRasterizationRendererComponent::EndRenderPass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &hdrInfo = GetHDRInfo();
	return hdrInfo.EndRenderPass(drawSceneInfo);
}
bool CRasterizationRendererComponent::ResolveRenderPass(const util::DrawSceneInfo &drawSceneInfo)
{
	auto &hdrInfo = GetHDRInfo();
	return hdrInfo.ResolveRenderPass(drawSceneInfo);
}

prosper::Shader *CRasterizationRendererComponent::GetWireframeShader() const { return m_whShaderWireframe.get(); }

////////

CELightingStageData::CELightingStageData(pragma::rendering::LightingStageRenderProcessor &renderProcessor) : renderProcessor {renderProcessor} {}
void CELightingStageData::PushArguments(lua_State *l) {}

////////

CEPrepassStageData::CEPrepassStageData(pragma::rendering::DepthStageRenderProcessor &renderProcessor, pragma::ShaderPrepassBase &shader) : renderProcessor {renderProcessor}, shader {shader} {}
void CEPrepassStageData::PushArguments(lua_State *l) {}

////////

void CRasterizationRenderer::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CRasterizationRendererComponent>();
}

static void cl_render_ssao_callback(NetworkState *, const ConVar &, bool, bool enabled)
{
	if(c_game == nullptr)
		return;
	auto &gameWorldShaderSettings = client->GetGameWorldShaderSettings();
	if(gameWorldShaderSettings.ssaoEnabled == enabled)
		return;
	gameWorldShaderSettings.ssaoEnabled = enabled;
	auto &context = c_engine->GetRenderContext();
	context.WaitIdle();
	for(auto &c : EntityCIterator<CRasterizationRendererComponent> {*c_game})
		c.SetSSAOEnabled(enabled);
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_ssao, cl_render_ssao_callback);
