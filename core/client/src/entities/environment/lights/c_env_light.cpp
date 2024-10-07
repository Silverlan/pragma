/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/rendering/lighting/c_light_data_buffer_manager.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_shaderinfo.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <prosper_command_buffer.hpp>

using namespace pragma;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

decltype(CLightComponent::s_lightCount) CLightComponent::s_lightCount = 0u;
prosper::IUniformResizableBuffer &CLightComponent::GetGlobalRenderBuffer() { return pragma::LightDataBufferManager::GetInstance().GetGlobalRenderBuffer(); }
prosper::IUniformResizableBuffer &CLightComponent::GetGlobalShadowBuffer() { return pragma::ShadowDataBufferManager::GetInstance().GetGlobalRenderBuffer(); }
uint32_t CLightComponent::GetMaxLightCount() { return pragma::LightDataBufferManager::GetInstance().GetMaxCount(); }
uint32_t CLightComponent::GetMaxShadowCount() { return pragma::ShadowDataBufferManager::GetInstance().GetMaxCount(); }
uint32_t CLightComponent::GetLightCount() { return s_lightCount; }
ComponentEventId CLightComponent::EVENT_SHOULD_PASS_ENTITY = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_SHOULD_PASS_MESH = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_HANDLE_SHADOW_MAP = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED = pragma::INVALID_COMPONENT_ID;
void CLightComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_SHOULD_PASS_ENTITY = registerEvent("SHOULD_PASS_ENTITY", ComponentEventInfo::Type::Explicit);
	EVENT_SHOULD_PASS_ENTITY_MESH = registerEvent("SHOULD_PASS_ENTITY_MESH", ComponentEventInfo::Type::Explicit);
	EVENT_SHOULD_PASS_MESH = registerEvent("SHOULD_PASS_MESH", ComponentEventInfo::Type::Explicit);
	EVENT_SHOULD_UPDATE_RENDER_PASS = registerEvent("SHOULD_UPDATE_RENDER_PASS", ComponentEventInfo::Type::Explicit);
	EVENT_GET_TRANSFORMATION_MATRIX = registerEvent("GET_TRANSFORMATION_MATRIX", ComponentEventInfo::Type::Explicit);
	EVENT_HANDLE_SHADOW_MAP = registerEvent("HANDLE_SHADOW_MAP", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_SHADOW_BUFFER_INITIALIZED = registerEvent("ON_SHADOW_BUFFER_INITIALIZED", ComponentEventInfo::Type::Broadcast);
}
void CLightComponent::InitializeBuffers()
{
	pragma::LightDataBufferManager::GetInstance().Initialize();
	pragma::ShadowDataBufferManager::GetInstance().Initialize();
}
CLightComponent *CLightComponent::GetLightByBufferIndex(LightBufferIndex idx) { return LightDataBufferManager::GetInstance().GetLightByBufferIndex(idx); }
CLightComponent *CLightComponent::GetLightByShadowBufferIndex(ShadowBufferIndex idx) { return ShadowDataBufferManager::GetInstance().GetLightByBufferIndex(idx); }
void CLightComponent::ClearBuffers()
{
	LightDataBufferManager::GetInstance().Reset();
	ShadowDataBufferManager::GetInstance().Reset();
}

CLightComponent::CLightComponent(BaseEntity &ent) : CBaseLightComponent(ent), m_stateFlags {StateFlags::StaticUpdateRequired | StateFlags::FullUpdateRequired | StateFlags::AddToGameScene} {}
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
	if(!m_renderBuffer && GetEntity().IsEnabled()) {
		auto &flags = m_bufferData.flags;
		umath::set_flag(flags, LightBufferData::BufferFlags::TurnedOn, true);
		InitializeRenderBuffer();
		UpdateBuffers();
	}
}
void CLightComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CLightComponent::InitializeRenderBuffer()
{
	if(m_renderBuffer != nullptr || umath::is_flag_set(m_lightFlags, LightFlags::BakedLightSource))
		return;
	m_renderBuffer = LightDataBufferManager::GetInstance().Request(*this, m_bufferData);
}

