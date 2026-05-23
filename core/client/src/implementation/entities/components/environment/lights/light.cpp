// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cstddef>

module pragma.client;

import :entities.components.lights.light;
import :client_state;
import :engine;
import :entities.components.color;
import :entities.components.game_occlusion_culler;
import :entities.components.lights.directional;
import :entities.components.lights.point;
import :entities.components.lights.shadow;
import :entities.components.lights.spot;
import :entities.components.radius;
import :entities.components.render;
import :entities.components.toggle;
import :entities.components.transform;
import :rendering.light_data_buffer_manager;
import :scripting.lua.libraries.vulkan;
import :game;

using namespace pragma;

decltype(CLightComponent::s_lightCount) CLightComponent::s_lightCount = 0u;
prosper::InFlightIndexedBuffer &CLightComponent::GetGlobalRenderBuffer() { return LightDataBufferManager::GetInstance().GetGlobalRenderBuffer(); }
prosper::InFlightIndexedBuffer &CLightComponent::GetGlobalShadowBuffer() { return ShadowDataBufferManager::GetInstance().GetGlobalRenderBuffer(); }
uint32_t CLightComponent::GetMaxLightCount() { return LightDataBufferManager::GetInstance().GetMaxCount(); }
uint32_t CLightComponent::GetMaxShadowCount() { return ShadowDataBufferManager::GetInstance().GetMaxCount(); }
uint32_t CLightComponent::GetLightCount() { return s_lightCount; }
ComponentEventId cLightComponent::EVENT_SHOULD_PASS_ENTITY = INVALID_COMPONENT_ID;
ComponentEventId cLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH = INVALID_COMPONENT_ID;
ComponentEventId cLightComponent::EVENT_SHOULD_PASS_MESH = INVALID_COMPONENT_ID;
ComponentEventId cLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS = INVALID_COMPONENT_ID;
ComponentEventId cLightComponent::EVENT_GET_TRANSFORMATION_MATRIX = INVALID_COMPONENT_ID;
ComponentEventId cLightComponent::EVENT_HANDLE_SHADOW_MAP = INVALID_COMPONENT_ID;
ComponentEventId cLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED = INVALID_COMPONENT_ID;
void CLightComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	cLightComponent::EVENT_SHOULD_PASS_ENTITY = registerEvent("SHOULD_PASS_ENTITY", ComponentEventInfo::Type::Explicit);
	cLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH = registerEvent("SHOULD_PASS_ENTITY_MESH", ComponentEventInfo::Type::Explicit);
	cLightComponent::EVENT_SHOULD_PASS_MESH = registerEvent("SHOULD_PASS_MESH", ComponentEventInfo::Type::Explicit);
	cLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS = registerEvent("SHOULD_UPDATE_RENDER_PASS", ComponentEventInfo::Type::Explicit);
	cLightComponent::EVENT_GET_TRANSFORMATION_MATRIX = registerEvent("GET_TRANSFORMATION_MATRIX", ComponentEventInfo::Type::Explicit);
	cLightComponent::EVENT_HANDLE_SHADOW_MAP = registerEvent("HANDLE_SHADOW_MAP", ComponentEventInfo::Type::Broadcast);
	cLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED = registerEvent("ON_SHADOW_BUFFER_INITIALIZED", ComponentEventInfo::Type::Broadcast);
}
void CLightComponent::InitializeBuffers()
{
	LightDataBufferManager::GetInstance().Initialize();
	ShadowDataBufferManager::GetInstance().Initialize();
}
CLightComponent *CLightComponent::GetLightByBufferIndex(LightBufferIndex idx) { return LightDataBufferManager::GetInstance().GetLightByBufferIndex(idx); }
CLightComponent *CLightComponent::GetLightByShadowBufferIndex(ShadowBufferIndex idx) { return ShadowDataBufferManager::GetInstance().GetLightByBufferIndex(idx); }
void CLightComponent::ClearBuffers()
{
	LightDataBufferManager::GetInstance().Reset();
	ShadowDataBufferManager::GetInstance().Reset();
}

CLightComponent::CLightComponent(ecs::BaseEntity &ent) : CBaseLightComponent(ent), m_stateFlags {StateFlags::StaticUpdateRequired | StateFlags::FullUpdateRequired | StateFlags::AddToGameScene} {}
CLightComponent::~CLightComponent()
{
	--s_lightCount;
	DestroyRenderBuffer();
	DestroyShadowBuffer();
}
void CLightComponent::SetBaked(bool baked)
{
	BaseEnvLightComponent::SetBaked(baked);
	if(baked) {
		DestroyRenderBuffer();
		return;
	}
	if(!m_renderBufferIndex && GetEntity().IsEnabled()) {
		auto &flags = m_bufferData.flags;
		math::set_flag(flags, LightBufferData::BufferFlags::TurnedOn, true);
		InitializeRenderBuffer();
		UpdateBuffers();
	}
}
void CLightComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CLightComponent::InitializeRenderBuffer()
{
	if(m_renderBufferIndex || math::is_flag_set(m_lightFlags, LightFlags::BakedLightSource))
		return;
	m_renderBufferIndex = LightDataBufferManager::GetInstance().Request(*this, m_bufferData);
	math::remove_flag(m_stateFlags, StateFlags::BufferDirty);
}

