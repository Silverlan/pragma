/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/c_softbody_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_attachment_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/entities/components/renderers/c_raytracing_renderer_component.hpp"
#include "pragma/entities/components/intersection_handler_component.hpp"
#include "pragma/entities/components/c_raytracing_component.hpp"
#include "pragma/entities/components/c_bvh_component.hpp"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar_global_functions.h"
#include <pragma/debug/intel_vtune.hpp>
#include <pragma/lua/classes/ldef_mat4.h>
#include <pragma/model/model.h>
#include <pragma/entities/parentinfo.h>
#include <prosper_descriptor_set_group.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <buffers/prosper_swap_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/components/base_child_component.hpp>
#include <pragma/entities/components/parent_component.hpp>
#include <pragma/math/intersection.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_component_manager_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/console/sh_cmd.h>
#include <util_image.hpp>

using namespace pragma;

namespace pragma {
	using ::operator|=;
};

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

static std::shared_ptr<prosper::IUniformResizableBuffer> s_instanceBuffer = nullptr;
decltype(CRenderComponent::s_ocExemptEntities) CRenderComponent::s_ocExemptEntities = {};
ComponentEventId CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA_MT = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_RENDER_MODE_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_SHOULD_DRAW = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_SHOULD_DRAW_SHADOW = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_UPDATE_RENDER_BUFFERS = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_UPDATE_INSTANTIABILITY = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_CLIP_PLANE_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_DEPTH_BIAS_CHANGED = INVALID_COMPONENT_ID;
void CRenderComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_UPDATE_RENDER_DATA_MT = registerEvent("ON_UPDATE_RENDER_DATA_MT", ComponentEventInfo::Type::Explicit);
	EVENT_ON_RENDER_BUFFERS_INITIALIZED = registerEvent("ON_RENDER_BUFFERS_INITIALIZED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_RENDER_BOUNDS_CHANGED = registerEvent("ON_RENDER_BOUNDS_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_RENDER_MODE_CHANGED = registerEvent("ON_RENDER_MODE_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_SHOULD_DRAW = registerEvent("SHOULD_DRAW", ComponentEventInfo::Type::Explicit);
	EVENT_SHOULD_DRAW_SHADOW = registerEvent("SHOULD_DRAW_SHADOW", ComponentEventInfo::Type::Explicit);
	EVENT_ON_UPDATE_RENDER_BUFFERS = registerEvent("ON_UPDATE_RENDER_BUFFERS", ComponentEventInfo::Type::Explicit);
	EVENT_ON_UPDATE_RENDER_MATRICES = registerEvent("ON_UPDATE_RENDER_MATRICES", ComponentEventInfo::Type::Explicit);
	EVENT_UPDATE_INSTANTIABILITY = registerEvent("UPDATE_INSTANTIABILITY", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_CLIP_PLANE_CHANGED = registerEvent("ON_CLIP_PLANE_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_DEPTH_BIAS_CHANGED = registerEvent("ON_DEPTH_BIAS_CHANGED", ComponentEventInfo::Type::Broadcast);
}
void CRenderComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CRenderComponent;

	{
		using TVisible = bool;
		auto memberInfo = create_component_member_info<T, TVisible, +[](const ComponentMemberInfo &, T &component, const TVisible &value) { component.SetHidden(!value); }, +[](const ComponentMemberInfo &, T &component, TVisible &value) { value = !component.IsHidden(); }>("visible", true);
		registerMember(std::move(memberInfo));
	}
}
CRenderComponent::CRenderComponent(BaseEntity &ent)
    : BaseRenderComponent(ent), m_renderGroups {util::TEnumProperty<pragma::rendering::RenderGroup>::Create(pragma::rendering::RenderGroup::None)}, m_renderPass {util::TEnumProperty<pragma::rendering::SceneRenderPass>::Create(rendering::SceneRenderPass::World)}
{
}
void CRenderComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CRenderComponent::InitializeBuffers()
{
	auto instanceSize = sizeof(pragma::ShaderEntity::InstanceData);
	auto instanceCount = 32'768u;
	auto maxInstanceCount = instanceCount * 100u;
	prosper::util::BufferCreateInfo createInfo {};
	if constexpr(USE_HOST_MEMORY_FOR_RENDER_DATA) {
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::HostAccessable | prosper::MemoryFeatureFlags::HostCoherent;
		createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	}
	else
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	createInfo.size = instanceSize * instanceCount;
	createInfo.usageFlags = prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#if ENTITY_RENDER_BUFFER_USE_STORAGE_BUFFER == 0
	createInfo.usageFlags |= prosper::BufferUsageFlags::UniformBufferBit;
#endif
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::StorageBufferBit;
#endif
	constexpr prosper::DeviceSize alignment = 256; // See https://vulkan.gpuinfo.org/displaydevicelimit.php?name=minUniformBufferOffsetAlignment
	auto internalAlignment = c_engine->GetRenderContext().CalcBufferAlignment(prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::StorageBufferBit);
	if(internalAlignment > alignment)
		throw std::runtime_error {"Unsupported minimum uniform buffer alignment (" + std::to_string(internalAlignment) + "!"};
	s_instanceBuffer = c_engine->GetRenderContext().CreateUniformResizableBuffer(createInfo, instanceSize, instanceSize * maxInstanceCount, 0.1f, nullptr, alignment);
	s_instanceBuffer->SetDebugName("entity_instance_data_buf");
	if constexpr(USE_HOST_MEMORY_FOR_RENDER_DATA)
		s_instanceBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit | prosper::IBuffer::MapFlags::Unsynchronized);

	pragma::initialize_articulated_buffers();
	pragma::initialize_vertex_animation_buffer();
}
const prosper::IBuffer *CRenderComponent::GetRenderBuffer() const { return m_renderBuffer.get(); }
std::optional<RenderBufferIndex> CRenderComponent::GetRenderBufferIndex() const { return m_renderBuffer ? m_renderBuffer->GetBaseIndex() : std::optional<RenderBufferIndex> {}; }
prosper::IDescriptorSet *CRenderComponent::GetRenderDescriptorSet() const { return (m_renderDescSetGroup != nullptr) ? m_renderDescSetGroup->GetDescriptorSet() : nullptr; }
void CRenderComponent::ClearRenderObjects()
{
	/*std::unordered_map<unsigned int,RenderInstance*>::iterator it;
	for(it=m_renderInstances.begin();it!=m_renderInstances.end();it++)
	{
		RenderInstance *instance = it->second;
		instance->Remove();
	}
	m_renderInstances.clear();*/ // Vulkan TODO
}
CRenderComponent::StateFlags CRenderComponent::GetStateFlags() const { return m_stateFlags; }
util::EventReply CRenderComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(eventId == BaseChildComponent::EVENT_ON_PARENT_CHANGED) {
		UpdateAncestorHiddenState();
		PropagateHiddenState();
	}
	return BaseRenderComponent::HandleEvent(eventId, evData);
}
void CRenderComponent::SetDepthPassEnabled(bool enabled) { umath::set_flag(m_stateFlags, StateFlags::EnableDepthPass, enabled); }
bool CRenderComponent::IsDepthPassEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::EnableDepthPass); }
void CRenderComponent::SetRenderClipPlane(const Vector4 &plane)
{
	if(plane == m_renderClipPlane)
		return;
	m_renderClipPlane = plane;
	BroadcastEvent(EVENT_ON_CLIP_PLANE_CHANGED);
}
void CRenderComponent::ClearRenderClipPlane()
{
	if(!m_renderClipPlane.has_value())
		return;
	m_renderClipPlane = {};
	BroadcastEvent(EVENT_ON_CLIP_PLANE_CHANGED);
}
const Vector4 *CRenderComponent::GetRenderClipPlane() const { return m_renderClipPlane.has_value() ? &*m_renderClipPlane : nullptr; }
void CRenderComponent::SetDepthBias(float d, float delta)
{
	if(m_depthBias.has_value() && m_depthBias->x == d && m_depthBias->y == delta)
		return;
	m_depthBias = {d, delta};
	BroadcastEvent(EVENT_ON_DEPTH_BIAS_CHANGED);
}
void CRenderComponent::ClearDepthBias()
{
	if(!m_depthBias.has_value())
		return;
	m_depthBias = {};
	BroadcastEvent(EVENT_ON_DEPTH_BIAS_CHANGED);
}
const Vector2 *CRenderComponent::GetDepthBias() const { return m_depthBias.has_value() ? &*m_depthBias : nullptr; }
void CRenderComponent::SetReceiveShadows(bool enabled) { umath::set_flag(m_stateFlags, StateFlags::DisableShadows, !enabled); }
bool CRenderComponent::IsReceivingShadows() const { return !umath::is_flag_set(m_stateFlags, StateFlags::DisableShadows); }
void CRenderComponent::Initialize()
{
	BaseRenderComponent::Initialize();

	BindEventUnhandled(CAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateBoneBuffer(); });
	BindEventUnhandled(CColorComponent::EVENT_ON_COLOR_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { SetRenderBufferDirty(); });
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		m_localRenderBounds = {};
		m_absoluteRenderBounds = {};
		m_localRenderSphere = {};
		m_absoluteRenderSphere = {};

		auto &ent = GetEntity();
		auto *mdlComponent = GetModelComponent();
		auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
		if(mdl == nullptr) {
			UpdateRenderMeshes();
			BroadcastEvent(EVENT_ON_RENDER_MODE_CHANGED);
			return;
		}

		Vector3 rMin, rMax;
		mdl->GetRenderBounds(rMin, rMax);
		auto pPhysComponent = ent.GetPhysicsComponent();
		auto lorigin = pPhysComponent != nullptr ? pPhysComponent->GetLocalOrigin() : Vector3 {};
		rMin += lorigin;
		rMax += lorigin;
		SetLocalRenderBounds(rMin, rMax);

		UpdateRenderMeshes();
		BroadcastEvent(EVENT_ON_RENDER_MODE_CHANGED);
	});
	UpdateInstantiability();
	UpdateAncestorHiddenState();
	PropagateHiddenState();
}
CRenderComponent::~CRenderComponent()
{
	ClearRenderObjects();
	auto it = std::find(s_ocExemptEntities.begin(), s_ocExemptEntities.end(), this);
	if(it != s_ocExemptEntities.end())
		s_ocExemptEntities.erase(it);

	if(m_renderBuffer != nullptr)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderBuffer);
	if(m_renderDescSetGroup != nullptr)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderDescSetGroup);
}
void CRenderComponent::OnRemove()
{
	BaseRenderComponent::OnRemove();
	if(GetEntity().IsRemoved())
		return;
	SetHidden(false);
}
void CRenderComponent::OnEntitySpawn()
{
	BaseRenderComponent::OnEntitySpawn();
	UpdateRenderMeshes();

	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::COcclusionCullerComponent>>();
	for(auto *ent : entIt) {
		auto occlusionCullerC = ent->GetComponent<pragma::COcclusionCullerComponent>();
		occlusionCullerC->AddEntity(static_cast<CBaseEntity &>(GetEntity()));
	}

	InitializeRenderBuffers();
}
void CRenderComponent::UpdateAbsoluteRenderBounds()
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::RenderBoundsDirty) == false)
		return;
	umath::set_flag(m_stateFlags, StateFlags::RenderBoundsDirty, false);
	UpdateAbsoluteAABBRenderBounds();
	UpdateAbsoluteSphereRenderBounds();
}
void CRenderComponent::UpdateAbsoluteSphereRenderBounds() { m_absoluteRenderSphere = CalcAbsoluteRenderSphere(); }
void CRenderComponent::UpdateAbsoluteAABBRenderBounds() { m_absoluteRenderBounds = CalcAbsoluteRenderBounds(); }
const bounding_volume::AABB &CRenderComponent::GetLocalRenderBounds() const { return m_localRenderBounds; }
const Sphere &CRenderComponent::GetLocalRenderSphere() const { return m_localRenderSphere; }

