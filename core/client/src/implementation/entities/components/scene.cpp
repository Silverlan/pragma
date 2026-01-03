// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <cstddef>

module pragma.client;

import :entities.components.scene;
import :engine;
import :game;
import :rendering.shaders;

using namespace pragma;

CSceneComponent::CSMCascadeDescriptor::CSMCascadeDescriptor() {}

///////////////////////////

CSceneComponent::CreateInfo::CreateInfo() : sampleCount {static_cast<prosper::SampleCountFlags>(get_cgame()->GetMSAASampleCount())} {}

///////////////////////////

ComponentEventId cSceneComponent::EVENT_ON_ACTIVE_CAMERA_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId cSceneComponent::EVENT_ON_BUILD_RENDER_QUEUES = INVALID_COMPONENT_ID;
ComponentEventId cSceneComponent::EVENT_ON_RENDERER_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId cSceneComponent::EVENT_POST_RENDER_PREPASS = INVALID_COMPONENT_ID;
void CSceneComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	cSceneComponent::EVENT_ON_ACTIVE_CAMERA_CHANGED = registerEvent("ON_ACTIVE_CAMERA_CHANGED", ComponentEventInfo::Type::Broadcast);
	cSceneComponent::EVENT_ON_BUILD_RENDER_QUEUES = registerEvent("ON_BUILD_RENDER_QUEUES", ComponentEventInfo::Type::Explicit);
	cSceneComponent::EVENT_ON_RENDERER_CHANGED = registerEvent("ON_RENDERER_CHANGED", ComponentEventInfo::Type::Broadcast);
	cSceneComponent::EVENT_POST_RENDER_PREPASS = registerEvent("POST_RENDER_PREPASS", ComponentEventInfo::Type::Explicit);
}

static std::shared_ptr<rendering::EntityInstanceIndexBuffer> g_entityInstanceIndexBuffer = nullptr;
const std::shared_ptr<rendering::EntityInstanceIndexBuffer> &CSceneComponent::GetEntityInstanceIndexBuffer() { return g_entityInstanceIndexBuffer; }

void CSceneComponent::UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, const rendering::RenderQueue &renderQueue, rendering::RenderPassStats *optStats)
{
	renderQueue.WaitForCompletion(optStats);
	GetEntityInstanceIndexBuffer()->UpdateBufferData(renderQueue);
	auto curEntity = std::numeric_limits<EntityIndex>::max();
	for(auto &item : renderQueue.queue) {
		if(item.entity == curEntity)
			continue;
		curEntity = item.entity;
		auto *ent = static_cast<ecs::CBaseEntity *>(get_cgame()->GetEntityByLocalIndex(item.entity));
		assert(ent);
		if(!ent) {
			// TODO: This should be unreachable, but there are cases where the entity does
			// end up as nullptr for some unknown reason.
			// This could happen if the entity is removed after the render queues have been built,
			// which is not allowed.
			continue;
		}
		auto *renderC = ent->GetRenderComponent();
		assert(renderC);
		if(!renderC) {
			// TODO: This should be unreachable, but there are cases where the render component does
			// end up as nullptr for some unknown reason.
			// This could happen if the render component is removed after the render queues have been built,
			// which is not allowed.
			continue;
		}
		if(optStats && math::is_flag_set(renderC->GetStateFlags(), CRenderComponent::StateFlags::RenderBufferDirty))
			(*optStats)->Increment(rendering::RenderPassStats::Counter::EntityBufferUpdates);
		auto *animC = renderC->GetAnimatedComponent();
		if(animC && animC->AreSkeletonUpdateCallbacksEnabled())
			animC->UpdateBoneMatricesMT();
		renderC->UpdateRenderBuffers(drawCmd);
	}
}

using SceneCount = uint32_t;
static std::array<SceneCount, 32> g_sceneUseCount {};
static std::array<CSceneComponent *, 32> g_scenes {};