void CLightComponent::InitializeShadowBuffer()
{
	if(m_shadowBufferIndex)
		return;
	m_shadowBufferData = std::make_unique<ShadowBufferData>();
	m_shadowBufferIndex = ShadowDataBufferManager::GetInstance().Request(*this, *m_shadowBufferData);
	math::remove_flag(m_stateFlags, StateFlags::ShadowBufferDirty);
	BroadcastEvent(cLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED);
}

void CLightComponent::UpdateBuffer()
{
	if(!math::is_flag_set(m_stateFlags, StateFlags::BufferDirty))
		return;
	math::set_flag(m_stateFlags, StateFlags::BufferDirty, false);
	if(!m_renderBufferIndex)
		return;
	LightDataBufferManager::GetInstance().SyncDataToGpu(*m_renderBufferIndex);
}

void CLightComponent::UpdateShadowBuffer()
{
	if(!math::is_flag_set(m_stateFlags, StateFlags::ShadowBufferDirty))
		return;
	math::set_flag(m_stateFlags, StateFlags::ShadowBufferDirty, false);
	if(!m_shadowBufferIndex)
		return;
	ShadowDataBufferManager::GetInstance().SyncDataToGpu(*m_shadowBufferIndex);
}

void CLightComponent::WriteBufferData(prosper::IBuffer::Offset offset, prosper::IBuffer::Size size, const void *data)
{
	if(!m_renderBufferIndex)
		return;
	memcpy(reinterpret_cast<uint8_t *>(&m_bufferData) + offset, data, size);
	math::set_flag(m_stateFlags, StateFlags::BufferDirty);
}

static std::queue<prosper::InFlightIndexedBuffer::Index> g_dirtyLightBuffers;
void CLightComponent::SetBufferDirty()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::BufferDirty))
		return;
	math::set_flag(m_stateFlags, StateFlags::BufferDirty);
	auto bufIdx = GetRenderBufferIndex();
	if(!bufIdx)
		return;
	g_dirtyLightBuffers.push(*bufIdx);
}

static std::queue<prosper::InFlightIndexedBuffer::Index> g_dirtyShadowBuffers;
void CLightComponent::SetShadowBufferDirty()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::ShadowBufferDirty))
		return;
	math::set_flag(m_stateFlags, StateFlags::ShadowBufferDirty);
	auto bufIdx = GetShadowBufferIndex();
	if(!bufIdx)
		return;
	g_dirtyShadowBuffers.push(*bufIdx);
}

void CLightComponent::UpdateDirtyLightBuffers()
{
	while(!g_dirtyLightBuffers.empty()) {
		auto idx = g_dirtyLightBuffers.front();
		g_dirtyLightBuffers.pop();
		auto *light = GetLightByBufferIndex(idx);
		if(!light)
			continue;
		light->UpdateBuffer();
	}
	// TODO: We might be able to skip this if we make the above cover all frame-in-flight buffers
	// (e.g. have one dirty queue per frame in flight and add a resource flag per frame in flight for each light source?)
	LightDataBufferManager::GetInstance().GetGlobalRenderBuffer().UpdateDirtyBuffers();

	while(!g_dirtyShadowBuffers.empty()) {
		auto idx = g_dirtyShadowBuffers.front();
		g_dirtyShadowBuffers.pop();
		auto *light = GetLightByShadowBufferIndex(idx);
		if(!light)
			continue;
		light->UpdateShadowBuffer();
	}
	ShadowDataBufferManager::GetInstance().GetGlobalRenderBuffer().UpdateDirtyBuffers();
}

void CLightComponent::DestroyRenderBuffer(bool freeBuffer)
{
	//m_bufferUpdateInfo.clear(); // prosper TODO
	if(!m_renderBufferIndex)
		return;
	auto flags = m_bufferData.flags;
	math::set_flag(flags, LightBufferData::BufferFlags::TurnedOn, false);
	WriteBufferData(offsetof(LightBufferData, flags), sizeof(flags), &flags);

	auto bufIdx = *m_renderBufferIndex;
	m_renderBufferIndex = {};
	math::remove_flag(m_stateFlags, StateFlags::BufferDirty);
	if(freeBuffer) {
		m_bufferData.flags = flags;
		LightDataBufferManager::GetInstance().Free(bufIdx);
	}
}

void CLightComponent::DestroyShadowBuffer(bool freeBuffer)
{
	if(!m_shadowBufferIndex)
		return;
	auto bufIdx = *m_shadowBufferIndex;
	m_shadowBufferIndex = {};
	m_shadowBufferData = nullptr;
	math::remove_flag(m_stateFlags, StateFlags::ShadowBufferDirty);
	if(freeBuffer)
		LightDataBufferManager::GetInstance().Free(bufIdx);
}

bool CLightComponent::ShouldRender() { return true; }