const bounding_volume::AABB &CRenderComponent::GetUpdatedAbsoluteRenderBounds() const
{
	const_cast<CRenderComponent *>(this)->UpdateAbsoluteRenderBounds();
	return GetAbsoluteRenderBounds();
}
const Sphere &CRenderComponent::GetUpdatedAbsoluteRenderSphere() const
{
	const_cast<CRenderComponent *>(this)->UpdateAbsoluteRenderBounds();
	return GetAbsoluteRenderSphere();
}

const bounding_volume::AABB &CRenderComponent::GetAbsoluteRenderBounds() const { return m_absoluteRenderBounds; }
const Sphere &CRenderComponent::GetAbsoluteRenderSphere() const { return m_absoluteRenderSphere; }

bounding_volume::AABB CRenderComponent::CalcAbsoluteRenderBounds() const
{
	auto absBounds = m_localRenderBounds;
	auto &min = absBounds.min;
	auto &max = absBounds.max;

	auto &ent = GetEntity();
	auto pose = ent.GetPose();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent) {
		auto physType = pPhysComponent->GetPhysicsType();
		if(physType == PHYSICSTYPE::DYNAMIC || physType == PHYSICSTYPE::STATIC)
			pose.SetOrigin(pose.GetOrigin() + pPhysComponent->GetLocalOrigin());
	}
	absBounds = absBounds.Transform(pose);
	return absBounds;
}
Sphere CRenderComponent::CalcAbsoluteRenderSphere() const
{
	auto r = m_localRenderSphere;

	auto &ent = GetEntity();
	auto pose = ent.GetPose();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent) {
		auto physType = pPhysComponent->GetPhysicsType();
		if(physType == PHYSICSTYPE::DYNAMIC || physType == PHYSICSTYPE::STATIC)
			pose.SetOrigin(pose.GetOrigin() + pPhysComponent->GetLocalOrigin());
	}
	auto &scale = pose.GetScale();
	r.radius *= umath::abs_max(scale.x, scale.y, scale.z);
	r.pos = pose * r.pos;
	return r;
}
void CRenderComponent::SetLocalRenderBounds(Vector3 min, Vector3 max)
{
	uvec::to_min_max(min, max);

	if(min == m_localRenderBounds.min && max == m_localRenderBounds.max)
		return;
	umath::set_flag(m_stateFlags, StateFlags::RenderBoundsDirty);
	GetEntity().SetStateFlag(BaseEntity::StateFlags::RenderBoundsChanged);

	if(uvec::distance_sqr(min, max) > 0.001f) {
		// If the render bounds form a plane, we'll add a slight width to it to
		// prevent potential culling errors.
		for(uint8_t i = 0; i < 3; ++i) {
			if(max[i] < min[i])
				continue;
			constexpr float minWidth = 0.01f;
			auto diff = minWidth - (max[i] - min[i]);
			if(diff > 0.f) {
				max[i] += diff / 2.f;
				min[i] -= diff / 2.f;
			}
		}
	}

	m_localRenderBounds = {min, max};
	m_localRenderSphere.pos = (min + max) * 0.5f;

	auto bounds = (max - min) * 0.5f;
	m_localRenderSphere.radius = uvec::length(bounds);

	CEOnRenderBoundsChanged ceData {min, max, m_localRenderSphere};
	BroadcastEvent(EVENT_ON_RENDER_BOUNDS_CHANGED);
}

