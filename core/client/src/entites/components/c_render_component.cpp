/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/c_softbody_component.hpp"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/model/c_modelmesh.h"
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
#include <pragma/math/intersection.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>

using namespace pragma;

namespace pragma
{
	using ::operator|=;
};

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

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
void CRenderComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_UPDATE_RENDER_DATA_MT = componentManager.RegisterEvent("ON_UPDATE_RENDER_DATA_MT",std::type_index(typeid(CRenderComponent)));
	EVENT_ON_RENDER_BUFFERS_INITIALIZED = componentManager.RegisterEvent("ON_RENDER_BUFFERS_INITIALIZED");
	EVENT_ON_RENDER_BOUNDS_CHANGED = componentManager.RegisterEvent("ON_RENDER_BOUNDS_CHANGED");
	EVENT_ON_RENDER_MODE_CHANGED = componentManager.RegisterEvent("ON_RENDER_MODE_CHANGED");
	EVENT_SHOULD_DRAW = componentManager.RegisterEvent("SHOULD_DRAW",std::type_index(typeid(CRenderComponent)));
	EVENT_SHOULD_DRAW_SHADOW = componentManager.RegisterEvent("SHOULD_DRAW_SHADOW",std::type_index(typeid(CRenderComponent)));
	EVENT_ON_UPDATE_RENDER_BUFFERS = componentManager.RegisterEvent("ON_UPDATE_RENDER_BUFFERS",std::type_index(typeid(CRenderComponent)));
	EVENT_ON_UPDATE_RENDER_MATRICES = componentManager.RegisterEvent("ON_UPDATE_RENDER_MATRICES",std::type_index(typeid(CRenderComponent)));
	EVENT_UPDATE_INSTANTIABILITY = componentManager.RegisterEvent("UPDATE_INSTANTIABILITY");
}
CRenderComponent::CRenderComponent(BaseEntity &ent)
	: BaseRenderComponent(ent)
{}
luabind::object CRenderComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CRenderComponentHandleWrapper>(l);}
void CRenderComponent::InitializeBuffers()
{
	auto instanceSize = sizeof(pragma::ShaderEntity::InstanceData);
	auto instanceCount = 32'768u;
	auto maxInstanceCount = instanceCount *100u;
	prosper::util::BufferCreateInfo createInfo {};
	if constexpr(USE_HOST_MEMORY_FOR_RENDER_DATA)
	{
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::HostAccessable | prosper::MemoryFeatureFlags::HostCoherent;
		createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	}
	else
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	createInfo.size = instanceSize *instanceCount;
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
		throw std::runtime_error{"Unsupported minimum uniform buffer alignment (" +std::to_string(internalAlignment) +"!"};
	s_instanceBuffer = c_engine->GetRenderContext().CreateUniformResizableBuffer(createInfo,instanceSize,instanceSize *maxInstanceCount,0.1f,nullptr,alignment);
	s_instanceBuffer->SetDebugName("entity_instance_data_buf");
	if constexpr(USE_HOST_MEMORY_FOR_RENDER_DATA)
		s_instanceBuffer->SetPermanentlyMapped(true,prosper::IBuffer::MapFlags::WriteBit | prosper::IBuffer::MapFlags::Unsynchronized);

	pragma::initialize_articulated_buffers();
	pragma::initialize_vertex_animation_buffer();
}
const prosper::IBuffer &CRenderComponent::GetRenderBuffer() const {return m_renderBuffer->GetBuffer();}
const std::shared_ptr<prosper::SwapBuffer> &CRenderComponent::GetSwapRenderBuffer() const {return m_renderBuffer;}
std::optional<RenderBufferIndex> CRenderComponent::GetRenderBufferIndex() const {return m_renderBuffer ? m_renderBuffer->GetBuffer().GetBaseIndex() : std::optional<RenderBufferIndex>{};}
prosper::IDescriptorSet *CRenderComponent::GetRenderDescriptorSet() const {return (m_renderDescSetGroup != nullptr) ? &m_renderDescSetGroup->GetDescriptorSet() : nullptr;}
prosper::SwapDescriptorSet *CRenderComponent::GetSwapRenderDescriptorSet() const {return m_renderDescSetGroup.get();}
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
CRenderComponent::StateFlags CRenderComponent::GetStateFlags() const {return m_stateFlags;}
void CRenderComponent::SetDepthPassEnabled(bool enabled) {umath::set_flag(m_stateFlags,StateFlags::EnableDepthPass,enabled);}
bool CRenderComponent::IsDepthPassEnabled() const {return umath::is_flag_set(m_stateFlags,StateFlags::EnableDepthPass);}
void CRenderComponent::SetRenderClipPlane(const Vector4 &plane) {m_renderClipPlane = plane;}
void CRenderComponent::ClearRenderClipPlane() {m_renderClipPlane = {};}
const Vector4 *CRenderComponent::GetRenderClipPlane() const {return m_renderClipPlane.has_value() ? &*m_renderClipPlane : nullptr;}
void CRenderComponent::SetReceiveShadows(bool enabled) {umath::set_flag(m_stateFlags,StateFlags::DisableShadows,!enabled);}
bool CRenderComponent::IsReceivingShadows() const {return !umath::is_flag_set(m_stateFlags,StateFlags::DisableShadows);}
void CRenderComponent::Initialize()
{
	BaseRenderComponent::Initialize();

	BindEventUnhandled(CAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateBoneBuffer();
	});
	BindEventUnhandled(CColorComponent::EVENT_ON_COLOR_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		SetRenderBufferDirty();
	});
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		m_localRenderBounds = {};
		m_absoluteRenderBounds = {};
		m_localRenderSphere = {};
		m_absoluteRenderSphere = {};

		auto &ent = GetEntity();
		auto *mdlComponent = GetModelComponent();
		auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
		if(mdl == nullptr)
		{
			UpdateRenderMeshes();
			BroadcastEvent(EVENT_ON_RENDER_MODE_CHANGED);
			return;
		}

		Vector3 rMin,rMax;
		mdl->GetRenderBounds(rMin,rMax);
		auto pPhysComponent = ent.GetPhysicsComponent();
		auto lorigin = pPhysComponent != nullptr ? pPhysComponent->GetLocalOrigin() : Vector3{};
		rMin += lorigin;
		rMax += lorigin;
		SetLocalRenderBounds(rMin,rMax);

		UpdateRenderMeshes();
		BroadcastEvent(EVENT_ON_RENDER_MODE_CHANGED);
	});
	UpdateInstantiability();
}
CRenderComponent::~CRenderComponent()
{
	ClearRenderObjects();
	auto it = std::find(s_ocExemptEntities.begin(),s_ocExemptEntities.end(),this);
	if(it != s_ocExemptEntities.end())
		s_ocExemptEntities.erase(it);

	if(m_renderBuffer != nullptr)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderBuffer);
	if(m_renderDescSetGroup != nullptr)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderDescSetGroup);
}
void CRenderComponent::OnEntitySpawn()
{
	BaseRenderComponent::OnEntitySpawn();
	UpdateRenderMeshes();

	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::COcclusionCullerComponent>>();
	for(auto *ent : entIt)
	{
		auto occlusionCullerC = ent->GetComponent<pragma::COcclusionCullerComponent>();
		occlusionCullerC->AddEntity(static_cast<CBaseEntity&>(GetEntity()));
	}
}
void CRenderComponent::UpdateAbsoluteRenderBounds()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::RenderBoundsDirty) == false)
		return;
	umath::set_flag(m_stateFlags,StateFlags::RenderBoundsDirty,false);
	UpdateAbsoluteAABBRenderBounds();
	UpdateAbsoluteSphereRenderBounds();
}
void CRenderComponent::UpdateAbsoluteSphereRenderBounds() {m_absoluteRenderSphere = CalcAbsoluteRenderSphere();}
void CRenderComponent::UpdateAbsoluteAABBRenderBounds() {m_absoluteRenderBounds = CalcAbsoluteRenderBounds();}
const bounding_volume::AABB &CRenderComponent::GetLocalRenderBounds() const {return m_localRenderBounds;}
const Sphere &CRenderComponent::GetLocalRenderSphere() const {return m_localRenderSphere;}