CSceneComponent *CSceneComponent::Create(const CreateInfo &createInfo, CSceneComponent *optParent)
{
	SceneIndex sceneIndex;
	if(optParent)
		sceneIndex = optParent->GetSceneIndex();
	else {
		// Find unused slot
		auto it = std::find(g_sceneUseCount.begin(), g_sceneUseCount.end(), 0);
		if(it == g_sceneUseCount.end())
			return nullptr;
		sceneIndex = (it - g_sceneUseCount.begin());
	}
	auto *scene = get_cgame()->CreateEntity<CScene>();
	if(scene == nullptr)
		return nullptr;
	auto sceneC = scene->GetComponent<CSceneComponent>();
	if(sceneC.expired()) {
		scene->Remove();
		return nullptr;
	}
	++g_sceneUseCount.at(sceneIndex);
	sceneC->Setup(createInfo, sceneIndex);
	if(optParent == nullptr)
		g_scenes.at(sceneIndex) = sceneC.get();
	else
		math::set_flag(sceneC->m_stateFlags, StateFlags::HasParentScene);

	scene->Spawn();
	return sceneC.get();
}

static uint32_t g_numScenes = 0;
CSceneComponent::CSceneComponent(ecs::BaseEntity &ent) : BaseEntityComponent {ent}, m_sceneRenderDesc {*this} { ++g_numScenes; }
CSceneComponent::~CSceneComponent()
{
	ClearWorldEnvironment();
	//pragma::get_cengine()->FlushCommandBuffers(); // We need to make sure all rendering commands have been completed, in case this scene is still in use somewhere // prosper TODO
}
void CSceneComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_cbLink.IsValid())
		m_cbLink.Remove();

	auto sceneIndex = GetSceneIndex();
	if(sceneIndex == std::numeric_limits<SceneIndex>::max())
		return;
	assert(g_sceneUseCount.size() > 0);
	--g_sceneUseCount.at(sceneIndex);
	if(math::is_flag_set(m_stateFlags, StateFlags::HasParentScene) == false) {
		g_scenes.at(sceneIndex) = nullptr;

		// Clear all entities from this scene
		std::vector<ecs::CBaseEntity *> *ents;
		get_cgame()->GetEntities(&ents);
		for(auto *ent : *ents) {
			if(ent == nullptr)
				continue;
			ent->RemoveFromScene(*this);
		}
	}

	if(--g_numScenes == 0)
		g_entityInstanceIndexBuffer = nullptr;
}
void CSceneComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CSceneComponent::Initialize() { BaseEntityComponent::Initialize(); }

void CSceneComponent::Setup(const CreateInfo &createInfo, SceneIndex sceneIndex)
{
	m_sceneIndex = sceneIndex;
	for(auto i = decltype(CShadowCSMComponent::MAX_CASCADE_COUNT) {0}; i < CShadowCSMComponent::MAX_CASCADE_COUNT; ++i)
		m_csmDescriptors.push_back(std::unique_ptr<CSMCascadeDescriptor>(new CSMCascadeDescriptor()));
	InitializeCameraBuffer();
	InitializeFogBuffer();
	InitializeDescriptorSetLayouts();

	InitializeRenderSettingsBuffer();
	InitializeSwapDescriptorBuffers();
	InitializeShadowDescriptorSet();

	if(g_entityInstanceIndexBuffer == nullptr)
		g_entityInstanceIndexBuffer = pragma::util::make_shared<rendering::EntityInstanceIndexBuffer>();
}

void CSceneComponent::Link(const CSceneComponent &other, bool linkCamera)
{
	auto &hCam = other.GetActiveCamera();
	if(hCam.valid())
		SetActiveCamera(const_cast<CCameraComponent &>(*hCam.get()));
	else
		SetActiveCamera();

	auto *renderer = const_cast<CSceneComponent &>(other).GetRenderer<CRendererComponent>();
	SetRenderer(renderer);

	// m_sceneRenderDesc.SetOcclusionCullingHandler(const_cast<pragma::OcclusionCullingHandler&>(other.m_sceneRenderDesc.GetOcclusionCullingHandler()).shared_from_this());

	auto *occlusionCuller = const_cast<CSceneComponent &>(other).FindOcclusionCuller<COcclusionCullerComponent>();
	if(occlusionCuller)
		static_cast<ecs::CBaseEntity &>(occlusionCuller->GetEntity()).AddToScene(*this);

	auto *worldEnv = other.GetWorldEnvironment();
	if(worldEnv)
		SetWorldEnvironment(*worldEnv);

	if(m_cbLink.IsValid())
		m_cbLink.Remove();
	if(linkCamera == false)
		return;
	m_cbLink = const_cast<CSceneComponent &>(other).AddEventCallback(cSceneComponent::EVENT_ON_ACTIVE_CAMERA_CHANGED, [this, &other](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &hCam = other.GetActiveCamera();
		if(hCam.valid())
			SetActiveCamera(const_cast<CCameraComponent &>(*hCam.get()));
		else
			SetActiveCamera();
		return util::EventReply::Unhandled;
	});
}