#if 0
void CRenderComponent::UpdateRenderBounds()
{
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	auto *phys = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || pPhysComponent->GetPhysicsType() != PHYSICSTYPE::SOFTBODY || !phys->IsSoftBody())
		AABB::GetRotatedBounds(m_renderMin,m_renderMax,Mat4{m_renderPose.GetRotation()},&m_renderMinRot,&m_renderMaxRot); // TODO: Use orientation
	else
	{
		phys->GetAABB(m_renderMin,m_renderMax);
		m_renderMinRot = m_renderMin;
		m_renderMaxRot = m_renderMax;
	}
}
#endif

Mat4 &CRenderComponent::GetTransformationMatrix() { return m_matTransformation; }
const umath::ScaledTransform &CRenderComponent::GetRenderPose() const { return m_renderPose; }
void CRenderComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseRenderComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(pragma::CTransformComponent)) {
		FlagCallbackForRemoval(static_cast<pragma::CTransformComponent &>(component).AddEventCallback(CTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			                         SetRenderBufferDirty();
			                         SetRenderBoundsDirty();
			                         return util::EventReply::Unhandled;
		                         }),
		  CallbackType::Component, &component);
	}
	else if(typeid(component) == typeid(pragma::CAttachmentComponent))
		m_attachmentComponent = static_cast<CAttachmentComponent *>(&component);
	else if(typeid(component) == typeid(pragma::CAnimatedComponent))
		m_animComponent = static_cast<CAnimatedComponent *>(&component);
	else if(typeid(component) == typeid(pragma::CLightMapReceiverComponent)) {
		m_stateFlags |= StateFlags::RenderBufferDirty;
		m_lightMapReceiverComponent = static_cast<CLightMapReceiverComponent *>(&component);
	}
}
void CRenderComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseRenderComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(pragma::CAttachmentComponent))
		m_attachmentComponent = nullptr;
	else if(typeid(component) == typeid(pragma::CAnimatedComponent))
		m_animComponent = nullptr;
	else if(typeid(component) == typeid(pragma::CLightMapReceiverComponent)) {
		m_stateFlags &= ~StateFlags::RenderBufferDirty;
		m_lightMapReceiverComponent = nullptr;
	}
}
bool CRenderComponent::IsInstantiable() const { return umath::is_flag_set(m_stateFlags, StateFlags::IsInstantiable); }
void CRenderComponent::SetInstaniationEnabled(bool enabled)
{
	umath::set_flag(m_stateFlags, StateFlags::InstantiationDisabled, !enabled);
	UpdateInstantiability();
}
void CRenderComponent::UpdateInstantiability()
{
	umath::set_flag(m_stateFlags, StateFlags::IsInstantiable, false);
	if(m_renderBuffer == nullptr || umath::is_flag_set(m_stateFlags, StateFlags::InstantiationDisabled))
		return;
	auto instantiable = true;
	BroadcastEvent(EVENT_UPDATE_INSTANTIABILITY, CEUpdateInstantiability {instantiable});
	umath::set_flag(m_stateFlags, StateFlags::IsInstantiable, instantiable);
}
void CRenderComponent::UpdateShouldDrawState()
{
	auto shouldDraw = true;
	BroadcastEvent(EVENT_SHOULD_DRAW, CEShouldDraw {shouldDraw});
	umath::set_flag(m_stateFlags, StateFlags::ShouldDraw, shouldDraw);

	UpdateShouldDrawShadowState();
}
void CRenderComponent::UpdateShouldDrawShadowState()
{
	auto shouldDraw = GetCastShadows();
	if(shouldDraw)
		BroadcastEvent(EVENT_SHOULD_DRAW_SHADOW, CEShouldDraw {shouldDraw});
	umath::set_flag(m_stateFlags, StateFlags::ShouldDrawShadow, shouldDraw);
}
CModelComponent *CRenderComponent::GetModelComponent() const { return static_cast<CModelComponent *>(GetEntity().GetModelComponent()); }
CAttachmentComponent *CRenderComponent::GetAttachmentComponent() const { return m_attachmentComponent; }
CAnimatedComponent *CRenderComponent::GetAnimatedComponent() const { return m_animComponent; }
CLightMapReceiverComponent *CRenderComponent::GetLightMapReceiverComponent() const { return m_lightMapReceiverComponent; }
void CRenderComponent::SetRenderOffsetTransform(const umath::ScaledTransform &t)
{
	m_renderOffset = t;
	SetRenderBufferDirty();
}
void CRenderComponent::ClearRenderOffsetTransform()
{
	m_renderOffset = {};
	SetRenderBufferDirty();
}
const umath::ScaledTransform *CRenderComponent::GetRenderOffsetTransform() const { return m_renderOffset.has_value() ? &*m_renderOffset : nullptr; }
bool CRenderComponent::IsInPvs(const Vector3 &camPos) const
{
	for(auto &c : c_game->GetWorldComponents()) {
		if(c.expired())
			continue;
		if(IsInPvs(camPos, static_cast<const CWorldComponent &>(*c)))
			return true;
	}
	return false;
}
bool CRenderComponent::IsInPvs(const Vector3 &camPos, const CWorldComponent &world) const
{
	auto &bspTree = world.GetBSPTree();
	if(!bspTree)
		return true;
	auto *leafNode = bspTree->FindLeafNode(camPos);
	if(!leafNode)
		return false;
	auto &renderBounds = GetUpdatedAbsoluteRenderBounds();
	return bspTree->IsAabbVisibleInCluster(renderBounds.min, renderBounds.max, leafNode->cluster);
}
GameShaderSpecialization CRenderComponent::GetShaderPipelineSpecialization() const
{
	auto *lmC = GetLightMapReceiverComponent();
	if(lmC && !lmC->GetMeshLightMapUvData().empty())
		return GameShaderSpecialization::Lightmapped;
	auto *animC = GetAnimatedComponent();
	if(animC)
		return GameShaderSpecialization::Animated;
	return GameShaderSpecialization::Generic;
}
void CRenderComponent::UpdateMatrices()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto orientation = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	umath::ScaledTransform pose {};
	if(pPhysComponent == nullptr || pPhysComponent->GetPhysicsType() != PHYSICSTYPE::SOFTBODY) {
		pose.SetOrigin(pPhysComponent != nullptr ? pPhysComponent->GetOrigin() : pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {});
		pose.SetRotation(orientation);
	}
	if(pTrComponent != nullptr)
		pose.SetScale(pTrComponent->GetScale());
	if(m_renderOffset.has_value())
		pose = *m_renderOffset * pose;
	m_matTransformation = pose.ToMatrix();

	CEOnUpdateRenderMatrices evData {pose, m_matTransformation};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_MATRICES, evData);
}
uint64_t CRenderComponent::GetLastRenderFrame() const { return m_lastRender; }
void CRenderComponent::SetLastRenderFrame(unsigned long long &t) { m_lastRender = t; }