bool CLightComponent::ShouldPass(const asset::Model &mdl, const geometry::CModelSubMesh &mesh)
{
	auto &materials = mdl.GetMaterials();
	auto texId = mdl.GetMaterialIndex(mesh);
	if(texId.has_value() == false || *texId >= materials.size() || !materials[*texId]) // Ignore meshes with invalid materials
		return false;
	auto &mat = materials[*texId];
	auto *info = mat.get()->GetShaderInfo();
	if(info == nullptr || const_cast<util::ShaderInfo *>(info)->GetShader() == nullptr) // Ignore meshes with nodraw (Or invalid) shaders
		return false;
	CEShouldPassMesh evData {mdl, mesh};
	InvokeEventCallbacks(cLightComponent::EVENT_SHOULD_PASS_MESH, evData);
	return evData.shouldPass;
}

void CLightComponent::InitializeLight(BaseEntityComponent &component) { CBaseLightComponent::InitializeLight(component); }

bool CLightComponent::ShouldPass(const ecs::CBaseEntity &ent, uint32_t &renderFlags)
{
	if(ShouldCastShadows() == false)
		return false;
	CEShouldPassEntity evData {ent, renderFlags};
	if(InvokeEventCallbacks(cLightComponent::EVENT_SHOULD_PASS_ENTITY, evData) == util::EventReply::Handled)
		return evData.shouldPass;
	return true;
}
bool CLightComponent::ShouldPass(const ecs::CBaseEntity &ent, const geometry::CModelMesh &mesh, uint32_t &renderFlags)
{
	if(ShouldCastShadows() == false)
		return false;
	CEShouldPassEntityMesh evData {ent, mesh, renderFlags};
	InvokeEventCallbacks(cLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH, evData);
	return evData.shouldPass;
}

CSceneComponent *CLightComponent::FindShadowScene() const
{
	auto sceneFlags = static_cast<const ecs::CBaseEntity &>(GetEntity()).GetSceneFlags();
	// A shadowed light source should always only be assigned to one scene slot, so
	// we'll just pick whichever is the first
	auto lowestBit = static_cast<int32_t>(sceneFlags) & -static_cast<int32_t>(sceneFlags);
	return CSceneComponent::GetByIndex(rendering::get_scene_index(lowestBit));
}
template<typename TCPPM>
TCPPM *CLightComponent::FindShadowOcclusionCuller() const
{
	auto *scene = FindShadowScene();
	return scene ? scene->FindOcclusionCuller<TCPPM>() : nullptr;
}
template COcclusionCullerComponent *CLightComponent::FindShadowOcclusionCuller() const;

bool CLightComponent::IsInCone(const ecs::CBaseEntity &ent, const Vector3 &dir, float angle) const
{
	auto pRenderComponent = ent.GetRenderComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentThis = GetEntity().GetTransformComponent();
	if(!pRenderComponent || pTrComponent == nullptr || !pTrComponentThis)
		return false;
	auto &start = pTrComponentThis->GetPosition();
	auto &sphere = pRenderComponent->GetUpdatedAbsoluteRenderSphere();
	return math::intersection::sphere_cone(pTrComponent->GetPosition() + sphere.pos, sphere.radius, start, dir, angle);
}
void CLightComponent::SetLightIntensity(float intensity, LightIntensityType type)
{
	CBaseLightComponent::SetLightIntensity(intensity, type);
	UpdateLightIntensity();
}
void CLightComponent::SetLightIntensityType(LightIntensityType type)
{
	CBaseLightComponent::SetLightIntensityType(type);
	UpdateLightIntensity();
}
void CLightComponent::UpdateLightIntensity()
{
	m_bufferData.intensity = GetLightIntensityCandela();
	SetBufferDirty();
}
bool CLightComponent::IsInRange(const ecs::CBaseEntity &ent) const
{
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto pRenderComponent = ent.GetRenderComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentThis = GetEntity().GetTransformComponent();
	if(pRadiusComponent.expired() || !pRenderComponent || pTrComponent == nullptr || !pTrComponentThis)
		return false;
	auto &origin = pTrComponentThis->GetPosition();
	auto &sphere = pRenderComponent->GetAbsoluteRenderSphere();
	auto &pos = pTrComponent->GetPosition();
	auto radius = pRadiusComponent->GetRadius();
	return (uvec::distance(pos + sphere.pos, origin) <= (radius + sphere.radius)) ? true : false;
}
bool CLightComponent::IsInRange(const ecs::CBaseEntity &ent, const geometry::CModelMesh &mesh) const
{
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentThis = GetEntity().GetTransformComponent();
	if(pRadiusComponent.expired() || pTrComponent == nullptr || !pTrComponentThis)
		return false;
	auto &origin = pTrComponentThis->GetPosition();
	auto radius = pRadiusComponent->GetRadius();
	auto &pos = pTrComponent->GetPosition();
	Vector3 min;
	Vector3 max;
	mesh.GetBounds(min, max);
	min += pos;
	max += pos;
	return math::intersection::aabb_sphere(min, max, origin, radius);
}