void CLightComponent::InitializeShadowBuffer()
{
	if(m_shadowBuffer != nullptr)
		return;
	m_shadowBuffer = ShadowDataBufferManager::GetInstance().Request(*this, *m_shadowBufferData);
	BroadcastEvent(EVENT_ON_SHADOW_BUFFER_INITIALIZED, CEOnShadowBufferInitialized {*m_shadowBuffer});
}

void CLightComponent::DestroyRenderBuffer(bool freeBuffer)
{
	//m_bufferUpdateInfo.clear(); // prosper TODO
	if(m_renderBuffer == nullptr)
		return;
	auto flags = m_bufferData.flags;
	umath::set_flag(flags, LightBufferData::BufferFlags::TurnedOn, false);
	if(m_renderBuffer != nullptr)
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, flags), flags);

	auto buf = m_renderBuffer;
	m_renderBuffer = nullptr;
	if(freeBuffer) {
		m_bufferData.flags = flags;
		LightDataBufferManager::GetInstance().Free(buf);
	}
}

void CLightComponent::DestroyShadowBuffer(bool freeBuffer)
{
	if(m_shadowBuffer == nullptr)
		return;
	auto buf = m_shadowBuffer;
	m_shadowBuffer = nullptr;
	if(freeBuffer)
		ShadowDataBufferManager::GetInstance().Free(buf);
}

bool CLightComponent::ShouldRender() { return true; }

bool CLightComponent::ShouldPass(const Model &mdl, const CModelSubMesh &mesh)
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
	InvokeEventCallbacks(EVENT_SHOULD_PASS_MESH, evData);
	return evData.shouldPass;
}

void CLightComponent::InitializeLight(BaseEntityComponent &component) { CBaseLightComponent::InitializeLight(component); }

bool CLightComponent::ShouldPass(const CBaseEntity &ent, uint32_t &renderFlags)
{
	if(ShouldCastShadows() == false)
		return false;
	CEShouldPassEntity evData {ent, renderFlags};
	if(InvokeEventCallbacks(EVENT_SHOULD_PASS_ENTITY, evData) == util::EventReply::Handled)
		return evData.shouldPass;
	return true;
}
bool CLightComponent::ShouldPass(const CBaseEntity &ent, const CModelMesh &mesh, uint32_t &renderFlags)
{
	if(ShouldCastShadows() == false)
		return false;
	CEShouldPassEntityMesh evData {ent, mesh, renderFlags};
	InvokeEventCallbacks(EVENT_SHOULD_PASS_ENTITY_MESH, evData);
	return evData.shouldPass;
}

CSceneComponent *CLightComponent::FindShadowScene() const
{
	auto sceneFlags = static_cast<const CBaseEntity &>(GetEntity()).GetSceneFlags();
	// A shadowed light source should always only be assigned to one scene slot, so
	// we'll just pick whichever is the first
	auto lowestBit = static_cast<int32_t>(sceneFlags) & -static_cast<int32_t>(sceneFlags);
	return CSceneComponent::GetByIndex(CSceneComponent::GetSceneIndex(lowestBit));
}
COcclusionCullerComponent *CLightComponent::FindShadowOcclusionCuller() const
{
	auto *scene = FindShadowScene();
	return scene ? scene->FindOcclusionCuller() : nullptr;
}

bool CLightComponent::IsInCone(const CBaseEntity &ent, const Vector3 &dir, float angle) const
{
	auto pRenderComponent = ent.GetRenderComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentThis = GetEntity().GetTransformComponent();
	if(!pRenderComponent || pTrComponent == nullptr || !pTrComponentThis)
		return false;
	auto &start = pTrComponentThis->GetPosition();
	auto &sphere = pRenderComponent->GetUpdatedAbsoluteRenderSphere();
	return umath::intersection::sphere_cone(pTrComponent->GetPosition() + sphere.pos, sphere.radius, start, dir, angle);
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
	if(m_renderBuffer == nullptr)
		return;
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, intensity), m_bufferData.intensity);
}
bool CLightComponent::IsInRange(const CBaseEntity &ent) const
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
bool CLightComponent::IsInRange(const CBaseEntity &ent, const CModelMesh &mesh) const
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
	return umath::intersection::aabb_sphere(min, max, origin, radius);
}

