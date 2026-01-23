// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.rasterization_renderer;
import :client_state;
import :engine;
import :entities.components.lights.directional;
import :entities.components.lights.shadow;
import :entities.components.renderer;
import :game;
import :rendering.shaders;

using namespace pragma;

ComponentEventId cRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_ON_RECORD_LIGHTING_PASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_PRE_EXECUTE_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_POST_EXECUTE_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_PRE_EXECUTE_LIGHTING_PASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_POST_EXECUTE_LIGHTING_PASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_PRE_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_POST_PREPASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_PRE_LIGHTING_PASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_POST_LIGHTING_PASS = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_UPDATE_RENDER_BUFFERS = INVALID_COMPONENT_ID;

ComponentEventId cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_SKYBOX = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_END_RECORD_SKYBOX = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_WORLD = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_END_RECORD_WORLD = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_PARTICLES = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_END_RECORD_PARTICLES = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_DEBUG = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_END_RECORD_DEBUG = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_WATER = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_END_RECORD_WATER = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_VIEW = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_END_RECORD_VIEW = INVALID_COMPONENT_ID;
ComponentEventId cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_PREPASS = INVALID_COMPONENT_ID;
void CRasterizationRendererComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	cRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS = registerEvent("ON_RECORD_PREPASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_ON_RECORD_LIGHTING_PASS = registerEvent("ON_RECORD_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_PRE_EXECUTE_PREPASS = registerEvent("PRE_EXECUTE_PREPASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_POST_EXECUTE_PREPASS = registerEvent("POST_EXECUTE_PREPASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_PRE_EXECUTE_LIGHTING_PASS = registerEvent("PRE_EXECUTE_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_POST_EXECUTE_LIGHTING_PASS = registerEvent("POST_EXECUTE_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_PRE_PREPASS = registerEvent("PRE_PREPASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_POST_PREPASS = registerEvent("POST_PREPASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_PRE_LIGHTING_PASS = registerEvent("PRE_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_POST_LIGHTING_PASS = registerEvent("POST_LIGHTING_PASS", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_UPDATE_RENDER_BUFFERS = registerEvent("UPDATE_RENDER_BUFFERS", ComponentEventInfo::Type::Explicit);

	cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_SKYBOX = registerEvent("MT_BEGIN_RECORD_SKYBOX", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_END_RECORD_SKYBOX = registerEvent("MT_END_RECORD_SKYBOX", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_WORLD = registerEvent("MT_BEGIN_RECORD_WORLD", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_END_RECORD_WORLD = registerEvent("MT_END_RECORD_WORLD", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_PARTICLES = registerEvent("MT_BEGIN_RECORD_PARTICLES", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_END_RECORD_PARTICLES = registerEvent("MT_END_RECORD_PARTICLES", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_DEBUG = registerEvent("MT_BEGIN_RECORD_DEBUG", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_END_RECORD_DEBUG = registerEvent("MT_END_RECORD_DEBUG", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_WATER = registerEvent("MT_BEGIN_RECORD_WATER", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_END_RECORD_WATER = registerEvent("MT_END_RECORD_WATER", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_VIEW = registerEvent("MT_BEGIN_RECORD_VIEW", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_END_RECORD_VIEW = registerEvent("MT_END_RECORD_VIEW", ComponentEventInfo::Type::Explicit);
	cRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_PREPASS = registerEvent("MT_BEGIN_RECORD_PREPASS", ComponentEventInfo::Type::Explicit);
}

void CRasterizationRendererComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

static ComponentHandle<CLightMapComponent> g_lightmapC = {};
void CRasterizationRendererComponent::UpdateLightmap(CLightMapComponent &lightMapC)
{
	if(!lightMapC.HasValidLightMap()) {
		CLightMapComponent::LOGGER.warn("Lightmap has no valid lightmap texture!");
		return;
	}
	for(auto &renderer : EntityCIterator<CRasterizationRendererComponent> {*get_cgame()})
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

CRasterizationRendererComponent::CRasterizationRendererComponent(ecs::BaseEntity &ent) : BaseEntityComponent {ent}, m_hdrInfo {*this}, m_stateFlags {StateFlags::PrepassEnabled | StateFlags::InitialRender}
{
	m_whShaderWireframe = get_cengine()->GetShader("wireframe");

	InitializeCommandBufferGroups();

	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.size = sizeof(m_rendererData);
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	m_rendererBuffer = get_cengine()->GetRenderContext().CreateBuffer(bufCreateInfo, &m_rendererData);

	auto *shaderPbr = static_cast<ShaderPBR *>(get_cengine()->GetShader("pbr").get());
	assert(shaderPbr);
	if(shaderPbr) {
		m_descSetGroupRenderer = shaderPbr->CreateDescriptorSetGroup(ShaderPBR::DESCRIPTOR_SET_RENDERER.setIndex);
		m_descSetGroupRenderer->GetDescriptorSet()->SetBindingUniformBuffer(*m_rendererBuffer, math::to_integral(ShaderScene::RendererBinding::Renderer));
	}

	InitializeLightDescriptorSets();
}

CRasterizationRendererComponent::~CRasterizationRendererComponent()
{
	auto &renderContext = get_cengine()->GetRenderContext();
	renderContext.KeepResourceAliveUntilPresentationComplete(m_prepassCommandBufferGroup);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_shadowCommandBufferGroup);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_lightingCommandBufferGroup);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_descSetGroupFogOverride);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_dsgLightsCompute);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_rendererBuffer);
	renderContext.KeepResourceAliveUntilPresentationComplete(m_descSetGroupRenderer);
}

void CRasterizationRendererComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(cRendererComponent::EVENT_RELOAD_RENDER_TARGET, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &reloadRenderTargetEv = static_cast<CEReloadRenderTarget &>(evData.get());
		reloadRenderTargetEv.resultSuccess = ReloadRenderTarget(reloadRenderTargetEv.width, reloadRenderTargetEv.height);
	});
	BindEventUnhandled(cRendererComponent::EVENT_RELOAD_BLOOM_RENDER_TARGET, [this](std::reference_wrapper<ComponentEvent> evData) { m_hdrInfo.ReloadBloomRenderTarget(static_cast<CEReloadBloomRenderTarget &>(evData.get()).width); });
	BindEventUnhandled(cRendererComponent::EVENT_BEGIN_RENDERING, [this](std::reference_wrapper<ComponentEvent> evData) { BeginRendering(static_cast<CEBeginRendering &>(evData.get()).drawSceneInfo); });
	BindEventUnhandled(cRendererComponent::EVENT_END_RENDERING, [this](std::reference_wrapper<ComponentEvent> evData) { EndRendering(); });
	BindEventUnhandled(cRendererComponent::EVENT_UPDATE_CAMERA_DATA, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateFrustumPlanes(static_cast<CEUpdateCameraData &>(evData.get()).scene); });
	BindEvent(cRendererComponent::EVENT_GET_SCENE_TEXTURE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		static_cast<CEGetSceneTexture &>(evData.get()).resultTexture = m_hdrInfo.sceneRenderTarget ? &m_hdrInfo.sceneRenderTarget->GetTexture() : nullptr;
		return util::EventReply::Handled;
	});
	BindEvent(cRendererComponent::EVENT_GET_PRESENTATION_TEXTURE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		static_cast<CEGetPresentationTexture &>(evData.get()).resultTexture = m_hdrInfo.toneMappedRenderTarget ? &m_hdrInfo.toneMappedRenderTarget->GetTexture() : nullptr;
		return util::EventReply::Handled;
	});
	BindEvent(cRendererComponent::EVENT_GET_HDR_PRESENTATION_TEXTURE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		static_cast<CEGetHdrPresentationTexture &>(evData.get()).resultTexture = m_hdrInfo.sceneRenderTarget ? &m_hdrInfo.sceneRenderTarget->GetTexture() : nullptr;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(cRendererComponent::EVENT_RECORD_COMMAND_BUFFERS, [this](std::reference_wrapper<ComponentEvent> evData) { RecordCommandBuffers(static_cast<CEDrawSceneInfo &>(evData.get()).drawSceneInfo); });
	BindEventUnhandled(cRendererComponent::EVENT_RENDER, [this](std::reference_wrapper<ComponentEvent> evData) { Render(static_cast<CERender &>(evData.get()).drawSceneInfo); });
	BindEventUnhandled(cRendererComponent::EVENT_UPDATE_RENDER_SETTINGS, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateRenderSettings(); });
	BindEventUnhandled(cRendererComponent::EVENT_UPDATE_RENDERER_BUFFER, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateRendererBuffer(static_cast<CEUpdateRendererBuffer &>(evData.get()).drawCommandBuffer); });

	auto &ent = GetEntity();
	m_rendererComponent = ent.AddComponent<CRendererComponent>().get();

	UpdateLightmap();
}

void CRasterizationRendererComponent::UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) { drawCmd->RecordUpdateBuffer(*m_rendererBuffer, 0ull, m_rendererData); }

prosper::IDescriptorSet *CRasterizationRendererComponent::GetDepthDescriptorSet() const { return (m_hdrInfo.dsgSceneDepth != nullptr) ? m_hdrInfo.dsgSceneDepth->GetDescriptorSet() : nullptr; }
prosper::IDescriptorSet *CRasterizationRendererComponent::GetRendererDescriptorSet() const { return m_descSetGroupRenderer->GetDescriptorSet(); }

prosper::IDescriptorSet *CRasterizationRendererComponent::GetLightSourceDescriptorSetCompute() const { return m_dsgLightsCompute->GetDescriptorSet(); }

void CRasterizationRendererComponent::InitializeCommandBufferGroups()
{
	auto &context = get_cengine()->GetRenderContext();
	auto &window = context.GetWindow();
	std::string dbgPrefix;
	auto uuid = GetEntity().GetUuid();
	dbgPrefix = util::uuid_to_string(uuid) + "_";
	m_prepassCommandBufferGroup = context.CreateSwapCommandBufferGroup(window, true, dbgPrefix + "prepass");
	m_shadowCommandBufferGroup = context.CreateSwapCommandBufferGroup(window, true, dbgPrefix + "shadow");
	m_lightingCommandBufferGroup = context.CreateSwapCommandBufferGroup(window, true, dbgPrefix + "lighting");
}

void CRasterizationRendererComponent::InitializeLightDescriptorSets()
{
	if(ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDERER.IsValid()) {
		auto &bufLightSources = CLightComponent::GetGlobalRenderBuffer();
		auto &bufShadowData = CLightComponent::GetGlobalShadowBuffer();
		auto &ds = *GetRendererDescriptorSet();
#if USE_LIGHT_SOURCE_UNIFORM_BUFFER == 1
		ds.SetBindingUniformBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufLightSources), math::to_integral(ShaderGameWorldLightingPass::RendererBinding::LightBuffers));
		ds.SetBindingUniformBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufShadowData), math::to_integral(ShaderGameWorldLightingPass::RendererBinding::ShadowData));
#else
		ds.SetBindingStorageBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufLightSources), pragma::math::to_integral(pragma::ShaderGameWorldLightingPass::RendererBinding::LightBuffers));
		ds.SetBindingStorageBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufShadowData), pragma::math::to_integral(pragma::ShaderGameWorldLightingPass::RendererBinding::ShadowData));