bool CLightComponent::ShouldUpdateRenderPass(rendering::ShadowMapType smType) const
{
	CEShouldUpdateRenderPass evData {};
	if(InvokeEventCallbacks(cLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS, evData) == util::EventReply::Handled)
		return evData.shouldUpdate;
	return math::is_flag_set(m_stateFlags, (smType == rendering::ShadowMapType::Static) ? StateFlags::StaticUpdateRequired : StateFlags::DynamicUpdateRequired);
}

void CLightComponent::UpdateBuffers()
{
	UpdateBuffer();
	UpdateShadowBuffer();
}
void CLightComponent::UpdateShadowTypes()
{
	auto b = ShouldCastShadows();
	auto shadowIndex = 0u;
	if(b == true) {
		InitializeShadowBuffer();
		if(m_shadowBufferIndex != std::nullopt) {
			shadowIndex = *m_shadowBufferIndex + 1u;
			if(m_bufferData.shadowIndex == shadowIndex)
				return;
		}
	}
	else {
		DestroyShadowBuffer();
		SetShadowMapIndex(std::numeric_limits<uint32_t>::max(), rendering::ShadowMapType::Dynamic);
		SetShadowMapIndex(std::numeric_limits<uint32_t>::max(), rendering::ShadowMapType::Static);
	}
	m_bufferData.shadowIndex = shadowIndex;
	SetBufferDirty();
}
bool CLightComponent::ShouldCastShadows() const { return GetEffectiveShadowType() != ShadowType::None; }
bool CLightComponent::ShouldCastDynamicShadows() const { return GetEffectiveShadowType() == ShadowType::Full; }
bool CLightComponent::ShouldCastStaticShadows() const { return ShouldCastShadows(); }
void CLightComponent::SetShadowType(ShadowType type)
{
	if(type == GetShadowType())
		return;
	CBaseLightComponent::SetShadowType(type);
	if(GetEffectiveShadowType() != ShadowType::None)
		InitializeShadowMap();
	else {
		m_shadowMapStatic = ComponentHandle<BaseEntityComponent> {};
		m_shadowMapDynamic = ComponentHandle<BaseEntityComponent> {};
	}
	UpdateShadowTypes(); // Has to be called AFTER the shadowmap has been initialized!
}

void CLightComponent::SetFalloffExponent(float falloffExponent)
{
	if(falloffExponent == m_bufferData.falloffExponent)
		return;
	BaseEnvLightComponent::SetFalloffExponent(falloffExponent);
	m_bufferData.SetFalloffExponent(falloffExponent);
	SetBufferDirty();
}

uint32_t CLightComponent::GetShadowMapIndex(rendering::ShadowMapType smType) const
{
	switch(smType) {
	case rendering::ShadowMapType::Dynamic:
		return m_bufferData.shadowMapIndexDynamic;
	case rendering::ShadowMapType::Static:
		return m_bufferData.shadowMapIndexStatic;
	}
	return 0u;
}

void CLightComponent::SetShadowMapIndex(uint32_t idx, rendering::ShadowMapType smType)
{
	idx = (idx == std::numeric_limits<uint32_t>::max()) ? 0u : (idx + 1);
	auto &target = (smType == rendering::ShadowMapType::Dynamic) ? m_bufferData.shadowMapIndexDynamic : m_bufferData.shadowMapIndexStatic;
	if(idx == target)
		return;
	target = idx;
	SetBufferDirty();
}

template<typename TCPPM>
void CLightComponent::InitializeShadowMap(TCPPM &sm)
{
	static_cast<CShadowComponent &>(sm).SetTextureReloadCallback([this]() { UpdateShadowTypes(); });
	UpdateShadowTypes();
}
template void CLightComponent::InitializeShadowMap(CShadowComponent &sm);

void CLightComponent::InitializeShadowMap()
{
	if(GetEffectiveShadowType() == ShadowType::None)
		return;
	CEHandleShadowMap ceData {};
	if(BroadcastEvent(cLightComponent::EVENT_HANDLE_SHADOW_MAP, ceData) == util::EventReply::Unhandled)
		m_shadowMapStatic = GetEntity().AddComponent<CShadowComponent>(true)->GetHandle();
	else if(ceData.resultShadow)
		m_shadowMapStatic = ceData.resultShadow->GetHandle();
	if(m_shadowMapStatic.valid())
		InitializeShadowMap(*m_shadowMapStatic);
	if(GetEffectiveShadowType() == ShadowType::Full) {
		CEHandleShadowMap ceData {};
		if(BroadcastEvent(cLightComponent::EVENT_HANDLE_SHADOW_MAP, ceData) == util::EventReply::Unhandled)
			m_shadowMapDynamic = GetEntity().AddComponent<CShadowComponent>(true)->GetHandle();
		else if(ceData.resultShadow)
			m_shadowMapDynamic = ceData.resultShadow->GetHandle();
		if(m_shadowMapDynamic.valid())
			InitializeShadowMap(*m_shadowMapDynamic);
	}
}