void CSceneComponent::InitializeShadowDescriptorSet()
{
	if(ShaderGameWorldLightingPass::DESCRIPTOR_SET_SHADOWS.IsValid()) {
		auto &context = get_cengine()->GetRenderContext();
		m_shadowDsg = context.CreateDescriptorSetGroup(ShaderGameWorldLightingPass::DESCRIPTOR_SET_SHADOWS);
		auto &cubeTex = context.GetDummyCubemapTexture();
		auto n = math::to_integral(GameLimits::MaxActiveShadowCubeMaps);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			m_shadowDsg->GetDescriptorSet()->SetBindingArrayTexture(*cubeTex, math::to_integral(ShaderSceneLit::ShadowBinding::ShadowCubeMaps), i);
	}
}

CSceneComponent *CSceneComponent::GetByIndex(SceneIndex sceneIndex) { return (sceneIndex < g_scenes.size()) ? g_scenes.at(sceneIndex) : nullptr; }

uint32_t CSceneComponent::GetSceneFlag(SceneIndex sceneIndex) { return 1 << sceneIndex; }

CSceneComponent::SceneIndex CSceneComponent::GetSceneIndex(uint32_t flag) { return math::get_least_significant_set_bit_index(flag); }

static auto cvShadowmapSize = console::get_client_con_var("cl_render_shadow_resolution");
static auto cvShaderQuality = console::get_client_con_var("cl_render_shader_quality");
void CSceneComponent::InitializeRenderSettingsBuffer()
{
	// Initialize Render Settings
	auto szShadowMap = cvShadowmapSize->GetFloat();
	auto w = GetWidth();
	auto h = GetHeight();

	m_renderSettings.posCam = Vector3(0.f, 0.f, 0.f);
	m_renderSettings.fov = 0.f;
	m_renderSettings.flags = math::to_integral(FRenderSetting::None);
	m_renderSettings.shadowRatioX = 1.f / szShadowMap;
	m_renderSettings.shadowRatioY = 1.f / szShadowMap;
	m_renderSettings.shaderQuality = cvShaderQuality->GetInt();

	if(m_renderer.valid())
		static_cast<CRendererComponent *>(m_renderer.get())->UpdateRenderSettings();

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(m_renderSettings);
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	m_renderSettingsBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo, &m_renderSettings);
	m_renderSettingsBuffer->SetDebugName("render_settings_buf");
	UpdateRenderSettings();
	//
}
void CSceneComponent::InitializeCameraBuffer()
{
	// Camera
	m_cameraData.P = umat::identity();
	m_cameraData.V = umat::identity();
	m_cameraData.VP = umat::identity();

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(m_cameraData);
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	m_cameraBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo, &m_cameraData);
	m_cameraBuffer->SetDebugName("camera_buf");
	//

	// View Camera
	m_cameraViewBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo, &m_cameraData);
	m_cameraViewBuffer->SetDebugName("camera_view_buf");
	//
}
void CSceneComponent::InitializeFogBuffer()
{
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(m_fogData);
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	m_fogBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo, &m_cameraData);
	m_fogBuffer->SetDebugName("fog_buf");
}
template<typename TCPPM>
TCPPM *CSceneComponent::FindOcclusionCuller()
{
	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<COcclusionCullerComponent>>();
	entIt.AttachFilter<EntityIteratorFilterUser>([this](ecs::BaseEntity &ent, std::size_t index) -> bool { return static_cast<ecs::CBaseEntity &>(ent).IsInScene(*this); });
	auto it = entIt.begin();
	auto *ent = (it != entIt.end()) ? *it : nullptr;
	return ent ? ent->GetComponent<COcclusionCullerComponent>().get() : nullptr;
}
template DLLCLIENT COcclusionCullerComponent *CSceneComponent::FindOcclusionCuller();
template<typename TCPPM>
const TCPPM *CSceneComponent::FindOcclusionCuller() const
{
	return const_cast<CSceneComponent *>(this)->FindOcclusionCuller<TCPPM>();
}
template DLLCLIENT const COcclusionCullerComponent *CSceneComponent::FindOcclusionCuller() const;
const std::shared_ptr<prosper::IBuffer> &CSceneComponent::GetFogBuffer() const { return m_fogBuffer; }
void CSceneComponent::UpdateCameraBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, bool bView)
{
	auto &cam = GetActiveCamera();
	if(cam.expired())
		return;
	auto &bufCam = (bView == true) ? GetViewCameraBuffer() : GetCameraBuffer();
	auto &v = cam->GetViewMatrix();
	auto &p = (bView == true) ? CCameraComponent::CalcProjectionMatrix(get_cgame()->GetViewModelFOVRad(), cam->GetAspectRatio(), cam->GetNearZ(), cam->GetFarZ()) : cam->GetProjectionMatrix();
	m_cameraData.V = v;
	m_cameraData.P = p;
	m_cameraData.VP = p * v;

	if(bView == false && m_renderer.valid())
		static_cast<CRendererComponent *>(m_renderer.get())->UpdateCameraData(*this, m_cameraData);

	drawCmd->RecordBufferBarrier(*bufCam, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit | prosper::PipelineStageFlags::ComputeShaderBit, prosper::PipelineStageFlags::TransferBit,
	  prosper::AccessFlags::ShaderReadBit, prosper::AccessFlags::TransferWriteBit);
	drawCmd->RecordUpdateBuffer(*bufCam, 0ull, m_cameraData);
}
void CSceneComponent::UpdateBuffers(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	UpdateCameraBuffer(drawCmd, false);
	UpdateCameraBuffer(drawCmd, true);

	// Update Render Buffer
	auto &cam = GetActiveCamera();
	if(cam.valid()) {
		m_renderSettings.posCam = cam->GetEntity().GetPosition();
		m_renderSettings.fov = cam->GetFOVRad();
	}
	else {
		m_renderSettings.posCam = {};
		m_renderSettings.fov = 0.f;
	}

	drawCmd->RecordBufferBarrier(*m_renderSettingsBuffer, prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit | prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::PipelineStageFlags::TransferBit, prosper::AccessFlags::ShaderReadBit, prosper::AccessFlags::TransferWriteBit);
	drawCmd->RecordUpdateBuffer(*m_renderSettingsBuffer, 0ull, m_renderSettings);
	// prosper TODO: Move camPos to camera buffer, and don't update render settings buffer every frame (update when needed instead)

	if(m_renderer.valid())
		static_cast<CRendererComponent *>(m_renderer.get())->UpdateRendererBuffer(drawCmd);
}
void CSceneComponent::RecordRenderCommandBuffers(const rendering::DrawSceneInfo &drawSceneInfo)
{
	auto *renderer = GetRenderer<CRendererComponent>();
	if(renderer == nullptr)
		return;
	renderer->RecordCommandBuffers(drawSceneInfo);
}
void CSceneComponent::InitializeDescriptorSetLayouts()
{
	/*auto &context = pragma::get_cengine()->GetRenderContext();
	m_descSetLayoutCamGraphics = Vulkan::DescriptorSetLayout::Create(context,{
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit}, // Camera
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit}, // Render Settings
		{prosper::DescriptorType::CombinedImageSampler,prosper::ShaderStageFlags::FragmentBit} // SSAO Map
	});
	m_descSetLayoutCamCompute = Vulkan::DescriptorSetLayout::Create(context,{
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::ComputeBit}, // Camera
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::ComputeBit} // Render Settings
	});*/ // prosper TODO
}
void CSceneComponent::InitializeSwapDescriptorBuffers()
{
	if(ShaderGameWorldLightingPass::DESCRIPTOR_SET_SCENE.IsValid() == false || ShaderPPFog::DESCRIPTOR_SET_FOG.IsValid() == false)
		return;
	m_camDescSetGroupGraphics = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderGameWorldLightingPass::DESCRIPTOR_SET_SCENE);
	auto &descSetGraphics = *m_camDescSetGroupGraphics->GetDescriptorSet();
	descSetGraphics.SetBindingUniformBuffer(*m_cameraBuffer, math::to_integral(ShaderGameWorldLightingPass::SceneBinding::Camera));
	descSetGraphics.SetBindingUniformBuffer(*m_renderSettingsBuffer, math::to_integral(ShaderGameWorldLightingPass::SceneBinding::RenderSettings));

	m_camDescSetGroupCompute = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderForwardPLightCulling::DESCRIPTOR_SET_SCENE);
	auto &descSetCompute = *m_camDescSetGroupCompute->GetDescriptorSet();
	descSetCompute.SetBindingUniformBuffer(*m_cameraBuffer, math::to_integral(ShaderForwardPLightCulling::CameraBinding::Camera));
	descSetCompute.SetBindingUniformBuffer(*m_renderSettingsBuffer, math::to_integral(ShaderForwardPLightCulling::CameraBinding::RenderSettings));

	m_camViewDescSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderGameWorldLightingPass::DESCRIPTOR_SET_SCENE);
	auto &descSetViewGraphics = *m_camViewDescSetGroup->GetDescriptorSet();
	descSetViewGraphics.SetBindingUniformBuffer(*m_cameraViewBuffer, math::to_integral(ShaderGameWorldLightingPass::SceneBinding::Camera));
	descSetViewGraphics.SetBindingUniformBuffer(*m_renderSettingsBuffer, math::to_integral(ShaderGameWorldLightingPass::SceneBinding::RenderSettings));

	m_fogDescSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderPPFog::DESCRIPTOR_SET_FOG);
	m_fogDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(*m_fogBuffer, 0u);
}
const std::shared_ptr<prosper::IBuffer> &CSceneComponent::GetRenderSettingsBuffer() const { return m_renderSettingsBuffer; }
RenderSettings &CSceneComponent::GetRenderSettings() { return m_renderSettings; }
const RenderSettings &CSceneComponent::GetRenderSettings() const { return const_cast<CSceneComponent *>(this)->GetRenderSettings(); }
const std::shared_ptr<prosper::IBuffer> &CSceneComponent::GetCameraBuffer() const { return m_cameraBuffer; }
const std::shared_ptr<prosper::IBuffer> &CSceneComponent::GetViewCameraBuffer() const { return m_cameraViewBuffer; }
const std::shared_ptr<prosper::IDescriptorSetGroup> &CSceneComponent::GetCameraDescriptorSetGroup(prosper::PipelineBindPoint bindPoint) const
{
	switch(bindPoint) {
	case prosper::PipelineBindPoint::Graphics:
		return m_camDescSetGroupGraphics;
	case prosper::PipelineBindPoint::Compute:
		return m_camDescSetGroupCompute;
	}
	static std::shared_ptr<prosper::IDescriptorSetGroup> nptr = nullptr;
	return nptr;
}
const std::shared_ptr<prosper::IDescriptorSetGroup> &CSceneComponent::GetViewCameraDescriptorSetGroup() const { return m_camViewDescSetGroup; }
prosper::IDescriptorSet *CSceneComponent::GetCameraDescriptorSetGraphics() const { return m_camDescSetGroupGraphics->GetDescriptorSet(); }
prosper::IDescriptorSet *CSceneComponent::GetCameraDescriptorSetCompute() const { return m_camDescSetGroupCompute->GetDescriptorSet(); }
prosper::IDescriptorSet *CSceneComponent::GetViewCameraDescriptorSet() const { return m_camViewDescSetGroup->GetDescriptorSet(); }
const std::shared_ptr<prosper::IDescriptorSetGroup> &CSceneComponent::GetFogDescriptorSetGroup() const { return m_fogDescSetGroup; }