const bounding_volume::AABB &CRenderComponent::GetUpdatedAbsoluteRenderBounds() const
{
	const_cast<CRenderComponent*>(this)->UpdateAbsoluteRenderBounds();
	return GetAbsoluteRenderBounds();
}
const Sphere &CRenderComponent::GetUpdatedAbsoluteRenderSphere() const
{
	const_cast<CRenderComponent*>(this)->UpdateAbsoluteRenderBounds();
	return GetAbsoluteRenderSphere();
}

const bounding_volume::AABB &CRenderComponent::GetAbsoluteRenderBounds() const {return m_absoluteRenderBounds;}
const Sphere &CRenderComponent::GetAbsoluteRenderSphere() const {return m_absoluteRenderSphere;}

bounding_volume::AABB CRenderComponent::CalcAbsoluteRenderBounds() const
{
	auto absBounds = m_localRenderBounds;
	auto &min = absBounds.min;
	auto &max = absBounds.max;

	auto &ent = GetEntity();
	auto pose = ent.GetPose();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent)
	{
		auto physType = pPhysComponent->GetPhysicsType();
		if(physType == PHYSICSTYPE::DYNAMIC || physType == PHYSICSTYPE::STATIC)
			pose.SetOrigin(pose.GetOrigin() +pPhysComponent->GetLocalOrigin());
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
	if(pPhysComponent)
	{
		auto physType = pPhysComponent->GetPhysicsType();
		if(physType == PHYSICSTYPE::DYNAMIC || physType == PHYSICSTYPE::STATIC)
			pose.SetOrigin(pose.GetOrigin() +pPhysComponent->GetLocalOrigin());
	}
	auto &scale = pose.GetScale();
	r.radius *= umath::abs_max(scale.x,scale.y,scale.z);
	r.pos = pose *r.pos;
	return r;
}
void CRenderComponent::SetLocalRenderBounds(Vector3 min,Vector3 max)
{
	uvec::to_min_max(min,max);

	if(min == m_localRenderBounds.min && max == m_localRenderBounds.max)
		return;
	umath::set_flag(m_stateFlags,StateFlags::RenderBoundsDirty);
	GetEntity().SetStateFlag(BaseEntity::StateFlags::RenderBoundsChanged);

	m_localRenderBounds = {min,max};
	m_localRenderSphere.pos = (min +max) *0.5f;
	
	auto bounds = (max -min) *0.5f;
	m_localRenderSphere.radius = uvec::length(bounds);

	CEOnRenderBoundsChanged ceData {min,max,m_localRenderSphere};
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

Mat4 &CRenderComponent::GetTransformationMatrix() {return m_matTransformation;}
const umath::ScaledTransform &CRenderComponent::GetRenderPose() const {return m_renderPose;}
void CRenderComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseRenderComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(pragma::CTransformComponent))
	{
		FlagCallbackForRemoval(static_cast<pragma::CTransformComponent&>(component).AddEventCallback(CTransformComponent::EVENT_ON_POSE_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			SetRenderBufferDirty();
			return util::EventReply::Unhandled;
		}),CallbackType::Component,&component);
	}
	else if(typeid(component) == typeid(pragma::CAttachableComponent))
		m_attachableComponent = static_cast<CAttachableComponent*>(&component);
	else if(typeid(component) == typeid(pragma::CAnimatedComponent))
		m_animComponent = static_cast<CAnimatedComponent*>(&component);
	else if(typeid(component) == typeid(pragma::CLightMapReceiverComponent))
		m_lightMapReceiverComponent = static_cast<CLightMapReceiverComponent*>(&component);
}
void CRenderComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseRenderComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(pragma::CAttachableComponent))
		m_attachableComponent = nullptr;
	else if(typeid(component) == typeid(pragma::CAnimatedComponent))
		m_animComponent = nullptr;
	else if(typeid(component) == typeid(pragma::CLightMapReceiverComponent))
		m_lightMapReceiverComponent = nullptr;
}
bool CRenderComponent::IsInstantiable() const {return umath::is_flag_set(m_stateFlags,StateFlags::IsInstantiable);}
void CRenderComponent::SetInstaniationEnabled(bool enabled)
{
	umath::set_flag(m_stateFlags,StateFlags::InstantiationDisabled,!enabled);
	UpdateInstantiability();
}
void CRenderComponent::UpdateInstantiability()
{
	umath::set_flag(m_stateFlags,StateFlags::IsInstantiable,false);
	if(m_renderBuffer == nullptr || umath::is_flag_set(m_stateFlags,StateFlags::InstantiationDisabled))
		return;
	auto instantiable = true;
	BroadcastEvent(EVENT_UPDATE_INSTANTIABILITY,CEUpdateInstantiability{instantiable});
	umath::set_flag(m_stateFlags,StateFlags::IsInstantiable,instantiable);
}
void CRenderComponent::UpdateShouldDrawState()
{
	auto shouldDraw = true;
	BroadcastEvent(EVENT_SHOULD_DRAW,CEShouldDraw{shouldDraw});
	umath::set_flag(m_stateFlags,StateFlags::ShouldDraw,shouldDraw);

	UpdateShouldDrawShadowState();
}
void CRenderComponent::UpdateShouldDrawShadowState()
{
	auto shouldDraw = GetCastShadows();
	if(shouldDraw)
		BroadcastEvent(EVENT_SHOULD_DRAW_SHADOW,CEShouldDraw{shouldDraw});
	umath::set_flag(m_stateFlags,StateFlags::ShouldDrawShadow,shouldDraw);
}
CModelComponent *CRenderComponent::GetModelComponent() const {return static_cast<CModelComponent*>(GetEntity().GetModelComponent());}
CAttachableComponent *CRenderComponent::GetAttachableComponent() const {return m_attachableComponent;}
CAnimatedComponent *CRenderComponent::GetAnimatedComponent() const {return m_animComponent;}
CLightMapReceiverComponent *CRenderComponent::GetLightMapReceiverComponent() const {return m_lightMapReceiverComponent;}
void CRenderComponent::SetRenderOffsetTransform(const umath::ScaledTransform &t) {m_renderOffset = t; SetRenderBufferDirty();}
void CRenderComponent::ClearRenderOffsetTransform() {m_renderOffset = {}; SetRenderBufferDirty();}
const umath::ScaledTransform *CRenderComponent::GetRenderOffsetTransform() const {return m_renderOffset.has_value() ? &*m_renderOffset : nullptr;}
GameShaderSpecialization CRenderComponent::GetShaderPipelineSpecialization() const
{
	if(GetAnimatedComponent())
		return GameShaderSpecialization::Animated;
	if(GetLightMapReceiverComponent())
		return GameShaderSpecialization::Lightmapped;
	return GameShaderSpecialization::Generic;
}
void CRenderComponent::UpdateMatrices()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto orientation = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	umath::ScaledTransform pose {};
	if(pPhysComponent == nullptr || pPhysComponent->GetPhysicsType() != PHYSICSTYPE::SOFTBODY)
	{
		pose.SetOrigin(pPhysComponent != nullptr ? pPhysComponent->GetOrigin() : pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{});
		pose.SetRotation(orientation);
	}
	if(pTrComponent != nullptr)
		pose.SetScale(pTrComponent->GetScale());
	if(m_renderOffset.has_value())
		pose = *m_renderOffset *pose;
	m_matTransformation = pose.ToMatrix();

	CEOnUpdateRenderMatrices evData{pose,m_matTransformation};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_MATRICES,evData);
}
uint64_t CRenderComponent::GetLastRenderFrame() const {return m_lastRender;}
void CRenderComponent::SetLastRenderFrame(unsigned long long &t) {m_lastRender = t;}