void CLightComponent::SetStateFlag(StateFlags flag, bool enabled) { math::set_flag(m_stateFlags, flag, enabled); }

void CLightComponent::SetMorphTargetsInShadowsEnabled(bool enabled) { SetStateFlag(StateFlags::EnableMorphTargetsInShadows, enabled); }
bool CLightComponent::AreMorphTargetsInShadowsEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::EnableMorphTargetsInShadows); }

void CLightComponent::Initialize()
{
	CBaseLightComponent::Initialize();

	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	ent.AddComponent<CShadowComponent>();

	BindEventUnhandled(baseToggleComponent::EVENT_ON_TURN_ON, [this](std::reference_wrapper<ComponentEvent> evData) {
		math::set_flag(m_bufferData.flags, LightBufferData::BufferFlags::TurnedOn, true);
		if(m_renderBufferIndex != std::nullopt)
			SetBufferDirty();
		else
			InitializeRenderBuffer();
		// TODO: This will update all light and shadow buffers for this light source.
		// This shouldn't be necessary, but without light sources seem to have incorrect buffer
		// data when turned on. Once the cause for this has been found and dealt with, this
		// line can be removed!
		UpdateBuffers();

		(TickPolicy::Never);
	});
	BindEventUnhandled(baseToggleComponent::EVENT_ON_TURN_OFF, [this](std::reference_wrapper<ComponentEvent> evData) {
		math::set_flag(m_bufferData.flags, LightBufferData::BufferFlags::TurnedOn, false);
		SetBufferDirty();
		m_tTurnedOff = get_cgame()->RealTime();

		SetTickPolicy(TickPolicy::Always);
		SetNextTick(get_cgame()->CurTime() + 30.f);
	});
	BindEventUnhandled(cBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		m_bufferData.sceneFlags = static_cast<ecs::CBaseEntity &>(GetEntity()).GetSceneFlags();
		SetBufferDirty();
	});
	SetTickPolicy(TickPolicy::Never);
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent != nullptr)
		reinterpret_cast<Vector3 &>(m_bufferData.position) = pTrComponent->GetPosition();
	if(m_bufferData.direction.x == 0.f && m_bufferData.direction.y == 0.f && m_bufferData.direction.z == 0.f)
		m_bufferData.direction.z = 1.f;
	m_bufferData.sceneFlags = ent.GetSceneFlags();

	++s_lightCount;
}
void CLightComponent::OnTick(double dt)
{
	auto frameId = get_cengine()->GetRenderContext().GetLastFrameId();
	if(m_lastThink == frameId)
		return;
	m_lastThink = frameId;

	if(get_cgame()->CurTime() - m_tTurnedOff > 30.0) {
		if(m_renderBufferIndex != std::nullopt) {
			auto pToggleComponent = GetEntity().GetComponent<CToggleComponent>();
			if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn() == false)
				DestroyRenderBuffer(); // Free buffer if light hasn't been on in 30 seconds
		}
		SetTickPolicy(TickPolicy::Never);
	}
}
void CLightComponent::UpdateTransformationMatrix(const Mat4 &biasMatrix, const Mat4 &viewMatrix, const Mat4 &projectionMatrix)
{
	if(m_shadowBufferData != nullptr) {
		m_shadowBufferData->view = viewMatrix;
		m_shadowBufferData->projection = projectionMatrix;
		m_shadowBufferData->depthVP = biasMatrix;
		SetShadowBufferDirty();
	}
}
void CLightComponent::UpdatePos()
{
	auto &pos = GetEntity().GetPosition();
	if(uvec::cmp(pos, reinterpret_cast<Vector3 &>(m_bufferData.position)) == false) {
		reinterpret_cast<Vector3 &>(m_bufferData.position) = pos;
		SetBufferDirty();
		math::set_flag(m_stateFlags, StateFlags::FullUpdateRequired);
	}
}
void CLightComponent::UpdateDir()
{
	LightType lightType;
	GetLight(lightType);
	if(lightType != LightType::Point) {
		auto &rot = GetEntity().GetRotation();
		auto dir = uquat::forward(rot);
		if(uvec::cmp(dir, reinterpret_cast<Vector3 &>(m_bufferData.direction)) == false) {
			reinterpret_cast<Vector3 &>(m_bufferData.direction) = dir;
			if(m_bufferData.direction.x == 0.f && m_bufferData.direction.y == 0.f && m_bufferData.direction.z == 0.f)
				m_bufferData.direction.z = 1.f;
			SetBufferDirty();
			math::set_flag(m_stateFlags, StateFlags::FullUpdateRequired);
		}
	}
}
void CLightComponent::UpdateColor()
{
	auto colorC = GetEntity().GetComponent<CColorComponent>();
	if(colorC.expired())
		return;
	auto &color = colorC->GetColor();
	m_bufferData.color = color;
	SetBufferDirty();

	if(color.a == 0 || (color.r == 0 && color.g == 0 && color.b == 0))
		math::set_flag(m_bufferData.flags, LightBufferData::BufferFlags::TurnedOn, false);
	else {
		auto pToggleComponent = GetEntity().GetComponent<CToggleComponent>();
		if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn() == true)
			math::set_flag(m_bufferData.flags, LightBufferData::BufferFlags::TurnedOn, true);
	}
}
void CLightComponent::UpdateRadius()
{
	auto radiusC = GetEntity().GetComponent<CRadiusComponent>();
	if(radiusC.expired())
		return;
	auto radius = radiusC->GetRadius();
	if(radius == m_bufferData.position.w)
		return;
	m_bufferData.position.w = radius;
	SetBufferDirty();
	math::set_flag(m_stateFlags, StateFlags::FullUpdateRequired);
}
void CLightComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	CBaseLightComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CTransformComponent)) {
		auto &trC = static_cast<CTransformComponent &>(component);
		FlagCallbackForRemoval(trC.AddEventCallback(cTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this, &trC](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			                         if(math::is_flag_set(static_cast<CEOnPoseChanged &>(evData.get()).changeFlags, TransformChangeFlags::PositionChanged))
				                         UpdatePos();

			                         if(math::is_flag_set(static_cast<CEOnPoseChanged &>(evData.get()).changeFlags, TransformChangeFlags::RotationChanged))
				                         UpdateDir();
			                         return util::EventReply::Unhandled;
		                         }),
		  CallbackType::Component, &component);
	}
	else if(typeid(component) == typeid(CRadiusComponent)) {
		FlagCallbackForRemoval(static_cast<CRadiusComponent &>(component).GetRadiusProperty()->AddCallback([this](std::reference_wrapper<const float> oldRadius, std::reference_wrapper<const float> radius) { UpdateRadius(); }), CallbackType::Component, &component);
	}
	else if(typeid(component) == typeid(CColorComponent)) {
		FlagCallbackForRemoval(static_cast<CColorComponent &>(component).GetColorProperty()->AddCallback([this](std::reference_wrapper<const Vector4> oldColor, std::reference_wrapper<const Vector4> color) { UpdateColor(); }), CallbackType::Component, &component);
	}
	else if(typeid(component) == typeid(CLightSpotComponent)) {
		m_bufferData.flags &= ~(LightBufferData::BufferFlags::TypeSpot | LightBufferData::BufferFlags::TypePoint | LightBufferData::BufferFlags::TypeDirectional);
		m_bufferData.flags |= LightBufferData::BufferFlags::TypeSpot;
		SetBufferDirty();
	}
	else if(typeid(component) == typeid(CLightPointComponent)) {
		m_bufferData.flags &= ~(LightBufferData::BufferFlags::TypeSpot | LightBufferData::BufferFlags::TypePoint | LightBufferData::BufferFlags::TypeDirectional);
		m_bufferData.flags |= LightBufferData::BufferFlags::TypePoint;
		SetBufferDirty();
	}
	else if(typeid(component) == typeid(CLightDirectionalComponent)) {
		m_bufferData.flags &= ~(LightBufferData::BufferFlags::TypeSpot | LightBufferData::BufferFlags::TypePoint | LightBufferData::BufferFlags::TypeDirectional);
		m_bufferData.flags |= LightBufferData::BufferFlags::TypeDirectional;
		SetBufferDirty();
	}
	else if(typeid(component) == typeid(CShadowComponent))
		m_shadowComponent = &static_cast<CShadowComponent &>(component);
}
void CLightComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	CBaseLightComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(CShadowComponent))
		m_shadowComponent = nullptr;
}
void CLightComponent::OnEntitySpawn()
{
	CBaseLightComponent::OnEntitySpawn();
	InitializeShadowMap();

	if(math::is_flag_set(m_lightFlags, LightFlags::BakedLightSource)) {
		m_bufferData.flags |= LightBufferData::BufferFlags::BakedLightSource;
		SetBufferDirty();
	}
	UpdatePos();
	UpdateDir();
	UpdateRadius();
	UpdateColor();
}