void CSceneComponent::SetExclusionRenderMask(rendering::RenderMask renderMask) { m_exclusionRenderMask = renderMask; }
rendering::RenderMask CSceneComponent::GetExclusionRenderMask() const { return m_exclusionRenderMask; }
void CSceneComponent::SetInclusionRenderMask(rendering::RenderMask renderMask) { m_inclusionRenderMask = renderMask; }
rendering::RenderMask CSceneComponent::GetInclusionRenderMask() const { return m_inclusionRenderMask; }

void CSceneComponent::BuildRenderQueues(const rendering::DrawSceneInfo &drawSceneInfo)
{
	CEDrawSceneInfo evData {drawSceneInfo};
	GetSceneRenderDesc().BuildRenderQueues(drawSceneInfo, [this, &drawSceneInfo, &evData]() {
		// Start building the render queues for the light sources
		// that create shadows and were previously visible.
		// At this point we don't actually know if they're still visible,
		// but it's very likely.
		for(auto &hLight : m_previouslyVisibleShadowedLights) {
			if(hLight.expired())
				continue;
			auto *shadowC = hLight->GetShadowComponent<CShadowComponent>();
			if(shadowC == nullptr)
				continue;
			shadowC->GetRenderer().BuildRenderQueues(drawSceneInfo);
		}

		InvokeEventCallbacks(cSceneComponent::EVENT_ON_BUILD_RENDER_QUEUES, evData);
	});
}