bool CLightComponent::ShouldUpdateRenderPass(ShadowMapType smType) const
{
	CEShouldUpdateRenderPass evData {};
	if(InvokeEventCallbacks(EVENT_SHOULD_UPDATE_RENDER_PASS, evData) == util::EventReply::Handled)
		return evData.shouldUpdate;
	return umath::is_flag_set(m_stateFlags, (smType == ShadowMapType::Static) ? StateFlags::StaticUpdateRequired : StateFlags::DynamicUpdateRequired);
}

void CLightComponent::UpdateBuffers()
{
	if(m_renderBuffer != nullptr)
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, 0ull, m_bufferData);
	if(m_shadowBuffer != nullptr && m_shadowBufferData != nullptr)
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_shadowBuffer, 0ull, m_shadowBufferData);
}
void CLightComponent::UpdateShadowTypes()
{
	auto b = ShouldCastShadows();
	auto shadowIndex = 0u;
	if(b == true) {
		InitializeShadowBuffer();
		if(m_shadowBuffer != nullptr) {
			shadowIndex = m_shadowBuffer->GetBaseIndex() + 1u;
			if(m_bufferData.shadowIndex == shadowIndex)
				return;
		}
	}
	else {
		DestroyShadowBuffer();
		SetShadowMapIndex(std::numeric_limits<uint32_t>::max(), ShadowMapType::Dynamic);
		SetShadowMapIndex(std::numeric_limits<uint32_t>::max(), ShadowMapType::Static);
	}
	m_bufferData.shadowIndex = shadowIndex;
	if(m_renderBuffer != nullptr)
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, shadowIndex), m_bufferData.shadowIndex);
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
		m_shadowMapStatic = ComponentHandle<CShadowComponent> {};
		m_shadowMapDynamic = ComponentHandle<CShadowComponent> {};
	}
	UpdateShadowTypes(); // Has to be called AFTER the shadowmap has been initialized!
}

void CLightComponent::SetFalloffExponent(float falloffExponent)
{
	if(falloffExponent == m_bufferData.falloffExponent)
		return;
	BaseEnvLightComponent::SetFalloffExponent(falloffExponent);
	m_bufferData.falloffExponent = falloffExponent;
	if(m_renderBuffer != nullptr)
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, falloffExponent), m_bufferData.falloffExponent);
}

uint32_t CLightComponent::GetShadowMapIndex(ShadowMapType smType) const
{
	switch(smType) {
	case ShadowMapType::Dynamic:
		return m_bufferData.shadowMapIndexDynamic;
	case ShadowMapType::Static:
		return m_bufferData.shadowMapIndexStatic;
	}
	return 0u;
}

void CLightComponent::SetShadowMapIndex(uint32_t idx, ShadowMapType smType)
{
	idx = (idx == std::numeric_limits<uint32_t>::max()) ? 0u : (idx + 1);
	auto &target = (smType == ShadowMapType::Dynamic) ? m_bufferData.shadowMapIndexDynamic : m_bufferData.shadowMapIndexStatic;
	if(idx == target)
		return;
	target = idx;
	if(m_renderBuffer != nullptr) {
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, (smType == ShadowMapType::Dynamic) ? offsetof(LightBufferData, shadowMapIndexDynamic) : offsetof(LightBufferData, shadowMapIndexStatic), target);
	}
}

void CLightComponent::InitializeShadowMap(CShadowComponent &sm)
{
	sm.SetTextureReloadCallback([this]() { UpdateShadowTypes(); });
	UpdateShadowTypes();
}

void CLightComponent::InitializeShadowMap()
{
	if(GetEffectiveShadowType() == ShadowType::None)
		return;
	CEHandleShadowMap ceData {};
	if(BroadcastEvent(EVENT_HANDLE_SHADOW_MAP, ceData) == util::EventReply::Unhandled)
		m_shadowMapStatic = GetEntity().AddComponent<CShadowComponent>(true);
	else if(ceData.resultShadow)
		m_shadowMapStatic = ceData.resultShadow->GetHandle<CShadowComponent>();
	if(m_shadowMapStatic.valid())
		InitializeShadowMap(*m_shadowMapStatic);
	if(GetEffectiveShadowType() == ShadowType::Full) {
		CEHandleShadowMap ceData {};
		if(BroadcastEvent(EVENT_HANDLE_SHADOW_MAP, ceData) == util::EventReply::Unhandled)
			m_shadowMapDynamic = GetEntity().AddComponent<CShadowComponent>(true);
		else if(ceData.resultShadow)
			m_shadowMapDynamic = ceData.resultShadow->GetHandle<CShadowComponent>();
		if(m_shadowMapDynamic.valid())
			InitializeShadowMap(*m_shadowMapDynamic);
	}
}