const LightBufferData &CLightComponent::GetBufferData() const { return const_cast<CLightComponent *>(this)->GetBufferData(); }
LightBufferData &CLightComponent::GetBufferData() { return m_bufferData; }
const ShadowBufferData *CLightComponent::GetShadowBufferData() const { return const_cast<CLightComponent *>(this)->GetShadowBufferData(); }
ShadowBufferData *CLightComponent::GetShadowBufferData() { return m_shadowBufferData.get(); }

template<typename TCPPM>
ComponentHandle<TCPPM> CLightComponent::GetShadowMap(rendering::ShadowMapType type) const
{
	if(type == rendering::ShadowMapType::Dynamic) {
		if(m_shadowMapDynamic.expired())
			return pragma::ComponentHandle<TCPPM> {};
		return const_cast<BaseEntityComponent *>(m_shadowMapDynamic.get())->GetHandle<TCPPM>();
	}
	if(m_shadowMapStatic.expired())
		return pragma::ComponentHandle<TCPPM> {};
	return const_cast<BaseEntityComponent *>(m_shadowMapStatic.get())->GetHandle<TCPPM>();
}
template ComponentHandle<CShadowComponent> CLightComponent::GetShadowMap(rendering::ShadowMapType type) const;

template<typename TCPPM>
TCPPM *CLightComponent::GetShadowComponent()
{
	return static_cast<TCPPM *>(m_shadowComponent);
}
template CShadowComponent *CLightComponent::GetShadowComponent();
template<typename TCPPM>
const TCPPM *CLightComponent::GetShadowComponent() const
{
	return const_cast<CLightComponent *>(this)->GetShadowComponent<TCPPM>();
}
template const CShadowComponent *CLightComponent::GetShadowComponent() const;