#endif

		m_dsgLightsCompute = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS);
#if USE_LIGHT_SOURCE_UNIFORM_BUFFER == 1
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingUniformBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufLightSources), math::to_integral(ShaderForwardPLightCulling::LightBinding::LightBuffers));
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingUniformBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufShadowData), math::to_integral(ShaderForwardPLightCulling::LightBinding::ShadowData));
#else
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingStorageBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufLightSources), pragma::math::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::LightBuffers));
		m_dsgLightsCompute->GetDescriptorSet()->SetBindingStorageBuffer(const_cast<prosper::IUniformResizableBuffer &>(bufShadowData), pragma::math::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::ShadowData));
#endif
	}
}

void CRasterizationRendererComponent::UpdateCSMDescriptorSet(BaseEnvLightDirectionalComponent &lightSource)
{
	auto *dsLights = GetRendererDescriptorSet();
	if(dsLights == nullptr)
		return;
	auto *pShadowMap = static_cast<CLightDirectionalComponent &>(lightSource).GetShadowMap();
	auto texture = pShadowMap ? pShadowMap->GetDepthTexture() : nullptr;
	if(texture == nullptr)
		return;
	auto numLayers = pShadowMap->GetLayerCount();
	for(auto i = decltype(numLayers) {0}; i < numLayers; ++i) {
		dsLights->SetBindingArrayTexture(*texture, math::to_integral(ShaderSceneLit::RendererBinding::CSM), i, i);
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
	//descSetHdrResolve.SetBindingTexture(*resolvedGlowTex,pragma::math::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));

	if(bSsao == true) {
		auto &ssaoInfo = GetSSAOInfo();
		auto *ssaoBlurTexResolved = &ssaoInfo.renderTargetBlur->GetTexture();
		if(ssaoBlurTexResolved->IsMSAATexture())
			ssaoBlurTexResolved = static_cast<prosper::MSAATexture *>(ssaoBlurTexResolved)->GetResolvedTexture().get();

		auto &ds = *m_descSetGroupRenderer->GetDescriptorSet();
		ds.SetBindingTexture(*ssaoBlurTexResolved, math::to_integral(ShaderScene::RendererBinding::SSAOMap));
	}
	auto &dummyTex = get_cengine()->GetRenderContext().GetDummyTexture();
	auto &ds = *m_descSetGroupRenderer->GetDescriptorSet();
	ds.SetBindingTexture(*dummyTex, math::to_integral(ShaderScene::RendererBinding::LightMapDiffuse));
	ds.SetBindingTexture(*dummyTex, math::to_integral(ShaderScene::RendererBinding::LightMapDiffuseIndirect));
	ds.SetBindingTexture(*dummyTex, math::to_integral(ShaderScene::RendererBinding::LightMapDominantDirection));

	m_lightMapInfo.lightMapTexture = nullptr;
	if(g_lightmapC.expired() == false)
		SetLightMap(*g_lightmapC);

	UpdateRenderSettings();
	return true;
}
void CRasterizationRendererComponent::SetFrameDepthBufferSamplingRequired() { m_bFrameDepthBufferSamplingRequired = true; }
void CRasterizationRendererComponent::EndRendering() {}
void CRasterizationRendererComponent::BeginRendering(const rendering::DrawSceneInfo &drawSceneInfo) { math::set_flag(m_stateFlags, StateFlags::DepthResolved | StateFlags::BloomResolved | StateFlags::RenderResolved, false); }

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

bool CRasterizationRendererComponent::IsSSAOEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::SSAOEnabled) && get_client_state()->GetGameWorldShaderSettings().ssaoEnabled; }
void CRasterizationRendererComponent::SetSSAOEnabled(bool b)
{
	math::set_flag(m_stateFlags, StateFlags::SSAOEnabled, b);
	UpdateRenderSettings();
	if(GetWidth() > 0 && GetHeight() > 0)
		ReloadRenderTarget(GetWidth(), GetHeight());
	/*m_hdrInfo.prepass.SetUseExtendedPrepass(b);
	if(b == true)
	{
	auto &context = pragma::get_cengine()->GetRenderContext();
	m_hdrInfo.ssaoInfo.Initialize(context,GetWidth(),GetHeight(),static_cast<Anvil::SampleCountFlagBits>(pragma::get_cgame()->GetMSAASampleCount()),m_hdrInfo.prepass.texturePositions,m_hdrInfo.prepass.textureNormals,m_hdrInfo.prepass.textureDepth);
	}
	else
	m_hdrInfo.ssaoInfo.Clear();
	UpdateRenderSettings();*/
}
uint32_t CRasterizationRendererComponent::GetWidth() const { return m_rendererComponent ? static_cast<CRendererComponent *>(m_rendererComponent)->GetWidth() : 0; }
uint32_t CRasterizationRendererComponent::GetHeight() const { return m_rendererComponent ? static_cast<CRendererComponent *>(m_rendererComponent)->GetHeight() : 0; }
void CRasterizationRendererComponent::UpdateRenderSettings()
{
	auto &tileInfo = m_rendererData.tileInfo;
	tileInfo = static_cast<uint32_t>(ShaderForwardPLightCulling::TILE_SIZE) << 16;
	tileInfo |= static_cast<uint32_t>(rendering::ForwardPlusInstance::CalcWorkGroupCount(GetWidth(), GetHeight()).first);
	m_rendererData.SetResolution(GetWidth(), GetHeight());

	if(IsSSAOEnabled() == true)
		m_rendererData.flags |= RendererData::Flags::SSAOEnabled;
}
void CRasterizationRendererComponent::SetShaderOverride(const std::string &srcShaderId, const std::string &shaderOverrideId)
{
	auto hSrcShader = get_cengine()->GetShader(srcShaderId);
	if(hSrcShader.get()->GetBaseTypeHashCode() != ShaderGameWorldLightingPass::HASH_TYPE)
		return;
	auto *srcShader = dynamic_cast<ShaderGameWorldLightingPass *>(hSrcShader.get());
	if(srcShader == nullptr)
		return;
	auto hDstShader = get_cengine()->GetShader(shaderOverrideId);
	auto dstShader = dynamic_cast<ShaderGameWorldLightingPass *>(hDstShader.get());
	if(dstShader == nullptr)
		return;
	m_shaderOverrides[typeid(*srcShader).hash_code()] = dstShader->GetHandle();
}
ShaderGameWorldLightingPass *CRasterizationRendererComponent::GetShaderOverride(ShaderGameWorldLightingPass *srcShader) const
{
	if(srcShader == nullptr)
		return nullptr;
	auto it = m_shaderOverrides.find(typeid(*srcShader).hash_code());
	if(it == m_shaderOverrides.end())
		return srcShader;
	return static_cast<ShaderGameWorldLightingPass *>(it->second.get());
}
void CRasterizationRendererComponent::ClearShaderOverride(const std::string &srcShaderId)
{
	auto hSrcShader = get_cengine()->GetShader(srcShaderId);
	auto *srcShader = dynamic_cast<ShaderGameWorldLightingPass *>(hSrcShader.get());
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
		math::set_flag(m_stateFlags, StateFlags::PrepassEnabled, false);
		break;
	case PrepassMode::DepthOnly:
		math::set_flag(m_stateFlags, StateFlags::PrepassEnabled, true);
		prepass.SetUseExtendedPrepass(false);
		break;
	case PrepassMode::Extended:
		math::set_flag(m_stateFlags, StateFlags::PrepassEnabled, true);
		prepass.SetUseExtendedPrepass(true);
		break;
	}
}
CRasterizationRendererComponent::PrepassMode CRasterizationRendererComponent::GetPrepassMode() const
{
	if(math::is_flag_set(m_stateFlags, StateFlags::PrepassEnabled) == false)
		return PrepassMode::NoPrepass;
	auto &prepass = const_cast<CRasterizationRendererComponent *>(this)->GetPrepass();
	return prepass.IsExtended() ? PrepassMode::Extended : PrepassMode::DepthOnly;
}