void CLightComponent::SetStateFlag(StateFlags flag, bool enabled) { umath::set_flag(m_stateFlags, flag, enabled); }

void CLightComponent::SetMorphTargetsInShadowsEnabled(bool enabled) { SetStateFlag(StateFlags::EnableMorphTargetsInShadows, enabled); }
bool CLightComponent::AreMorphTargetsInShadowsEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::EnableMorphTargetsInShadows); }

void CLightComponent::Initialize()
{
	CBaseLightComponent::Initialize();

	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	ent.AddComponent<CShadowComponent>();

	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_ON, [this](std::reference_wrapper<ComponentEvent> evData) {
		umath::set_flag(m_bufferData.flags, LightBufferData::BufferFlags::TurnedOn, true);
		if(m_renderBuffer != nullptr)
			c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, flags), m_bufferData.flags);
		else
			InitializeRenderBuffer();
		// TODO: This will update all light and shadow buffers for this light source.
		// This shouldn't be necessary, but without light sources seem to have incorrect buffer
		// data when turned on. Once the cause for this has been found and dealt with, this
		// line can be removed!
		UpdateBuffers();

		(pragma::TickPolicy::Never);
	});
	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_OFF, [this](std::reference_wrapper<ComponentEvent> evData) {
		umath::set_flag(m_bufferData.flags, LightBufferData::BufferFlags::TurnedOn, false);
		if(m_renderBuffer != nullptr)
			c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, flags), m_bufferData.flags);
		m_tTurnedOff = c_game->RealTime();

		SetTickPolicy(pragma::TickPolicy::Always);
		SetNextTick(c_game->CurTime() + 30.f);
	});
	BindEventUnhandled(CBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		m_bufferData.sceneFlags = static_cast<CBaseEntity &>(GetEntity()).GetSceneFlags();
		if(m_renderBuffer != nullptr)
			c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, sceneFlags), m_bufferData.sceneFlags);
	});
	SetTickPolicy(pragma::TickPolicy::Never);
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
	auto frameId = c_engine->GetRenderContext().GetLastFrameId();
	if(m_lastThink == frameId)
		return;
	m_lastThink = frameId;

	if(c_game->CurTime() - m_tTurnedOff > 30.0) {
		if(m_renderBuffer != nullptr) {
			auto pToggleComponent = GetEntity().GetComponent<CToggleComponent>();
			if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn() == false)
				DestroyRenderBuffer(); // Free buffer if light hasn't been on in 30 seconds
		}
		SetTickPolicy(pragma::TickPolicy::Never);
	}
}
void CLightComponent::UpdateTransformationMatrix(const Mat4 &biasMatrix, const Mat4 &viewMatrix, const Mat4 &projectionMatrix)
{
	if(m_shadowBufferData != nullptr) {
		m_shadowBufferData->view = viewMatrix;
		m_shadowBufferData->projection = projectionMatrix;
		m_shadowBufferData->depthVP = biasMatrix;
	}
	if(m_shadowBuffer != nullptr)
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_shadowBuffer, offsetof(ShadowBufferData, depthVP), biasMatrix);
}
void CLightComponent::UpdatePos()
{
	auto &pos = GetEntity().GetPosition();
	if(uvec::cmp(pos, reinterpret_cast<Vector3 &>(m_bufferData.position)) == false) {
		reinterpret_cast<Vector3 &>(m_bufferData.position) = pos;
		if(m_renderBuffer)
			c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, position), m_bufferData.position);
		umath::set_flag(m_stateFlags, StateFlags::FullUpdateRequired);
	}
}
void CLightComponent::UpdateDir()
{
	pragma::LightType lightType;
	GetLight(lightType);
	if(lightType != pragma::LightType::Point) {
		auto &rot = GetEntity().GetRotation();
		auto dir = uquat::forward(rot);
		if(uvec::cmp(dir, reinterpret_cast<Vector3 &>(m_bufferData.direction)) == false) {
			reinterpret_cast<Vector3 &>(m_bufferData.direction) = dir;
			if(m_bufferData.direction.x == 0.f && m_bufferData.direction.y == 0.f && m_bufferData.direction.z == 0.f)
				m_bufferData.direction.z = 1.f;
			if(m_renderBuffer)
				c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, direction), m_bufferData.direction);
			umath::set_flag(m_stateFlags, StateFlags::FullUpdateRequired);
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
	if(m_renderBuffer != nullptr)
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, color), m_bufferData.color);

	if(color.a == 0 || (color.r == 0 && color.g == 0 && color.b == 0))
		umath::set_flag(m_bufferData.flags, LightBufferData::BufferFlags::TurnedOn, false);
	else {
		auto pToggleComponent = GetEntity().GetComponent<CToggleComponent>();
		if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn() == true)
			umath::set_flag(m_bufferData.flags, LightBufferData::BufferFlags::TurnedOn, true);
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
	if(m_renderBuffer != nullptr)
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, position) + offsetof(Vector4, w), m_bufferData.position.w);
	umath::set_flag(m_stateFlags, StateFlags::FullUpdateRequired);
}
void CLightComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	CBaseLightComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CTransformComponent)) {
		auto &trC = static_cast<CTransformComponent &>(component);
		FlagCallbackForRemoval(trC.AddEventCallback(CTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this, &trC](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			                         if(umath::is_flag_set(static_cast<pragma::CEOnPoseChanged &>(evData.get()).changeFlags, pragma::TransformChangeFlags::PositionChanged))
				                         UpdatePos();

			                         if(umath::is_flag_set(static_cast<pragma::CEOnPoseChanged &>(evData.get()).changeFlags, pragma::TransformChangeFlags::RotationChanged))
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
		if(m_renderBuffer != nullptr)
			c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, flags), m_bufferData.flags);
	}
	else if(typeid(component) == typeid(CLightPointComponent)) {
		m_bufferData.flags &= ~(LightBufferData::BufferFlags::TypeSpot | LightBufferData::BufferFlags::TypePoint | LightBufferData::BufferFlags::TypeDirectional);
		m_bufferData.flags |= LightBufferData::BufferFlags::TypePoint;
		if(m_renderBuffer != nullptr)
			c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, flags), m_bufferData.flags);
	}
	else if(typeid(component) == typeid(CLightDirectionalComponent)) {
		m_bufferData.flags &= ~(LightBufferData::BufferFlags::TypeSpot | LightBufferData::BufferFlags::TypePoint | LightBufferData::BufferFlags::TypeDirectional);
		m_bufferData.flags |= LightBufferData::BufferFlags::TypeDirectional;
		if(m_renderBuffer != nullptr)
			c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, flags), m_bufferData.flags);
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

	if(umath::is_flag_set(m_lightFlags, LightFlags::BakedLightSource)) {
		m_bufferData.flags |= LightBufferData::BufferFlags::BakedLightSource;
		if(m_renderBuffer != nullptr)
			c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_renderBuffer, offsetof(LightBufferData, flags), m_bufferData.flags);
	}
	UpdatePos();
	UpdateDir();
	UpdateRadius();
	UpdateColor();
}