bool CLightComponent::HasShadowsEnabled() const { return m_shadowComponent && GetEffectiveShadowType() != ShadowType::None; }

Mat4 &CLightComponent::GetTransformationMatrix(unsigned int j)
{
	CEGetTransformationMatrix evData {j};
	InvokeEventCallbacks(cLightComponent::EVENT_GET_TRANSFORMATION_MATRIX, evData);
	if(evData.transformation != nullptr)
		return *evData.transformation;
	static Mat4 m;
	m = umat::identity();
	return m;
}

std::optional<prosper::InFlightIndexedBuffer::Index> CLightComponent::GetRenderBufferIndex() const { return m_renderBufferIndex; }
std::optional<prosper::InFlightIndexedBuffer::Index> CLightComponent::GetShadowBufferIndex() const { return m_shadowBufferIndex; }
std::optional<prosper::BufferView> CLightComponent::GetCurrentFrameRenderBufferView() const
{
	if(!m_renderBufferIndex)
		return {};
	auto &buf = LightDataBufferManager::GetInstance().GetGlobalRenderBuffer();
	return buf.GetCurrentBufferView(*m_renderBufferIndex);
}

///////////////////

static void debug_light_sources(NetworkState *state, BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &context = get_cengine()->GetRenderContext();
	context.WaitIdle();

	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightComponent>>();
	std::vector<CLightComponent *> lights;
	lights.reserve(entIt.GetCount());
	for(auto *ent : entIt)
		lights.push_back(ent->GetComponent<CLightComponent>().get());

	auto numLights = 0u;
	auto numTurnedOn = 0u;
	std::vector<size_t> discrepancies {};
	for(auto *l : lights) {
		++numLights;
		auto pToggleComponent = l->GetEntity().GetComponent<CToggleComponent>();
		if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn() == true)
			++numTurnedOn;
	}

	auto &lightBufManager = LightDataBufferManager::GetInstance();
	Con::COUT << "Light buffer count: " << lightBufManager.GetLightDataBufferCount() << Con::endl;
	auto numTotal = lightBufManager.GetMaxCount();
	Con::COUT << "Max light count: " << numTotal << Con::endl;
	Con::COUT << "Allocated buffer instances: " << lightBufManager.GetGlobalRenderBuffer().GetAllocatedBufferCount() << Con::endl;

	auto getBufferData = [](prosper::BufferView &buf) -> LightBufferData {
		LightBufferData data;
		buf.Read(0ull, sizeof(data), &data);
		return data;
	};
	auto printBufferData = [](const LightBufferData &data) {
		std::string type = "Unknown";
		if((data.flags & LightBufferData::BufferFlags::TypeSpot) != LightBufferData::BufferFlags::None)
			type = "Spot";
		else if((data.flags & LightBufferData::BufferFlags::TypePoint) != LightBufferData::BufferFlags::None)
			type = "Point";
		else if((data.flags & LightBufferData::BufferFlags::TypeDirectional) != LightBufferData::BufferFlags::None)
			type = "Directional";
		Con::COUT << "\t\tPosition: (" << data.position.x << "," << data.position.y << "," << data.position.z << ")" << Con::endl;
		Con::COUT << "\t\tShadow Index: " << data.shadowIndex << Con::endl;
		Con::COUT << "\t\tShadow Map Index (static): " << data.shadowMapIndexStatic << Con::endl;
		Con::COUT << "\t\tShadow Map Index (dynamic): " << data.shadowMapIndexDynamic << Con::endl;
		Con::COUT << "\t\tType: " << type << Con::endl;
		Con::COUT << "\t\tColor: (" << data.color.r << "," << data.color.g << "," << data.color.b << ")" << Con::endl;
		Con::COUT << "\t\tIntensity (candela): " << data.intensity << Con::endl;
		Con::COUT << "\t\tDirection: (" << data.direction.x << "," << data.direction.y << "," << data.direction.z << ")" << Con::endl;
		Con::COUT << "\t\tCone Start Offset: " << data.direction.w << Con::endl;
		Con::COUT << "\t\tDistance: " << data.position.w << Con::endl;
		Con::COUT << "\t\tOuter cone half-angle: " << math::rad_to_deg(data.outerConeHalfAngle) << Con::endl;
		Con::COUT << "\t\tInner cone half-angle: " << math::rad_to_deg(data.innerConeHalfAngle) << Con::endl;
		Con::COUT << "\t\tAttenuation: " << data.attenuation << Con::endl;
		Con::COUT << "\t\tFlags: " << math::to_integral(data.flags) << Con::endl;
		Con::COUT << "\t\tTurned On: " << (((data.flags & LightBufferData::BufferFlags::TurnedOn) == LightBufferData::BufferFlags::TurnedOn) ? "Yes" : "No") << Con::endl;
	};

	Con::COUT << "Enabled light buffers:" << Con::endl;
	auto &buf = lightBufManager.GetGlobalRenderBuffer();
	// Note: If the buffer is dirty, the information in curBuf may not be up-to-date, so we make sure to update it here.
	for(auto *l : lights) {
		l->SetBufferDirty();
		l->UpdateBuffers();
	}
	auto printGlobalBufferData = [&](prosper::IBuffer &curBuf) {
		std::vector<uint8_t> bufData;
		bufData.resize(curBuf.GetSize());
		curBuf.Read(0, bufData.size(), bufData.data());

		auto *rawData = bufData.data();
		for(auto i = decltype(numTotal) {0u}; i < numTotal; ++i) {
			util::ScopeGuard sg {[&rawData, &buf]() { rawData += buf.GetStride(); }};
			auto &data = *reinterpret_cast<LightBufferData *>(rawData);
			if(!math::is_flag_set(data.flags, LightBufferData::BufferFlags::TurnedOn))
				continue;
			Con::COUT << "Buffer Index: " << i << Con::endl;
			printBufferData(data);

			auto *light = CLightComponent::GetLightByBufferIndex(i);
			Con::COUT << "Light Source: ";
			if(light)
				Con::COUT << light->GetEntity().GetLocalIndex();
			else
				Con::COUT << "None";
			Con::COUT << Con::endl;
			if(light) {
				auto &bufferData = light->GetBufferData();
				if(memcmp(&data, &bufferData, sizeof(data)) != 0) {
					discrepancies.push_back(light->GetEntity().GetLocalIndex());
					Con::CWAR << "Memory discrepancy found! Actual memory data of light source:" << Con::endl;
					printBufferData(bufferData);
				}
			}
		}
	};
	printGlobalBufferData(buf.GetCurrentBuffer());

	Con::COUT << Con::endl;
	Con::COUT << "Light sources:" << Con::endl;
	auto lightId = 0u;
	for(auto *l : lights) {
		Con::COUT << "Light #" << lightId << ":" << Con::endl;
		Con::COUT << "\tType: ";
		auto type = LightType::Undefined;
		auto *pLight = l->GetLight(type);
		switch(type) {
		case LightType::Directional:
			Con::COUT << "Directional";
			break;
		case LightType::Point:
			Con::COUT << "Point";
			break;
		case LightType::Spot:
			Con::COUT << "Spot";
			break;
		default:
			Con::COUT << "Unknown";
			break;
		}
		Con::COUT << Con::endl;

		auto bufView = l->GetCurrentFrameRenderBufferView();
		if(!bufView)
			Con::COUT << "\tBuffer: NULL" << Con::endl;
		else {
			auto data = getBufferData(*bufView);
			printBufferData(data);
		}
		++lightId;
	}
	Con::COUT << "Number of lights: " << numLights << Con::endl;
	Con::COUT << "Turned on: " << numTurnedOn << Con::endl;
	if(discrepancies.empty() == false) {
		Con::CWAR << "Discrepancies found in " << discrepancies.size() << " lights:" << Con::endl;
		for(auto idx : discrepancies)
			Con::COUT << "\t" << idx << Con::endl;
	}
}
namespace {
	auto UVN = console::client::register_command("debug_light_sources", &debug_light_sources, console::ConVarFlags::None, "Prints debug information about all light sources in the scene.");
}