rendering::WorldEnvironment *CSceneComponent::GetWorldEnvironment() const { return m_worldEnvironment.get(); }
void CSceneComponent::SetWorldEnvironment(rendering::WorldEnvironment &env)
{
	ClearWorldEnvironment();

	m_worldEnvironment = env.shared_from_this();
	m_envCallbacks.push_back(m_worldEnvironment->GetShaderQualityProperty()->AddCallback([this](std::reference_wrapper<const int32_t> oldVal, std::reference_wrapper<const int32_t> newVal) { m_renderSettings.shaderQuality = newVal.get(); }));
	m_envCallbacks.push_back(m_worldEnvironment->GetShadowResolutionProperty()->AddCallback([this](std::reference_wrapper<const uint32_t> oldVal, std::reference_wrapper<const uint32_t> newVal) {
		Vector2 shadowRatio {1.f / static_cast<float>(newVal.get()), 1.f / static_cast<float>(newVal.get())};
		m_renderSettings.shadowRatioX = shadowRatio.x;
		m_renderSettings.shadowRatioY = shadowRatio.y;
	}));
	m_envCallbacks.push_back(m_worldEnvironment->GetUnlitProperty()->AddCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) { UpdateRenderSettings(); }));

	// Fog
	auto &fog = m_worldEnvironment->GetFogSettings();
	m_envCallbacks.push_back(fog.GetColorProperty()->AddCallback([this](std::reference_wrapper<const Color> oldVal, std::reference_wrapper<const Color> newVal) {
		m_fogData.color = newVal.get().ToVector4();
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer, offsetof(FogData, color), m_fogData.color);
	}));
	m_envCallbacks.push_back(fog.GetStartProperty()->AddCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> newVal) {
		m_fogData.start = newVal.get();
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer, offsetof(FogData, start), m_fogData.start);
	}));
	m_envCallbacks.push_back(fog.GetEndProperty()->AddCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> newVal) {
		m_fogData.end = newVal.get();
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer, offsetof(FogData, end), m_fogData.end);
	}));
	m_envCallbacks.push_back(fog.GetMaxDensityProperty()->AddCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> newVal) {
		m_fogData.density = newVal.get();
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer, offsetof(FogData, density), m_fogData.density);
	}));
	m_envCallbacks.push_back(fog.GetTypeProperty()->AddCallback([this](std::reference_wrapper<const uint8_t> oldVal, std::reference_wrapper<const uint8_t> newVal) {
		m_fogData.type = static_cast<uint32_t>(newVal.get());
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer, offsetof(FogData, type), m_fogData.type);
	}));
	m_envCallbacks.push_back(fog.GetEnabledProperty()->AddCallback([this](std::reference_wrapper<const bool> oldVal, std::reference_wrapper<const bool> newVal) {
		m_fogData.flags = static_cast<uint32_t>(newVal.get());
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer, offsetof(FogData, flags), m_fogData.flags);
	}));
	m_fogData.color = fog.GetColor().ToVector4();
	m_fogData.start = fog.GetStart();
	m_fogData.end = fog.GetEnd();
	m_fogData.density = fog.GetMaxDensity();
	m_fogData.type = math::to_integral(fog.GetType());
	m_fogData.flags = fog.IsEnabled();
	get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer, 0ull, m_fogData);
}
template<typename TCPPM>
void CSceneComponent::SetLightMap(TCPPM &lightMapC)
{
	auto &renderSettings = GetRenderSettings();
	m_lightMap = lightMapC.GetHandle();
	auto &prop = lightMapC.GetLightMapExposureProperty();
	UpdateRenderSettings();
	UpdateRendererLightMap();
}
template DLLCLIENT void CSceneComponent::SetLightMap(CLightMapComponent &lightMapC);
void CSceneComponent::UpdateRendererLightMap()
{
	if(m_renderer.expired() || m_lightMap.expired())
		return;
	auto *lightMapC = static_cast<CLightMapComponent *>(m_lightMap.get());
	auto &texLightMap = lightMapC->GetLightMap();
	if(texLightMap == nullptr)
		return;
	// TODO: Not ideal to have this here; How to handle this in a better way?
	auto raster = m_renderer->GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(raster.valid())
		raster->SetLightMap(*lightMapC);
}
void CSceneComponent::UpdateRenderSettings()
{
	if(m_worldEnvironment == nullptr)
		return;
	auto &unlitProperty = m_worldEnvironment->GetUnlitProperty();
	auto flags = FRenderSetting::None;
	if(unlitProperty->GetValue() == true)
		flags |= FRenderSetting::Unlit;
	m_renderSettings.flags = math::to_integral(flags);
	if(m_renderer.valid())
		static_cast<CRendererComponent *>(m_renderer.get())->UpdateRenderSettings();
}
void CSceneComponent::ClearWorldEnvironment()
{
	for(auto &hCb : m_envCallbacks) {
		if(hCb.IsValid())
			hCb.Remove();
	}
	m_envCallbacks.clear();
	if(m_cbFogCallback.IsValid() == true)
		m_cbFogCallback.Remove();
	m_worldEnvironment = nullptr;
}