const pragma::LightBufferData &CLightComponent::GetBufferData() const { return const_cast<CLightComponent *>(this)->GetBufferData(); }
pragma::LightBufferData &CLightComponent::GetBufferData() { return m_bufferData; }
const pragma::ShadowBufferData *CLightComponent::GetShadowBufferData() const { return const_cast<CLightComponent *>(this)->GetShadowBufferData(); }
pragma::ShadowBufferData *CLightComponent::GetShadowBufferData() { return m_shadowBufferData.get(); }

pragma::ComponentHandle<CShadowComponent> CLightComponent::GetShadowMap(ShadowMapType type) const { return (type == ShadowMapType::Dynamic) ? m_shadowMapDynamic : m_shadowMapStatic; }

pragma::CShadowComponent *CLightComponent::GetShadowComponent() { return m_shadowComponent; }
const pragma::CShadowComponent *CLightComponent::GetShadowComponent() const { return const_cast<CLightComponent *>(this)->GetShadowComponent(); }
bool CLightComponent::HasShadowsEnabled() const { return m_shadowComponent && GetEffectiveShadowType() != ShadowType::None; }

Mat4 &CLightComponent::GetTransformationMatrix(unsigned int j)
{
	CEGetTransformationMatrix evData {j};
	InvokeEventCallbacks(EVENT_GET_TRANSFORMATION_MATRIX, evData);
	if(evData.transformation != nullptr)
		return *evData.transformation;
	static Mat4 m;
	m = umat::identity();
	return m;
}