void CRenderComponent::UpdateRenderMeshes()
{
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	if(!ent.IsSpawned())
		return;
	c_game->UpdateEntityModel(&ent);
	auto *mdlComponent = GetModelComponent();
	auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
#if 0
	m_renderMeshContainer = nullptr;
	if(mdl == nullptr)
		return;
	m_renderMeshContainer = std::make_unique<SortedRenderMeshContainer>(&ent,static_cast<CModelComponent&>(*mdlComponent).GetLODMeshes());
#endif
}
void CRenderComponent::ReceiveData(NetPacket &packet) { m_renderFlags = packet->Read<decltype(m_renderFlags)>(); }
std::optional<Intersection::LineMeshResult> CRenderComponent::CalcRayIntersection(const Vector3 &start, const Vector3 &dir, bool precise) const
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("render_component_calc_ray_intersection");
	util::ScopeGuard sg {[]() { ::debug::get_domain().EndTask(); }};
#endif
	auto &lodMeshes = GetLODMeshes();
	if(lodMeshes.empty())
		return {};
	auto &pose = GetEntity().GetPose();
	auto invPose = pose.GetInverse();
	invPose.SetScale(Vector3 {1.f, 1.f, 1.f});

	// Move ray into entity space
	auto lstart = invPose * start;
	auto ldir = dir;
	uvec::rotate(&ldir, invPose.GetRotation());

	auto &scale = GetEntity().GetScale();
	lstart /= scale;
	ldir /= scale;

	// Cheap line-aabb check
	auto aabb = GetLocalRenderBounds();
	auto n = ldir;
	auto d = uvec::length(n);
	n /= d;
	float dIntersect;
	if(umath::intersection::line_aabb(lstart, n, aabb.min, aabb.max, &dIntersect) == umath::intersection::Result::NoIntersection || dIntersect > d)
		return {};

	auto *mdlC = GetModelComponent();
	auto *intersectionHandlerC = mdlC ? mdlC->GetIntersectionHandlerComponent() : nullptr;
	if(intersectionHandlerC) {
		auto res = intersectionHandlerC->IntersectionTest(lstart, n, umath::CoordinateSpace::Object, 0.f, d);
		if(!res.has_value())
			return {};
		Intersection::LineMeshResult result {};
		result.hitPos = start + uvec::get_normal(dir) * res->distance;
		result.hitValue = res->distance;
		result.result = umath::intersection::Result::Intersect;
		if(precise) {
			result.precise = std::make_shared<Intersection::LineMeshResult::Precise>();
			result.precise->subMesh = res->mesh;
			result.precise->triIdx = res->primitiveIndex;
			result.precise->u = res->u;
			result.precise->v = res->v;
			result.precise->t = res->t;
		}
		return result;
	}

	auto mdl = mdlC ? mdlC->GetModel() : nullptr;
	if(mdl) {
		auto &hitboxes = mdl->GetHitboxes();
		if(hitboxes.empty() == false) {
			// We'll assume that there are enough hitboxes to cover the entire model
			Hitbox *closestHitbox = nullptr;
			auto closestHitboxDistance = std::numeric_limits<float>::max();
			uint32_t closestHitboxBoneId = std::numeric_limits<uint32_t>::max();
			for(auto &hb : hitboxes) {
				Vector3 min, max, origin;
				Quat rot;
				if(mdlC->GetHitboxBounds(hb.first, min, max, origin, rot) == false || uvec::length_sqr(min) < 0.001f || uvec::length_sqr(max) < 0.001f)
					continue;
				float dist;
				if(umath::intersection::line_obb(start, dir, min, max, &dist, origin, rot) == false || dist >= closestHitboxDistance)
					continue;
				closestHitboxDistance = dist;
				closestHitbox = &hb.second;
				closestHitboxBoneId = hb.first;
			}
			if(closestHitbox == nullptr)
				return {};
			if(precise == false) {
				Intersection::LineMeshResult result {};
				result.hitPos = start + dir * closestHitboxDistance;
				result.hitValue = closestHitboxDistance;
				result.result = umath::intersection::Result::Intersect;
				result.hitbox = closestHitbox;
				result.boneId = closestHitboxBoneId;
				return result;
			}
		}
	}

	std::optional<Intersection::LineMeshResult> bestResult = {};
	for(auto &mesh : lodMeshes) {
		Vector3 min, max;
		mesh->GetBounds(min, max);
		if(umath::intersection::line_aabb(lstart, n, min, max, &dIntersect) == umath::intersection::Result::NoIntersection || dIntersect > d)
			continue;
		for(auto &subMesh : mesh->GetSubMeshes()) {
			subMesh->GetBounds(min, max);
			if(umath::intersection::line_aabb(lstart, n, min, max, &dIntersect) == umath::intersection::Result::NoIntersection || dIntersect > d)
				continue;
			Intersection::LineMeshResult result;
			if(Intersection::LineMesh(lstart, ldir, *subMesh, result, true) == false)
				continue;
			// Confirm that this is the best result so far
			if(bestResult.has_value() && result.hitValue > bestResult->hitValue)
				continue;
			if(result.precise)
				result.precise->mesh = mesh;
			bestResult = result;
		}
	}
	if(bestResult.has_value()) {
		// Move back to world space
		bestResult->hitPos = pose * bestResult->hitPos;
	}
	return bestResult;
}
void CRenderComponent::SetExemptFromOcclusionCulling(bool exempt)
{
	umath::set_flag(m_stateFlags, StateFlags::ExemptFromOcclusionCulling, exempt);
	auto it = std::find(s_ocExemptEntities.begin(), s_ocExemptEntities.end(), this);
	if(exempt) {
		if(it == s_ocExemptEntities.end())
			s_ocExemptEntities.push_back(this);
	}
	else if(it != s_ocExemptEntities.end())
		s_ocExemptEntities.erase(it);
}
bool CRenderComponent::IsExemptFromOcclusionCulling() const { return umath::is_flag_set(m_stateFlags, StateFlags::ExemptFromOcclusionCulling); }
void CRenderComponent::SetRenderBufferDirty() { umath::set_flag(m_stateFlags, StateFlags::RenderBufferDirty); }
void CRenderComponent::SetRenderBoundsDirty() { umath::set_flag(m_stateFlags, StateFlags::RenderBoundsDirty); }
void CRenderComponent::UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, bool bForceBufferUpdate)
{
	// Commented because render buffers must not be initialized on a non-main thread
	// InitializeRenderBuffers();
	auto updateRenderBuffer = umath::is_flag_set(m_stateFlags, StateFlags::RenderBufferDirty) || bForceBufferUpdate;
	auto bufferDirty = false;
	if(updateRenderBuffer) {
		umath::set_flag(m_stateFlags, StateFlags::RenderBufferDirty, false);
		UpdateMatrices();

		// Update Render Buffer
		Vector4 color(1.f, 1.f, 1.f, 1.f);
		auto pColorComponent = GetEntity().GetComponent<CColorComponent>();
		if(pColorComponent.valid())
			color = pColorComponent->GetColor();
		color = Vector4 {uimg::linear_to_srgb(reinterpret_cast<Vector3 &>(color)), color.w};

		auto renderFlags = pragma::ShaderEntity::InstanceData::RenderFlags::None;
		auto *pMdlComponent = GetModelComponent();
		auto bWeighted = pMdlComponent && static_cast<const pragma::CModelComponent &>(*pMdlComponent).IsWeighted();
		auto *animC = GetAnimatedComponent();

		// Note: If the RenderFlags::Weighted flag is set, 'GetShaderPipelineSpecialization' must not return
		// something other than GameShaderSpecialization::Animated, otherwise there may be rendering artifacts.
		// (Usually z-fighting because the prepass and lighting pass shaders will perform different calculations.)
		if(bWeighted == true && animC && !m_lightMapReceiverComponent) // && animC->ShouldUpdateBones())
			renderFlags |= pragma::ShaderEntity::InstanceData::RenderFlags::Weighted;
		auto &m = GetTransformationMatrix();
		m_instanceData.modelMatrix = m;
		m_instanceData.color = color;
		m_instanceData.renderFlags = renderFlags;
		m_instanceData.entityIndex = GetEntity().GetLocalIndex();
		bufferDirty = true;
	}
	if(bufferDirty) {
		constexpr auto pipelineStages = prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit | prosper::PipelineStageFlags::GeometryShaderBit;
		drawCmd->RecordBufferBarrier(*m_renderBuffer, pipelineStages, prosper::PipelineStageFlags::TransferBit, prosper::AccessFlags::UniformReadBit, prosper::AccessFlags::TransferWriteBit);
		drawCmd->RecordUpdateBuffer(*m_renderBuffer, 0, m_instanceData);
		drawCmd->RecordBufferBarrier(*m_renderBuffer, prosper::PipelineStageFlags::TransferBit, pipelineStages, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::UniformReadBit);
	}

	CEOnUpdateRenderBuffers evData {drawCmd};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_BUFFERS, evData);
}
const pragma::ShaderEntity::InstanceData &CRenderComponent::GetInstanceData() const { return m_instanceData; }
void CRenderComponent::UpdateRenderDataMT(const CSceneComponent &scene, const CCameraComponent &cam, const Mat4 &vp)
{
	m_renderDataMutex.lock();
	// Note: This is called from the render thread, which is why we can't update the render buffers here
	auto frameId = c_engine->GetRenderContext().GetLastFrameId();
	if(m_lastRender == frameId) {
		m_renderDataMutex.unlock();
		return; // Only update once per frame
	}
	m_lastRender = frameId;
	m_renderDataMutex.unlock();

	UpdateAbsoluteRenderBounds();

	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	auto *mdlC = GetModelComponent();
	if(mdlC)
		mdlC->UpdateLOD(scene, cam, vp); // TODO: Don't update this every frame for every entity!

	CEOnUpdateRenderData evData {};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_DATA_MT, evData);

	auto pAttComponent = GetAttachmentComponent();
	if(pAttComponent) {
		auto *attInfo = pAttComponent->GetAttachmentData();
		if(attInfo != nullptr && (attInfo->flags & FAttachmentMode::UpdateEachFrame) != FAttachmentMode::None && GetEntity().GetParent())
			pAttComponent->UpdateAttachmentOffset(false);
	}
}