/*Vulkan::Texture &CSceneComponent::ResolveRenderTexture(Vulkan::CommandBufferObject *cmdBuffer) {return const_cast<Vulkan::Texture&>(m_hdrInfo.texture->Resolve(cmdBuffer));}
Vulkan::Texture &CSceneComponent::ResolveDepthTexture(Vulkan::CommandBufferObject *cmdBuffer) {return const_cast<Vulkan::Texture&>(m_hdrInfo.prepass.textureDepth->Resolve(cmdBuffer));}
Vulkan::Texture &CSceneComponent::ResolveBloomTexture(Vulkan::CommandBufferObject *cmdBuffer) {return const_cast<Vulkan::Texture&>(m_hdrInfo.textureBloom->Resolve(cmdBuffer));}*/ // prosper TODO

void CSceneComponent::Resize(uint32_t width, uint32_t height, bool reload)
{
	if(m_renderer.expired() || (reload == false && width == GetWidth() && height == GetHeight()))
		return;
	ReloadRenderTarget(width, height);
}
void CSceneComponent::LinkWorldEnvironment(CSceneComponent &other)
{
	// T
	auto &worldEnv = *GetWorldEnvironment();
	auto &worldEnvOther = *other.GetWorldEnvironment();
	worldEnv.GetShaderQualityProperty()->Link(*worldEnvOther.GetShaderQualityProperty());
	worldEnv.GetShadowResolutionProperty()->Link(*worldEnvOther.GetShadowResolutionProperty());
	worldEnv.GetUnlitProperty()->Link(*worldEnvOther.GetUnlitProperty());

	auto &fogSettings = worldEnv.GetFogSettings();
	auto &fogSettingsOther = worldEnvOther.GetFogSettings();
	fogSettings.GetColorProperty()->Link(*fogSettingsOther.GetColorProperty());
	fogSettings.GetEnabledProperty()->Link(*fogSettingsOther.GetEnabledProperty());
	fogSettings.GetEndProperty()->Link(*fogSettingsOther.GetEndProperty());
	fogSettings.GetMaxDensityProperty()->Link(*fogSettingsOther.GetMaxDensityProperty());
	fogSettings.GetStartProperty()->Link(*fogSettingsOther.GetStartProperty());
	fogSettings.GetTypeProperty()->Link(*fogSettingsOther.GetTypeProperty());
}