const std::shared_ptr<prosper::IBuffer> &CLightComponent::GetRenderBuffer() const { return m_renderBuffer; }
const std::shared_ptr<prosper::IBuffer> &CLightComponent::GetShadowBuffer() const { return m_shadowBuffer; }
void CLightComponent::SetRenderBuffer(const std::shared_ptr<prosper::IBuffer> &renderBuffer, bool freeBuffer)
{
	DestroyRenderBuffer(freeBuffer);
	m_renderBuffer = renderBuffer;
}
void CLightComponent::SetShadowBuffer(const std::shared_ptr<prosper::IBuffer> &renderBuffer, bool freeBuffer)
{
	DestroyShadowBuffer(freeBuffer);
	m_shadowBuffer = renderBuffer;
}

///////////////////

void Console::commands::debug_light_sources(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto &context = c_engine->GetRenderContext();
	context.WaitIdle();

	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightComponent>>();
	std::vector<pragma::CLightComponent *> lights;
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
	Con::cout << "Light buffer count: " << lightBufManager.GetLightDataBufferCount() << Con::endl;
	auto numTotal = lightBufManager.GetMaxCount();
	Con::cout << "Max light count: " << numTotal << Con::endl;
	Con::cout << "Allocated buffer instances: " << lightBufManager.GetGlobalRenderBuffer().GetTotalInstanceCount() << Con::endl;

	auto getBufferData = [](prosper::IBuffer &buf) -> LightBufferData {
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
		Con::cout << "\t\tPosition: (" << data.position.x << "," << data.position.y << "," << data.position.z << ")" << Con::endl;
		Con::cout << "\t\tShadow Index: " << data.shadowIndex << Con::endl;
		Con::cout << "\t\tShadow Map Index (static): " << data.shadowMapIndexStatic << Con::endl;
		Con::cout << "\t\tShadow Map Index (dynamic): " << data.shadowMapIndexDynamic << Con::endl;
		Con::cout << "\t\tType: " << type << Con::endl;
		Con::cout << "\t\tColor: (" << data.color.r << "," << data.color.g << "," << data.color.b << ")" << Con::endl;
		Con::cout << "\t\tIntensity (candela): " << data.intensity << Con::endl;
		Con::cout << "\t\tDirection: (" << data.direction.x << "," << data.direction.y << "," << data.direction.z << ")" << Con::endl;
		Con::cout << "\t\tCone Start Offset: " << data.direction.w << Con::endl;
		Con::cout << "\t\tDistance: " << data.position.w << Con::endl;
		Con::cout << "\t\tOuter cone half-angle: " << umath::rad_to_deg(data.outerConeHalfAngle) << Con::endl;
		Con::cout << "\t\tInner cone half-angle: " << umath::rad_to_deg(data.innerConeHalfAngle) << Con::endl;
		Con::cout << "\t\tAttenuation: " << data.attenuation << Con::endl;
		Con::cout << "\t\tFlags: " << umath::to_integral(data.flags) << Con::endl;
		Con::cout << "\t\tTurned On: " << (((data.flags & LightBufferData::BufferFlags::TurnedOn) == LightBufferData::BufferFlags::TurnedOn) ? "Yes" : "No") << Con::endl;
	};

	Con::cout << "Enabled light buffers:" << Con::endl;
	auto &buf = lightBufManager.GetGlobalRenderBuffer();

	auto createInfo = buf.GetCreateInfo();
	createInfo.usageFlags = prosper::BufferUsageFlags::TransferDstBit;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	auto tmpBuf = context.CreateBuffer(createInfo);
	auto setupCmd = context.GetSetupCommandBuffer();
	prosper::util::BufferCopy copyInfo {};
	copyInfo.size = createInfo.size;
	setupCmd->RecordCopyBuffer(copyInfo, buf, *tmpBuf);
	context.FlushSetupCommandBuffer();

	std::vector<uint8_t> bufData;
	bufData.resize(buf.GetSize());
	tmpBuf->Read(0, bufData.size(), bufData.data());

	auto *rawData = bufData.data();
	for(auto i = decltype(numTotal) {0u}; i < numTotal; ++i) {
		util::ScopeGuard sg {[&rawData, &buf]() { rawData += buf.GetStride(); }};
		auto &data = *reinterpret_cast<LightBufferData *>(rawData);
		if(!umath::is_flag_set(data.flags, LightBufferData::BufferFlags::TurnedOn))
			continue;
		Con::cout << "Buffer Index: " << i << Con::endl;
		printBufferData(data);
	}

	Con::cout << Con::endl;
	Con::cout << "Light sources:" << Con::endl;
	auto lightId = 0u;
	for(auto *l : lights) {
		Con::cout << "Light #" << lightId << ":" << Con::endl;
		Con::cout << "\tType: ";
		auto type = pragma::LightType::Undefined;
		auto *pLight = l->GetLight(type);
		switch(type) {
		case pragma::LightType::Directional:
			Con::cout << "Directional";
			break;
		case pragma::LightType::Point:
			Con::cout << "Point";
			break;
		case pragma::LightType::Spot:
			Con::cout << "Spot";
			break;
		default:
			Con::cout << "Unknown";
			break;
		}
		Con::cout << Con::endl;

		auto &buf = l->GetRenderBuffer();
		if(buf == nullptr)
			Con::cout << "\tBuffer: NULL" << Con::endl;
		else {
			auto data = getBufferData(*buf);
			printBufferData(data);
		}
		++lightId;
	}
	Con::cout << "Number of lights: " << numLights << Con::endl;
	Con::cout << "Turned on: " << numTurnedOn << Con::endl;
	if(discrepancies.empty() == false) {
		Con::cwar << "Discrepancies found in " << discrepancies.size() << " lights:" << Con::endl;
		for(auto idx : discrepancies)
			Con::cout << "\t" << idx << Con::endl;
	}
}