ShaderPrepassBase &CRasterizationRendererComponent::GetPrepassShader() const { return const_cast<CRasterizationRendererComponent *>(this)->GetPrepass().GetShader(); }

rendering::HDRData &CRasterizationRendererComponent::GetHDRInfo() { return m_hdrInfo; }
const rendering::HDRData &CRasterizationRendererComponent::GetHDRInfo() const { return const_cast<CRasterizationRendererComponent *>(this)->GetHDRInfo(); }
// GlowData &CRasterizationRendererComponent::GetGlowInfo() {return m_glowInfo;}
rendering::SSAOInfo &CRasterizationRendererComponent::GetSSAOInfo() { return m_hdrInfo.ssaoInfo; }
rendering::Prepass &CRasterizationRendererComponent::GetPrepass() { return m_hdrInfo.prepass; }
const rendering::ForwardPlusInstance &CRasterizationRendererComponent::GetForwardPlusInstance() const { return const_cast<CRasterizationRendererComponent *>(this)->GetForwardPlusInstance(); }
rendering::ForwardPlusInstance &CRasterizationRendererComponent::GetForwardPlusInstance() { return m_hdrInfo.forwardPlusInstance; }

void CRasterizationRendererComponent::SetBloomThreshold(float threshold) { m_rendererData.bloomThreshold = threshold; }
float CRasterizationRendererComponent::GetBloomThreshold() const { return m_rendererData.bloomThreshold; }