void CSceneComponent::UpdateRenderData()
{
	UpdateRenderSettings();
	UpdateRendererLightMap();
}

template<typename TCPPM>
void CSceneComponent::SetRenderer(TCPPM *renderer)
{
	m_renderer = renderer ? renderer->GetHandle() : pragma::ComponentHandle<BaseEntityComponent> {};
	UpdateRenderSettings();
	UpdateRendererLightMap();
	BroadcastEvent(cSceneComponent::EVENT_ON_RENDERER_CHANGED);
}
template DLLCLIENT void CSceneComponent::SetRenderer(CRendererComponent *renderer);

template<typename TCPPM>
TCPPM *CSceneComponent::GetRenderer()
{
	return static_cast<TCPPM *>(m_renderer.get());
}
template DLLCLIENT CRendererComponent *CSceneComponent::GetRenderer();

template<typename TCPPM>
const TCPPM *CSceneComponent::GetRenderer() const
{
	return const_cast<CSceneComponent *>(this)->GetRenderer<CRendererComponent>();
}
template DLLCLIENT const CRendererComponent *CSceneComponent::GetRenderer() const;

SceneDebugMode CSceneComponent::GetDebugMode() const { return m_debugMode; }
void CSceneComponent::SetDebugMode(SceneDebugMode debugMode) { m_debugMode = debugMode; }