/////////////////

CEShouldPassEntity::CEShouldPassEntity(const ecs::CBaseEntity &entity, uint32_t &renderFlags) : entity {entity}, renderFlags {renderFlags} {}
void CEShouldPassEntity::PushArguments(lua::State *l) {}

/////////////////

CEShouldPassMesh::CEShouldPassMesh(const asset::Model &model, const geometry::CModelSubMesh &mesh) : model {model}, mesh {mesh} {}
void CEShouldPassMesh::PushArguments(lua::State *l) {}

/////////////////

CEShouldPassEntityMesh::CEShouldPassEntityMesh(const ecs::CBaseEntity &entity, const geometry::CModelMesh &mesh, uint32_t &renderFlags) : entity {entity}, mesh {mesh}, renderFlags {renderFlags} {}
void CEShouldPassEntityMesh::PushArguments(lua::State *l) {}

/////////////////

CEShouldUpdateRenderPass::CEShouldUpdateRenderPass() {}
void CEShouldUpdateRenderPass::PushArguments(lua::State *l) {}

/////////////////

CEGetTransformationMatrix::CEGetTransformationMatrix(uint32_t index) : index {index} {}
void CEGetTransformationMatrix::PushArguments(lua::State *l) {}

/////////////////

CEHandleShadowMap::CEHandleShadowMap() {}
void CEHandleShadowMap::PushArguments(lua::State *l) {}