void CRenderComponent::UpdateRenderMeshes()
{
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
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
void CRenderComponent::ReceiveData(NetPacket &packet)
{
	m_renderFlags = packet->Read<decltype(m_renderFlags)>();
}
std::optional<Intersection::LineMeshResult> CRenderComponent::CalcRayIntersection(const Vector3 &start,const Vector3 &dir,bool precise) const
{
	auto &lodMeshes = GetLODMeshes();
	if(lodMeshes.empty())
		return {};
	auto &pose = GetEntity().GetPose();
	auto invPose = pose.GetInverse();

	// Move ray into entity space
	auto lstart = invPose *start;
	auto ldir = dir;
	uvec::rotate(&ldir,invPose.GetRotation());

	auto &scale = GetEntity().GetScale();
	lstart /= scale;
	ldir /= scale;

	// Cheap line-aabb check
	auto aabb = GetLocalRenderBounds();
	aabb.min /= scale;
	aabb.max /= scale;
	auto n = ldir;
	auto d = uvec::length(n);
	n /= d;
	float dIntersect;
	if(umath::intersection::line_aabb(lstart,n,aabb.min,aabb.max,&dIntersect) == umath::intersection::Result::NoIntersection || dIntersect > d)
		return {};

	auto mdlC = GetEntity().GetModelComponent();
	auto mdl = mdlC ? mdlC->GetModel() : nullptr;
	if(mdl)
	{
		auto &hitboxes = mdl->GetHitboxes();
		if(hitboxes.empty() == false)
		{
			// We'll assume that there are enough hitboxes to cover the entire model
			Hitbox *closestHitbox = nullptr;
			auto closestHitboxDistance = std::numeric_limits<float>::max();
			uint32_t closestHitboxBoneId = std::numeric_limits<uint32_t>::max();
			for(auto &hb : hitboxes)
			{
				Vector3 min,max,origin;
				Quat rot;
				if(mdlC->GetHitboxBounds(hb.first,min,max,origin,rot) == false)
					continue;
				float dist;
				if(umath::intersection::line_obb(start,dir,min,max,&dist,origin,rot) == false || dist >= closestHitboxDistance)
					continue;
				closestHitboxDistance = dist;
				closestHitbox = &hb.second;
				closestHitboxBoneId = hb.first;
			}
			if(closestHitbox == nullptr)
				return {};
			if(precise == false)
			{
				Intersection::LineMeshResult result {};
				result.hitPos = start +dir *closestHitboxDistance;
				result.hitValue = closestHitboxDistance;
				result.result = umath::intersection::Result::Intersect;
				result.hitbox = closestHitbox;
				result.boneId = closestHitboxBoneId;
				return result;
			}
		}
	}

	std::optional<Intersection::LineMeshResult> bestResult = {};
	for(auto &mesh : lodMeshes)
	{
		Vector3 min,max;
		mesh->GetBounds(min,max);
		if(umath::intersection::line_aabb(lstart,n,min,max,&dIntersect) == umath::intersection::Result::NoIntersection || dIntersect > d)
			continue;
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			subMesh->GetBounds(min,max);
			if(umath::intersection::line_aabb(lstart,n,min,max,&dIntersect) == umath::intersection::Result::NoIntersection || dIntersect > d)
				continue;
			Intersection::LineMeshResult result;
			if(Intersection::LineMesh(lstart,ldir,*subMesh,result,true) == false)
				continue;
			// Confirm that this is the best result so far
			if(bestResult.has_value() && result.hitValue > bestResult->hitValue)
				continue;
			if(result.precise)
				result.precise->mesh = mesh;
			bestResult = result;
		}
	}
	if(bestResult.has_value())
	{
		// Move back to world space
		bestResult->hitPos = pose *bestResult->hitPos;
	}
	return bestResult;
}
void CRenderComponent::SetExemptFromOcclusionCulling(bool exempt)
{
	umath::set_flag(m_stateFlags,StateFlags::ExemptFromOcclusionCulling,exempt);
	auto it = std::find(s_ocExemptEntities.begin(),s_ocExemptEntities.end(),this);
	if(exempt)
	{
		if(it == s_ocExemptEntities.end())
			s_ocExemptEntities.push_back(this);
	}
	else if(it != s_ocExemptEntities.end())
		s_ocExemptEntities.erase(it);
}
bool CRenderComponent::IsExemptFromOcclusionCulling() const {return umath::is_flag_set(m_stateFlags,StateFlags::ExemptFromOcclusionCulling);}
void CRenderComponent::SetRenderBufferDirty() {umath::set_flag(m_stateFlags,StateFlags::RenderBufferDirty);}
void CRenderComponent::SetRenderBoundsDirty() {umath::set_flag(m_stateFlags,StateFlags::RenderBoundsDirty);}
void CRenderComponent::UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,bool bForceBufferUpdate)
{
	InitializeRenderBuffers();
	auto updateRenderBuffer = umath::is_flag_set(m_stateFlags,StateFlags::RenderBufferDirty) || bForceBufferUpdate;
	auto bufferDirty = false;
	if(updateRenderBuffer)
	{
		umath::set_flag(m_stateFlags,StateFlags::RenderBufferDirty,false);
		UpdateMatrices();

		// Update Render Buffer
		Vector4 color(1.f,1.f,1.f,1.f);
		auto pColorComponent = GetEntity().GetComponent<CColorComponent>();
		if(pColorComponent.valid())
			color = pColorComponent->GetColor().ToVector4();

		auto renderFlags = pragma::ShaderEntity::InstanceData::RenderFlags::None;
		auto *pMdlComponent = GetModelComponent();
		auto bWeighted = pMdlComponent && static_cast<const pragma::CModelComponent&>(*pMdlComponent).IsWeighted();
		if(bWeighted == true)
			renderFlags |= pragma::ShaderEntity::InstanceData::RenderFlags::Weighted;
		auto &m = GetTransformationMatrix();
		m_instanceData.modelMatrix = m;
		m_instanceData.color = color;
		m_instanceData.renderFlags = renderFlags;
		bufferDirty = true;
	}
	auto &renderBuffer = GetSwapRenderBuffer();
	if(renderBuffer)
		renderBuffer->Update(0ull,sizeof(m_instanceData),&m_instanceData,bufferDirty);

	CEOnUpdateRenderBuffers evData {drawCmd};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_BUFFERS,evData);
}
void CRenderComponent::UpdateRenderDataMT(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const CSceneComponent &scene,const CCameraComponent &cam,const Mat4 &vp)
{
	m_renderDataMutex.lock();
		// Note: This is called from the render thread, which is why we can't update the render buffers here
		auto frameId = c_engine->GetRenderContext().GetLastFrameId();
		if(m_lastRender == frameId)
		{
			m_renderDataMutex.unlock();
			return; // Only update once per frame
		}
		m_lastRender = frameId;
	m_renderDataMutex.unlock();

	UpdateAbsoluteRenderBounds();

	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto *mdlC = GetModelComponent();
	if(mdlC)
		mdlC->UpdateLOD(scene,cam,vp); // TODO: Don't update this every frame for every entity!

	CEOnUpdateRenderData evData {drawCmd};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_DATA_MT,evData);

	auto pAttComponent = GetAttachableComponent();
	if(pAttComponent)
	{
		auto *attInfo = pAttComponent->GetAttachmentData();
		if(attInfo != nullptr && (attInfo->flags &FAttachmentMode::UpdateEachFrame) != FAttachmentMode::None && attInfo->parent.valid())
			pAttComponent->UpdateAttachmentOffset(false);
	}
}