SceneRenderDesc &CSceneComponent::GetSceneRenderDesc() { return m_sceneRenderDesc; }
const SceneRenderDesc &CSceneComponent::GetSceneRenderDesc() const { return const_cast<CSceneComponent *>(this)->GetSceneRenderDesc(); }

void CSceneComponent::SetParticleSystemColorFactor(const Vector4 &colorFactor) { m_particleSystemColorFactor = colorFactor; }
const Vector4 &CSceneComponent::GetParticleSystemColorFactor() const { return m_particleSystemColorFactor; }

bool CSceneComponent::IsValid() const { return math::is_flag_set(m_stateFlags, StateFlags::ValidRenderer); }

CSceneComponent *CSceneComponent::GetParentScene()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::HasParentScene) == false)
		return nullptr;
	return GetByIndex(GetSceneIndex());
}

CSceneComponent::SceneIndex CSceneComponent::GetSceneIndex() const { return m_sceneIndex; }

uint32_t CSceneComponent::GetWidth() const { return m_renderer.valid() ? static_cast<const CRendererComponent *>(m_renderer.get())->GetWidth() : 0; }
uint32_t CSceneComponent::GetHeight() const { return m_renderer.valid() ? static_cast<const CRendererComponent *>(m_renderer.get())->GetHeight() : 0; }

//const Vulkan::DescriptorSet &CSceneComponent::GetBloomGlowDescriptorSet() const {return m_descSetBloomGlow;} // prosper TODO

void CSceneComponent::ReloadRenderTarget(uint32_t width, uint32_t height)
{
	math::set_flag(m_stateFlags, StateFlags::ValidRenderer, false);

	if(m_renderer.expired() || static_cast<CRendererComponent *>(m_renderer.get())->ReloadRenderTarget(*this, width, height) == false)
		return;

	math::set_flag(m_stateFlags, StateFlags::ValidRenderer, true);
}

const ComponentHandle<CCameraComponent> &CSceneComponent::GetActiveCamera() const { return const_cast<CSceneComponent *>(this)->GetActiveCamera(); }
ComponentHandle<CCameraComponent> &CSceneComponent::GetActiveCamera() { return m_camera; }
void CSceneComponent::SetActiveCamera(CCameraComponent &cam)
{
	m_camera = cam.GetHandle<CCameraComponent>();
	m_renderSettings.nearZ = cam.GetNearZ();
	m_renderSettings.farZ = cam.GetFarZ();

	BroadcastEvent(cSceneComponent::EVENT_ON_ACTIVE_CAMERA_CHANGED);
}
void CSceneComponent::SetActiveCamera()
{
	m_camera = decltype(m_camera) {};

	BroadcastEvent(cSceneComponent::EVENT_ON_ACTIVE_CAMERA_CHANGED);
}

/////////////////

CEDrawSceneInfo::CEDrawSceneInfo(const rendering::DrawSceneInfo &drawSceneInfo) : drawSceneInfo {drawSceneInfo} {}
void CEDrawSceneInfo::PushArguments(lua::State *l) { Lua::Push<const rendering::DrawSceneInfo *>(l, &drawSceneInfo); }

////////

void CScene::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSceneComponent>();
}