bool CRenderComponent::AddToRenderGroup(const std::string &name)
{
	auto mask = static_cast<CGame *>(GetEntity().GetNetworkState()->GetGameState())->GetRenderMask(name);
	if(!mask.has_value())
		return false;
	AddToRenderGroup(*mask);
	return true;
}
const util::PEnumProperty<pragma::rendering::SceneRenderPass> &CRenderComponent::GetSceneRenderPassProperty() const { return m_renderPass; }
pragma::rendering::SceneRenderPass CRenderComponent::GetSceneRenderPass() const { return *m_renderPass; }
void CRenderComponent::SetHidden(bool hidden)
{
	if(hidden == IsHidden())
		return;
	umath::set_flag(m_stateFlags, StateFlags::Hidden, hidden);
	PropagateHiddenState();
	UpdateVisibility();
}
bool CRenderComponent::IsHidden() const
{
	if(ShouldIgnoreAncestorVisibility())
		return umath::is_flag_set(m_stateFlags, StateFlags::Hidden);
	return umath::is_flag_set(m_stateFlags, StateFlags::Hidden | StateFlags::AncestorHidden);
}
bool CRenderComponent::IsVisible() const { return !IsHidden() && *m_renderPass != pragma::rendering::SceneRenderPass::None; }
void CRenderComponent::SetIgnoreAncestorVisibility(bool ignoreVisibility)
{
	umath::set_flag(m_stateFlags, StateFlags::IgnoreAncestorVisibility, ignoreVisibility);
	PropagateHiddenState();
	UpdateVisibility();
}
bool CRenderComponent::ShouldIgnoreAncestorVisibility() const { return umath::is_flag_set(m_stateFlags, StateFlags::IgnoreAncestorVisibility); }
void CRenderComponent::UpdateAncestorHiddenState()
{
	auto parentHidden = false;
	if(!ShouldIgnoreAncestorVisibility()) {
		auto *entParent = GetEntity().GetParent();
		while(entParent) {
			auto renderC = entParent->GetComponent<CRenderComponent>();
			if(renderC.valid()) {
				if(renderC->IsHidden()) {
					parentHidden = true;
					break;
				}
				if(renderC->ShouldIgnoreAncestorVisibility())
					break;
			}
			entParent = entParent->GetParent();
		}
	}
	if(umath::is_flag_set(m_stateFlags, StateFlags::AncestorHidden) == parentHidden)
		return;
	umath::set_flag(m_stateFlags, StateFlags::AncestorHidden, parentHidden);
	UpdateVisibility();
}
void CRenderComponent::PropagateHiddenState()
{
	auto hidden = IsHidden();
	std::function<void(BaseEntity &)> propagate = nullptr;
	propagate = [&propagate, hidden](BaseEntity &ent) {
		auto parentC = ent.GetComponent<ParentComponent>();
		if(parentC.expired())
			return;
		for(auto &hChild : parentC->GetChildren()) {
			if(hChild.expired())
				continue;
			auto renderC = hChild->GetEntity().GetComponent<CRenderComponent>();
			if(renderC.valid()) {
				umath::set_flag(renderC->m_stateFlags, StateFlags::AncestorHidden, hidden);
				renderC->UpdateVisibility();
			}
			propagate(hChild->GetEntity());
		}
	};
	propagate(GetEntity());
}
void CRenderComponent::UpdateVisibility()
{
	if(IsHidden())
		ClearRenderBuffers();
	else if(GetEntity().IsSpawned())
		InitializeRenderBuffers();
}
void CRenderComponent::SetSceneRenderPass(pragma::rendering::SceneRenderPass pass)
{
	*m_renderPass = pass;

	auto it = std::find(s_ocExemptEntities.begin(), s_ocExemptEntities.end(), this);
	switch(pass) {
	case rendering::SceneRenderPass::View:
		{
			if(it == s_ocExemptEntities.end())
				s_ocExemptEntities.push_back(this);
			break;
		}
	default:
		{
			if(it != s_ocExemptEntities.end() && IsExemptFromOcclusionCulling() == false)
				s_ocExemptEntities.erase(it);
		}
	}

	UpdateVisibility();
}
bool CRenderComponent::IsInRenderGroup(pragma::rendering::RenderGroup group) const { return umath::is_flag_set(GetRenderGroups(), group); }
void CRenderComponent::AddToRenderGroup(pragma::rendering::RenderGroup group) { SetRenderGroups(GetRenderGroups() | group); }
bool CRenderComponent::RemoveFromRenderGroup(const std::string &name)
{
	auto mask = static_cast<CGame *>(GetEntity().GetNetworkState()->GetGameState())->GetRenderMask(name);
	if(!mask.has_value())
		return false;
	RemoveFromRenderGroup(*mask);
	return true;
}
void CRenderComponent::RemoveFromRenderGroup(pragma::rendering::RenderGroup group) { SetRenderGroups(GetRenderGroups() & ~group); }