void CRenderComponent::SetRenderMode(RenderMode mode)
{
	if(mode == m_renderMode)
		return;
	m_renderMode = mode;

	auto it = std::find(s_ocExemptEntities.begin(),s_ocExemptEntities.end(),this);
	if(mode == RenderMode::View)
	{
		if(it == s_ocExemptEntities.end())
			s_ocExemptEntities.push_back(this);
	}
	else if(it != s_ocExemptEntities.end() && IsExemptFromOcclusionCulling() == false)
		s_ocExemptEntities.erase(it);

	if(mode == RenderMode::None)
		ClearRenderBuffers();

	UpdateShouldDrawState();
	BroadcastEvent(EVENT_ON_RENDER_MODE_CHANGED);
}
void CRenderComponent::InitializeRenderBuffers()
{
	// Initialize render buffer if it doesn't exist
	if(m_renderBuffer != nullptr || pragma::ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE.IsValid() == false)
		return;

	m_renderBuffer = prosper::SwapBuffer::Create(*s_instanceBuffer);
	m_renderDescSetGroup = prosper::SwapDescriptorSet::Create(c_engine->GetRenderContext(),pragma::ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE);
	m_renderDescSetGroup->SetBindingUniformBuffer(
		*m_renderBuffer,umath::to_integral(pragma::ShaderGameWorldLightingPass::InstanceBinding::Instance)
	);
	UpdateBoneBuffer();
	m_renderDescSetGroup->Update();
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
	auto wpBoneBuffer = static_cast<pragma::CAnimatedComponent&>(*pAnimComponent).GetSwapBoneBuffer();
	if(!wpBoneBuffer)
		return;
	m_renderDescSetGroup->SetBindingUniformBuffer(
		*wpBoneBuffer,umath::to_integral(pragma::ShaderGameWorldLightingPass::InstanceBinding::BoneMatrices)
	);
}
void CRenderComponent::ClearRenderBuffers()
{
	m_renderBuffer = nullptr;
	m_renderDescSetGroup = nullptr;
}
RenderMode CRenderComponent::GetRenderMode() const {return m_renderMode;}
bool CRenderComponent::ShouldDraw() const {return umath::is_flag_set(m_stateFlags,StateFlags::ShouldDraw);}
bool CRenderComponent::ShouldDrawShadow() const
{
	// TODO: Streamline this! We only need one flag!
	return umath::is_flag_set(m_stateFlags,StateFlags::ShouldDrawShadow) && !umath::is_flag_set(m_stateFlags,StateFlags::DisableShadows) && GetCastShadows();
}