/////////////////

CEShouldPassEntity::CEShouldPassEntity(const CBaseEntity &entity, uint32_t &renderFlags) : entity {entity}, renderFlags {renderFlags} {}
void CEShouldPassEntity::PushArguments(lua_State *l) {}

/////////////////

CEShouldPassMesh::CEShouldPassMesh(const Model &model, const CModelSubMesh &mesh) : model {model}, mesh {mesh} {}
void CEShouldPassMesh::PushArguments(lua_State *l) {}

/////////////////

CEShouldPassEntityMesh::CEShouldPassEntityMesh(const CBaseEntity &entity, const CModelMesh &mesh, uint32_t &renderFlags) : entity {entity}, mesh {mesh}, renderFlags {renderFlags} {}
void CEShouldPassEntityMesh::PushArguments(lua_State *l) {}

/////////////////

CEShouldUpdateRenderPass::CEShouldUpdateRenderPass() {}
void CEShouldUpdateRenderPass::PushArguments(lua_State *l) {}

/////////////////

CEGetTransformationMatrix::CEGetTransformationMatrix(uint32_t index) : index {index} {}
void CEGetTransformationMatrix::PushArguments(lua_State *l) {}

/////////////////

CEHandleShadowMap::CEHandleShadowMap() {}
void CEHandleShadowMap::PushArguments(lua_State *l) {}

/////////////////

CEOnShadowBufferInitialized::CEOnShadowBufferInitialized(prosper::IBuffer &shadowBuffer) : shadowBuffer {shadowBuffer} {}
void CEOnShadowBufferInitialized::PushArguments(lua_State *l) { Lua::Push<std::shared_ptr<Lua::Vulkan::Buffer>>(l, shadowBuffer.shared_from_this()); }