void CRenderComponent::SetRenderGroups(pragma::rendering::RenderGroup mode)
{
	if(mode == *m_renderGroups)
		return;
	*m_renderGroups = mode;

	UpdateShouldDrawState();
	BroadcastEvent(EVENT_ON_RENDER_MODE_CHANGED);
}
void CRenderComponent::InitializeRenderBuffers()
{
	// Initialize render buffer if it doesn't exist
	if(m_renderBuffer != nullptr || pragma::ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE.IsValid() == false || *m_renderPass == rendering::SceneRenderPass::None)
		return;

	c_engine->GetRenderContext().WaitIdle();
	umath::set_flag(m_stateFlags, StateFlags::RenderBufferDirty);
	m_renderBuffer = s_instanceBuffer->AllocateBuffer();
	if(!m_renderBuffer)
		return;
	m_renderDescSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE);
	m_renderDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(*m_renderBuffer, umath::to_integral(pragma::ShaderGameWorldLightingPass::InstanceBinding::Instance));
	UpdateBoneBuffer();
	m_renderDescSetGroup->GetDescriptorSet()->Update();
	UpdateInstantiability();

	BroadcastEvent(EVENT_ON_RENDER_BUFFERS_INITIALIZED);
}
void CRenderComponent::UpdateBoneBuffer()
{
	if(m_renderBuffer == nullptr)
		return;
	auto &ent = GetEntity();
	auto pAnimComponent = ent.GetAnimatedComponent();
	if(pAnimComponent.expired())
		return;
	auto *buf = static_cast<pragma::CAnimatedComponent &>(*pAnimComponent).GetBoneBuffer();
	if(!buf)
		return;
	c_engine->GetRenderContext().WaitIdle();
	m_renderDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(const_cast<prosper::IBuffer &>(*buf), umath::to_integral(pragma::ShaderGameWorldLightingPass::InstanceBinding::BoneMatrices));
	m_renderDescSetGroup->GetDescriptorSet()->Update();
}
void CRenderComponent::ClearRenderBuffers()
{
	if(m_renderBuffer)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderBuffer);
	m_renderBuffer = nullptr;

	if(m_renderDescSetGroup)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderDescSetGroup);
	m_renderDescSetGroup = nullptr;
}
pragma::rendering::RenderGroup CRenderComponent::GetRenderGroups() const { return *m_renderGroups; }
const util::PEnumProperty<pragma::rendering::RenderGroup> &CRenderComponent::GetRenderGroupsProperty() const { return m_renderGroups; }
bool CRenderComponent::ShouldDraw() const { return m_renderBuffer && umath::is_flag_set(m_stateFlags, StateFlags::ShouldDraw); }
bool CRenderComponent::ShouldDrawShadow() const
{
	// TODO: Streamline this! We only need one flag!
	return m_renderBuffer && umath::is_flag_set(m_stateFlags, StateFlags::ShouldDrawShadow) && !umath::is_flag_set(m_stateFlags, StateFlags::DisableShadows) && GetCastShadows();
}