RenderMeshGroup &CRenderComponent::GetLodRenderMeshGroup(uint32_t lod)
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent)
	{
		static RenderMeshGroup meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent&>(*pMdlComponent).GetLodRenderMeshGroup(lod);
}
const RenderMeshGroup &CRenderComponent::GetLodRenderMeshGroup(uint32_t lod) const {return const_cast<CRenderComponent*>(this)->GetLodRenderMeshGroup(lod);}
RenderMeshGroup &CRenderComponent::GetLodMeshGroup(uint32_t lod)
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent)
	{
		static RenderMeshGroup meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent&>(*pMdlComponent).GetLodMeshGroup(lod);
}
const RenderMeshGroup &CRenderComponent::GetLodMeshGroup(uint32_t lod) const {return const_cast<CRenderComponent*>(this)->GetLodMeshGroup(lod);}
const std::vector<std::shared_ptr<ModelSubMesh>> &CRenderComponent::GetRenderMeshes() const {return const_cast<CRenderComponent*>(this)->GetRenderMeshes();}
std::vector<std::shared_ptr<ModelSubMesh>> &CRenderComponent::GetRenderMeshes()
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent)
	{
		static std::vector<std::shared_ptr<ModelSubMesh>> meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent&>(*pMdlComponent).GetRenderMeshes();
}
std::vector<rendering::RenderBufferData> &CRenderComponent::GetRenderBufferData()
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent)
	{
		static std::vector<rendering::RenderBufferData> renderBufferData {};
		return renderBufferData;
	}
	return static_cast<pragma::CModelComponent&>(*pMdlComponent).GetRenderBufferData();
}
const std::vector<std::shared_ptr<ModelMesh>> &CRenderComponent::GetLODMeshes() const {return const_cast<CRenderComponent*>(this)->GetLODMeshes();}
std::vector<std::shared_ptr<ModelMesh>> &CRenderComponent::GetLODMeshes()
{
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pSoftBodyComponent = ent.GetComponent<pragma::CSoftBodyComponent>();
	if(pSoftBodyComponent.valid())
	{
		static std::vector<std::shared_ptr<ModelMesh>> meshes {};
		return meshes;
		// TODO
		//auto *pSoftBodyData = pSoftBodyComponent->GetSoftBodyData();
		//if(pSoftBodyData != nullptr)
		//	return pSoftBodyData->meshes;
	}
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent)
	{
		static std::vector<std::shared_ptr<ModelMesh>> meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent&>(*pMdlComponent).GetLODMeshes();
}
bool CRenderComponent::RenderCallback(RenderObject *o,CBaseEntity *ent,pragma::CCameraComponent *cam,pragma::ShaderGameWorldLightingPass *shader,Material *mat)
{
	auto pRenderComponent = ent->GetRenderComponent();
	return pRenderComponent && pRenderComponent->RenderCallback(o,cam,shader,mat);
}
bool CRenderComponent::RenderCallback(RenderObject*,pragma::CCameraComponent *cam,pragma::ShaderGameWorldLightingPass*,Material*)
{
	return ShouldDraw();
}
const std::vector<CRenderComponent*> &CRenderComponent::GetEntitiesExemptFromOcclusionCulling() {return s_ocExemptEntities;}
const std::shared_ptr<prosper::IUniformResizableBuffer> &CRenderComponent::GetInstanceBuffer() {return s_instanceBuffer;}
void CRenderComponent::ClearBuffers()
{
	s_instanceBuffer = nullptr;
	pragma::clear_articulated_buffers();
	pragma::clear_vertex_animation_buffer();
	CRaytracingComponent::ClearBuffers();
}