Float CRasterizationRendererComponent::GetHDRExposure() const { return m_hdrInfo.exposure; }
Float CRasterizationRendererComponent::GetMaxHDRExposure() const { return m_hdrInfo.max_exposure; }
void CRasterizationRendererComponent::SetMaxHDRExposure(Float exposure) { m_hdrInfo.max_exposure = exposure; }

const std::vector<math::Plane> &CRasterizationRendererComponent::GetFrustumPlanes() const { return m_frustumPlanes; }
const std::vector<math::Plane> &CRasterizationRendererComponent::GetClippedFrustumPlanes() const { return m_clippedFrustumPlanes; }

void CRasterizationRendererComponent::UpdateFrustumPlanes(CSceneComponent &scene)
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
	camera->SetForward(uvec::PRM_FORWARD);
	camera->SetUp(uvec::PRM_UP);
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

void CRasterizationRendererComponent::SetLightMap(CLightMapComponent &lightMapC)
{
	m_rendererData.lightmapExposurePow = lightMapC.CalcLightMapPowExposurePow();
	/*if(
		m_lightMapInfo.lightMapComponent.get() == &lightMapC &&
		lightMapC.GetLightMap() == m_lightMapInfo.lightMapTexture &&
		lightMapC.GetDirectionalLightMap() == m_lightMapInfo.directionalLightMapTexture
	)
		return;*/
	m_lightMapInfo.lightMapTexture = lightMapC.GetTexture(CLightMapComponent::Texture::DiffuseMap);
	if(!m_lightMapInfo.lightMapTexture) {
		m_lightMapInfo.lightMapTexture = lightMapC.GetTexture(CLightMapComponent::Texture::DiffuseDirectMap);
		m_lightMapInfo.lightMapIndirectTexture = lightMapC.GetTexture(CLightMapComponent::Texture::DiffuseIndirectMap);
		m_lightMapInfo.lightMapDominantDirectionTexture = lightMapC.GetTexture(CLightMapComponent::Texture::DominantDirectionMap);
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

	auto &dummyTex = get_cengine()->GetRenderContext().GetDummyTexture();
	auto getTex = [&dummyTex](const std::shared_ptr<prosper::Texture> &tex) { return tex ? tex : dummyTex; };
	ds.SetBindingTexture(*getTex(m_lightMapInfo.lightMapTexture), math::to_integral(ShaderScene::RendererBinding::LightMapDiffuse));
	ds.SetBindingTexture(*getTex(m_lightMapInfo.lightMapIndirectTexture), math::to_integral(ShaderScene::RendererBinding::LightMapDiffuseIndirect));
	ds.SetBindingTexture(*getTex(m_lightMapInfo.lightMapDominantDirectionTexture), math::to_integral(ShaderScene::RendererBinding::LightMapDominantDirection));
}
const ComponentHandle<CLightMapComponent> &CRasterizationRendererComponent::GetLightMap() const { return m_lightMapInfo.lightMapComponent; }
bool CRasterizationRendererComponent::HasIndirectLightmap() const { return m_lightMapInfo.lightMapIndirectTexture != nullptr; }
bool CRasterizationRendererComponent::HasDirectionalLightmap() const { return m_lightMapInfo.lightMapDominantDirectionTexture != nullptr; }

void CRasterizationRendererComponent::ReloadPresentationRenderTarget() { m_hdrInfo.ReloadPresentationRenderTarget(GetWidth(), GetHeight(), GetSampleCount()); }

prosper::SampleCountFlags CRasterizationRendererComponent::GetSampleCount() const { return const_cast<CRasterizationRendererComponent *>(this)->GetHDRInfo().sceneRenderTarget->GetTexture().GetImage().GetSampleCount(); }
bool CRasterizationRendererComponent::IsMultiSampled() const { return GetSampleCount() != prosper::SampleCountFlags::e1Bit; }

prosper::RenderTarget *CRasterizationRendererComponent::GetPrepassRenderTarget(const rendering::DrawSceneInfo &drawSceneInfo) { return GetPrepass().renderTarget.get(); }
prosper::RenderTarget *CRasterizationRendererComponent::GetLightingPassRenderTarget(const rendering::DrawSceneInfo &drawSceneInfo)
{
	auto &hdrInfo = GetHDRInfo();
	auto &rt = hdrInfo.GetRenderTarget(drawSceneInfo);
	return &rt;
}

prosper::RenderTarget *CRasterizationRendererComponent::BeginRenderPass(const rendering::DrawSceneInfo &drawSceneInfo, prosper::IRenderPass *customRenderPass, bool secondaryCommandBuffers)
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
bool CRasterizationRendererComponent::EndRenderPass(const rendering::DrawSceneInfo &drawSceneInfo)
{
	auto &hdrInfo = GetHDRInfo();
	return hdrInfo.EndRenderPass(drawSceneInfo);
}
bool CRasterizationRendererComponent::ResolveRenderPass(const rendering::DrawSceneInfo &drawSceneInfo)
{
	auto &hdrInfo = GetHDRInfo();
	return hdrInfo.ResolveRenderPass(drawSceneInfo);
}

prosper::Shader *CRasterizationRendererComponent::GetWireframeShader() const { return m_whShaderWireframe.get(); }

////////

CELightingStageData::CELightingStageData(rendering::LightingStageRenderProcessor &renderProcessor) : renderProcessor {renderProcessor} {}
void CELightingStageData::PushArguments(lua::State *l) {}

////////

CEPrepassStageData::CEPrepassStageData(rendering::DepthStageRenderProcessor &renderProcessor, ShaderPrepassBase &shader) : renderProcessor {renderProcessor}, shader {shader} {}
void CEPrepassStageData::PushArguments(lua::State *l) {}

////////

CEUpdateRenderBuffers::CEUpdateRenderBuffers(const rendering::DrawSceneInfo &drawSceneInfo) : drawSceneInfo {drawSceneInfo} {}
void CEUpdateRenderBuffers::PushArguments(lua::State *l) {}

////////

void CRasterizationRenderer::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CRasterizationRendererComponent>();
}

static void cl_render_ssao_callback(NetworkState *, const console::ConVar &, bool, bool enabled)
{
	if(get_cgame() == nullptr)
		return;
	auto &gameWorldShaderSettings = get_client_state()->GetGameWorldShaderSettings();
	if(gameWorldShaderSettings.ssaoEnabled == enabled)
		return;
	gameWorldShaderSettings.ssaoEnabled = enabled;
	auto &context = get_cengine()->GetRenderContext();
	context.WaitIdle();
	for(auto &c : EntityCIterator<CRasterizationRendererComponent> {*get_cgame()})
		c.SetSSAOEnabled(enabled);
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>("cl_render_ssao", &cl_render_ssao_callback);
}