RenderMeshGroup &CRenderComponent::GetLodRenderMeshGroup(uint32_t lod)
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static RenderMeshGroup meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent &>(*pMdlComponent).GetLodRenderMeshGroup(lod);
}
const RenderMeshGroup &CRenderComponent::GetLodRenderMeshGroup(uint32_t lod) const { return const_cast<CRenderComponent *>(this)->GetLodRenderMeshGroup(lod); }
RenderMeshGroup &CRenderComponent::GetLodMeshGroup(uint32_t lod)
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static RenderMeshGroup meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent &>(*pMdlComponent).GetLodMeshGroup(lod);
}
const RenderMeshGroup &CRenderComponent::GetLodMeshGroup(uint32_t lod) const { return const_cast<CRenderComponent *>(this)->GetLodMeshGroup(lod); }
const std::vector<std::shared_ptr<ModelSubMesh>> &CRenderComponent::GetRenderMeshes() const { return const_cast<CRenderComponent *>(this)->GetRenderMeshes(); }
std::vector<std::shared_ptr<ModelSubMesh>> &CRenderComponent::GetRenderMeshes()
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static std::vector<std::shared_ptr<ModelSubMesh>> meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent &>(*pMdlComponent).GetRenderMeshes();
}
std::vector<rendering::RenderBufferData> &CRenderComponent::GetRenderBufferData()
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static std::vector<rendering::RenderBufferData> renderBufferData {};
		return renderBufferData;
	}
	return static_cast<pragma::CModelComponent &>(*pMdlComponent).GetRenderBufferData();
}
const std::vector<std::shared_ptr<ModelMesh>> &CRenderComponent::GetLODMeshes() const { return const_cast<CRenderComponent *>(this)->GetLODMeshes(); }
std::vector<std::shared_ptr<ModelMesh>> &CRenderComponent::GetLODMeshes()
{
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	auto pSoftBodyComponent = ent.GetComponent<pragma::CSoftBodyComponent>();
	if(pSoftBodyComponent.valid()) {
		static std::vector<std::shared_ptr<ModelMesh>> meshes {};
		return meshes;
		// TODO
		//auto *pSoftBodyData = pSoftBodyComponent->GetSoftBodyData();
		//if(pSoftBodyData != nullptr)
		//	return pSoftBodyData->meshes;
	}
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static std::vector<std::shared_ptr<ModelMesh>> meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent &>(*pMdlComponent).GetLODMeshes();
}
bool CRenderComponent::RenderCallback(RenderObject *o, CBaseEntity *ent, pragma::CCameraComponent *cam, pragma::ShaderGameWorldLightingPass *shader, Material *mat)
{
	auto pRenderComponent = ent->GetRenderComponent();
	return pRenderComponent && pRenderComponent->RenderCallback(o, cam, shader, mat);
}
bool CRenderComponent::RenderCallback(RenderObject *, pragma::CCameraComponent *cam, pragma::ShaderGameWorldLightingPass *, Material *) { return ShouldDraw(); }
const std::vector<CRenderComponent *> &CRenderComponent::GetEntitiesExemptFromOcclusionCulling() { return s_ocExemptEntities; }
const std::shared_ptr<prosper::IUniformResizableBuffer> &CRenderComponent::GetInstanceBuffer() { return s_instanceBuffer; }
void CRenderComponent::ClearBuffers()
{
	s_instanceBuffer = nullptr;
	pragma::clear_articulated_buffers();
	pragma::clear_vertex_animation_buffer();
	CRaytracingComponent::ClearBuffers();
}
void CRenderComponent::SetTranslucencyPassDistanceOverride(double distance) { m_translucencyPassDistanceOverrideSqr = umath::pow2(distance); }
void CRenderComponent::ClearTranslucencyPassDistanceOverride() { m_translucencyPassDistanceOverrideSqr = {}; }
const std::optional<double> &CRenderComponent::GetTranslucencyPassDistanceOverrideSqr() const { return m_translucencyPassDistanceOverrideSqr; }