/////////////////

CEUpdateInstantiability::CEUpdateInstantiability(bool &instantiable)
	: instantiable{instantiable}
{}
void CEUpdateInstantiability::PushArguments(lua_State *l)
{
	Lua::PushBool(l,instantiable);
}
uint32_t CEUpdateInstantiability::GetReturnCount() {return 1u;}
void CEUpdateInstantiability::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		instantiable = Lua::CheckBool(l,-1);
}

/////////////////

CEShouldDraw::CEShouldDraw(bool &shouldDraw)
	: shouldDraw{shouldDraw}
{}
void CEShouldDraw::PushArguments(lua_State *l)
{
	Lua::PushBool(l,shouldDraw);
}
uint32_t CEShouldDraw::GetReturnCount() {return 1u;}
void CEShouldDraw::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		shouldDraw = Lua::CheckBool(l,-1);
}

/////////////////

CEOnUpdateRenderMatrices::CEOnUpdateRenderMatrices(umath::ScaledTransform &pose,Mat4 &transformation)
	: pose{pose},transformation{transformation}
{}
void CEOnUpdateRenderMatrices::PushArguments(lua_State *l)
{
	Lua::Push<umath::ScaledTransform>(l,pose);
	Lua::Push<Mat4>(l,transformation);
}
uint32_t CEOnUpdateRenderMatrices::GetReturnCount() {return 3;}
void CEOnUpdateRenderMatrices::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-2))
		pose = Lua::Check<umath::ScaledTransform>(l,-2);
	if(Lua::IsSet(l,-1))
		transformation = *Lua::CheckMat4(l,-1);
}

/////////////////

CEOnUpdateRenderData::CEOnUpdateRenderData(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &commandBuffer)
	: commandBuffer{commandBuffer}
{}
void CEOnUpdateRenderData::PushArguments(lua_State *l) {throw std::runtime_error{"Lua callbacks of multi-threaded events are not allowed!"};}

/////////////////

CEOnUpdateRenderBuffers::CEOnUpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &commandBuffer)
	: commandBuffer{commandBuffer}
{}
void CEOnUpdateRenderBuffers::PushArguments(lua_State *l)
{}

/////////////////

CEOnRenderBoundsChanged::CEOnRenderBoundsChanged(const Vector3 &min,const Vector3 &max,const Sphere &sphere)
	: min{min},max{max},sphere{sphere}
{}
void CEOnRenderBoundsChanged::PushArguments(lua_State *l)
{
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
	Lua::Push<Vector3>(l,sphere.pos);
	Lua::PushNumber(l,sphere.radius);
}