/////////////////

CEUpdateInstantiability::CEUpdateInstantiability(bool &instantiable) : instantiable {instantiable} {}
void CEUpdateInstantiability::PushArguments(lua_State *l) { Lua::PushBool(l, instantiable); }
uint32_t CEUpdateInstantiability::GetReturnCount() { return 1u; }
void CEUpdateInstantiability::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		instantiable = Lua::CheckBool(l, -1);
}

/////////////////

CEShouldDraw::CEShouldDraw(bool &shouldDraw) : shouldDraw {shouldDraw} {}
void CEShouldDraw::PushArguments(lua_State *l) { Lua::PushBool(l, shouldDraw); }
uint32_t CEShouldDraw::GetReturnCount() { return 1u; }
void CEShouldDraw::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		shouldDraw = Lua::CheckBool(l, -1);
}

/////////////////

CEOnUpdateRenderMatrices::CEOnUpdateRenderMatrices(umath::ScaledTransform &pose, Mat4 &transformation) : pose {pose}, transformation {transformation} {}
void CEOnUpdateRenderMatrices::PushArguments(lua_State *l)
{
	Lua::Push<umath::ScaledTransform>(l, pose);
	Lua::Push<Mat4>(l, transformation);
}
uint32_t CEOnUpdateRenderMatrices::GetReturnCount() { return 3; }
void CEOnUpdateRenderMatrices::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -2))
		pose = Lua::Check<umath::ScaledTransform>(l, -2);
	if(Lua::IsSet(l, -1))
		transformation = *Lua::CheckMat4(l, -1);
}

/////////////////

CEOnUpdateRenderData::CEOnUpdateRenderData() {}
void CEOnUpdateRenderData::PushArguments(lua_State *l) { throw std::runtime_error {"Lua callbacks of multi-threaded events are not allowed!"}; }

/////////////////

CEOnUpdateRenderBuffers::CEOnUpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &commandBuffer) : commandBuffer {commandBuffer} {}
void CEOnUpdateRenderBuffers::PushArguments(lua_State *l) {}

/////////////////

CEOnRenderBoundsChanged::CEOnRenderBoundsChanged(const Vector3 &min, const Vector3 &max, const Sphere &sphere) : min {min}, max {max}, sphere {sphere} {}
void CEOnRenderBoundsChanged::PushArguments(lua_State *l)
{
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
	Lua::Push<Vector3>(l, sphere.pos);
	Lua::PushNumber(l, sphere.radius);
}

/////////////////

void pragma::rendering::RenderBufferData::SetDepthPrepassEnabled(bool enabled) { umath::set_flag(stateFlags, StateFlags::EnableDepthPrepass, enabled); }
bool pragma::rendering::RenderBufferData::IsDepthPrepassEnabled() const { return umath::is_flag_set(stateFlags, StateFlags::EnableDepthPrepass); }

void pragma::rendering::RenderBufferData::SetGlowPassEnabled(bool enabled) { umath::set_flag(stateFlags, StateFlags::EnableGlowPass, enabled); }
bool pragma::rendering::RenderBufferData::IsGlowPassEnabled() const { return umath::is_flag_set(stateFlags, StateFlags::EnableGlowPass); }

void Console::commands::debug_entity_render_buffer(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	auto ents = command::find_target_entity(state, *charComponent, argv);
	if(ents.empty()) {
		Con::cwar << "No target entity found!" << Con::endl;
		return;
	}
	auto *ent = ents.front();
	auto mdlC = ent->GetComponent<pragma::CModelComponent>();
	if(mdlC.expired()) {
		Con::cwar << "Target entity has no model component!" << Con::endl;
	}
	else {
		auto vdata = mdlC->GetRenderBufferData();
		if(vdata.empty()) {
			Con::cwar << "No render buffer data found!" << Con::endl;
			return;
		}
		size_t lod = 0;
		for(auto &data : vdata) {
			Con::cout << "Render buffer data for LOD " << lod << ": " << Con::endl;
			Con::cout << "pipelineSpecializationFlags: " << magic_enum::enum_name(data.pipelineSpecializationFlags) << Con::endl;
			Con::cout << "material: ";
			if(data.material)
				Con::cout << data.material->GetName();
			else
				Con::cout << "NULL";
			Con::cout << Con::endl;
			Con::cout << "stateFlags: " << magic_enum::enum_name(data.stateFlags) << Con::endl;
		}
		Con::cout << Con::endl;
	}

	auto renderC = ent->GetComponent<pragma::CRenderComponent>();
	if(renderC.expired()) {
		Con::cwar << "Target entity has no render component!" << Con::endl;
	}
	else {
		auto printInstanceData = [](const pragma::ShaderEntity::InstanceData &instanceData) {
			Con::cout << "modelMatrix: " << umat::to_string(instanceData.modelMatrix) << Con::endl;
			Con::cout << "color: " << instanceData.color << Con::endl;
			Con::cout << "renderFlags: " << magic_enum::enum_name(instanceData.renderFlags) << Con::endl;
			Con::cout << "entityIndex: " << instanceData.entityIndex << Con::endl;
			Con::cout << "padding: " << instanceData.padding << Con::endl;
		};
		auto &instanceData = renderC->GetInstanceData();
		Con::cout << "Instance data:" << Con::endl;
		printInstanceData(instanceData);

		auto *buf = renderC->GetRenderBuffer();
		pragma::ShaderEntity::InstanceData bufData;
		if(!buf || !buf->Read(0, sizeof(bufData), &bufData))
			Con::cwar << "Failed to read buffer data!" << Con::endl;
		else {
			if(memcmp(&instanceData, &bufData, sizeof(bufData)) != 0) {
				Con::cwar << "Instance data does not match data in buffer! Data in buffer:" << Con::endl;
				printInstanceData(bufData);
			}
		}
		Con::cout << Con::endl;
	}
}
