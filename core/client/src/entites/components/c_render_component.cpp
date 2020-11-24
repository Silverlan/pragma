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
ComponentEventId CRenderComponent::EVENT_SHOULD_DRAW = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_SHOULD_DRAW_SHADOW = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_UPDATE_RENDER_BUFFERS = INVALID_COMPONENT_ID;;
ComponentEventId CRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES = INVALID_COMPONENT_ID;
void CRenderComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_UPDATE_RENDER_DATA_MT = componentManager.RegisterEvent("ON_UPDATE_RENDER_DATA_MT",std::type_index(typeid(CRenderComponent)));
	EVENT_ON_RENDER_BUFFERS_INITIALIZED = componentManager.RegisterEvent("ON_RENDER_BUFFERS_INITIALIZED");
	EVENT_ON_RENDER_BOUNDS_CHANGED = componentManager.RegisterEvent("ON_RENDER_BUFFERS_INITIALIZED");
	EVENT_SHOULD_DRAW = componentManager.RegisterEvent("SHOULD_DRAW",std::type_index(typeid(CRenderComponent)));
	EVENT_SHOULD_DRAW_SHADOW = componentManager.RegisterEvent("SHOULD_DRAW_SHADOW",std::type_index(typeid(CRenderComponent)));
	EVENT_ON_UPDATE_RENDER_BUFFERS = componentManager.RegisterEvent("ON_UPDATE_RENDER_BUFFERS",std::type_index(typeid(CRenderComponent)));
	EVENT_ON_UPDATE_RENDER_MATRICES = componentManager.RegisterEvent("ON_UPDATE_RENDER_MATRICES",std::type_index(typeid(CRenderComponent)));
}
CRenderComponent::CRenderComponent(BaseEntity &ent)
	: BaseRenderComponent(ent),m_renderMode(util::TEnumProperty<RenderMode>::Create(RenderMode::Auto))
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
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	s_instanceBuffer = c_engine->GetRenderContext().CreateUniformResizableBuffer(createInfo,instanceSize,instanceSize *maxInstanceCount,0.1f);
	s_instanceBuffer->SetDebugName("entity_instance_data_buf");
	if constexpr(USE_HOST_MEMORY_FOR_RENDER_DATA)
		s_instanceBuffer->SetPermanentlyMapped(true);

	pragma::initialize_articulated_buffers();
}
std::weak_ptr<prosper::IBuffer> CRenderComponent::GetRenderBuffer() const {return m_renderBuffer;}
prosper::IDescriptorSet *CRenderComponent::GetRenderDescriptorSet() const {return (m_renderDescSetGroup != nullptr) ? m_renderDescSetGroup->GetDescriptorSet() : nullptr;}
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
void CRenderComponent::GetDepthBias(float &outConstantFactor,float &outBiasClamp,float &outSlopeFactor) const
{
	outConstantFactor = m_depthBias.constantFactor;
	outBiasClamp = m_depthBias.biasClamp;
	outSlopeFactor = m_depthBias.slopeFactor;
}
void CRenderComponent::SetDepthBias(float constantFactor,float biasClamp,float slopeFactor)
{
	m_depthBias = {constantFactor,biasClamp,slopeFactor};
	umath::set_flag(m_stateFlags,StateFlags::HasDepthBias,(constantFactor != 0.f) || (biasClamp != 0.f) || (slopeFactor != 0.f));
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
		uvec::zero(&m_renderMin);
		uvec::zero(&m_renderMax);
		uvec::zero(&m_renderMinRot);
		uvec::zero(&m_renderMaxRot);

		auto &ent = GetEntity();
		auto &mdlComponent = GetModelComponent();
		auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
		if(mdl == nullptr)
		{
			UpdateRenderMeshes();
			m_renderMode->InvokeCallbacks();
			return;
		}

		Vector3 rMin,rMax;
		mdl->GetRenderBounds(rMin,rMax);
		auto pPhysComponent = ent.GetPhysicsComponent();
		auto lorigin = pPhysComponent != nullptr ? pPhysComponent->GetLocalOrigin() : Vector3{};
		rMin += lorigin;
		rMax += lorigin;
		SetRenderBounds(rMin,rMax);
		UpdateRenderBounds();

		UpdateRenderMeshes();
		m_renderMode->InvokeCallbacks();
	});
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
Sphere CRenderComponent::GetRenderSphereBounds() const
{
	auto r = m_renderSphere;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent != nullptr)
	{
		auto scale = pTrComponent->GetScale();
		r.radius *= umath::abs_max(scale.x,scale.y,scale.z);
	}
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return r;
	auto physType = pPhysComponent->GetPhysicsType();
	if(physType == PHYSICSTYPE::DYNAMIC || physType == PHYSICSTYPE::STATIC)
	{
		auto &lorigin = pPhysComponent->GetLocalOrigin();
		r.pos += lorigin;
	}
	return r;
}
void CRenderComponent::GetAbsoluteRenderBounds(Vector3 &outMin,Vector3 &outMax) const
{
	auto &pose = GetEntity().GetPose();
	GetRenderBounds(&outMin,&outMax);
	outMin = pose *outMin;
	outMax = pose *outMax;
	uvec::to_min_max(outMin,outMax);
}
void CRenderComponent::GetRenderBounds(Vector3 *min,Vector3 *max) const
{
	*min = m_renderMin;
	*max = m_renderMax;
	auto &ent = GetEntity();
	auto ptrComponent = ent.GetTransformComponent();
	if(ptrComponent != nullptr)
	{
		auto &scale = ptrComponent->GetScale();
		*min *= scale;
		*max *= scale;
	}
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	auto physType = pPhysComponent->GetPhysicsType();
	if(physType != PHYSICSTYPE::DYNAMIC && physType != PHYSICSTYPE::STATIC)
		return;
	auto &lorigin = pPhysComponent->GetLocalOrigin();
	*min += lorigin;
	*max += lorigin;
}
void CRenderComponent::GetRotatedRenderBounds(Vector3 *min,Vector3 *max)
{
	*min = m_renderMinRot;
	*max = m_renderMaxRot;
	auto &ent = GetEntity();
	auto ptrComponent = ent.GetTransformComponent();
	if(ptrComponent != nullptr)
	{
		auto &scale = ptrComponent->GetScale();
		*min = scale;
		*max = scale;
	}
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	auto physType = pPhysComponent->GetPhysicsType();
	if(physType != PHYSICSTYPE::DYNAMIC && physType != PHYSICSTYPE::STATIC)
		return;
	auto &lorigin = pPhysComponent->GetLocalOrigin();
	*min += lorigin;
	*max += lorigin;
}
void CRenderComponent::SetRenderBounds(Vector3 min,Vector3 max)
{
	//auto &mdl = GetModel();
	uvec::to_min_max(min,max);
	if(min.x != m_renderMin.x || min.y != m_renderMin.y || min.z != m_renderMin.z || max.x != m_renderMax.x || max.y != m_renderMax.y || max.z != m_renderMax.z)
		GetEntity().SetStateFlag(BaseEntity::StateFlags::RenderBoundsChanged);
	m_renderMin = min;
	m_renderMax = max;
	m_renderSphere.pos = (min +max) *0.5f;
	Vector3 bounds = (max -min) *0.5f;
	m_renderSphere.radius = uvec::length(bounds);

	CEOnRenderBoundsChanged ceData {min,max,m_renderSphere};
	BroadcastEvent(EVENT_ON_RENDER_BOUNDS_CHANGED);
}

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
	else if(typeid(component) == typeid(pragma::CModelComponent))
		m_mdlComponent = component.GetHandle<CModelComponent>();
	else if(typeid(component) == typeid(pragma::CAnimatedComponent))
		m_animComponent = component.GetHandle<CAnimatedComponent>();
	else if(typeid(component) == typeid(pragma::CLightMapReceiverComponent))
		m_lightMapReceiverComponent = component.GetHandle<CLightMapReceiverComponent>();
}
void CRenderComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseRenderComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(pragma::CModelComponent))
		m_mdlComponent = {};
	else if(typeid(component) == typeid(pragma::CAnimatedComponent))
		m_animComponent = {};
	else if(typeid(component) == typeid(pragma::CLightMapReceiverComponent))
		m_lightMapReceiverComponent = {};
}
util::WeakHandle<CModelComponent> &CRenderComponent::GetModelComponent() const {return m_mdlComponent;}
util::WeakHandle<CAnimatedComponent> &CRenderComponent::GetAnimatedComponent() const {return m_animComponent;}
util::WeakHandle<CLightMapReceiverComponent> &CRenderComponent::GetLightMapReceiverComponent() const {return m_lightMapReceiverComponent;}
void CRenderComponent::SetRenderOffsetTransform(const umath::ScaledTransform &t) {m_renderOffset = t; SetRenderBufferDirty();}
void CRenderComponent::ClearRenderOffsetTransform() {m_renderOffset = {}; SetRenderBufferDirty();}
const umath::ScaledTransform *CRenderComponent::GetRenderOffsetTransform() const {return m_renderOffset.has_value() ? &*m_renderOffset : nullptr;}
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
	auto &mdlComponent = GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	m_renderMeshContainer = nullptr;
	if(mdl == nullptr)
		return;
	m_renderMeshContainer = std::make_unique<SortedRenderMeshContainer>(&ent,static_cast<CModelComponent&>(*mdlComponent).GetLODMeshes());
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
	Vector3 min,max;
	GetRenderBounds(&min,&max);
	min /= scale;
	max /= scale;
	auto n = ldir;
	auto d = uvec::length(n);
	n /= d;
	float dIntersect;
	if(Intersection::LineAABB(lstart,n,min,max,&dIntersect) == Intersection::Result::NoIntersection || dIntersect > d)
		return {};

	auto mdlC = GetEntity().GetModelComponent();
	auto mdl = mdlC.valid() ? mdlC->GetModel() : nullptr;
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
				if(Intersection::LineOBB(start,dir,min,max,&dist,origin,rot) == false || dist >= closestHitboxDistance)
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
				result.result = Intersection::Result::Intersect;
				result.hitbox = closestHitbox;
				result.boneId = closestHitboxBoneId;
				return result;
			}
		}
	}

	std::optional<Intersection::LineMeshResult> bestResult = {};
	for(auto &mesh : lodMeshes)
	{
		mesh->GetBounds(min,max);
		if(Intersection::LineAABB(lstart,n,min,max,&dIntersect) == Intersection::Result::NoIntersection || dIntersect > d)
			continue;
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			subMesh->GetBounds(min,max);
			if(Intersection::LineAABB(lstart,n,min,max,&dIntersect) == Intersection::Result::NoIntersection || dIntersect > d)
				continue;
			Intersection::LineMeshResult result;
			if(Intersection::LineMesh(lstart,ldir,*subMesh,result,true) == false)
				continue;
			// Confirm that this is the best result so far
			if(bestResult.has_value() && result.hitValue > bestResult->hitValue)
				continue;
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
void CRenderComponent::UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,bool bForceBufferUpdate)
{
	InitializeRenderBuffers();
	auto updateRenderBuffer = umath::is_flag_set(m_stateFlags,StateFlags::RenderBufferDirty) || bForceBufferUpdate;
	if(updateRenderBuffer)
	{
		umath::set_flag(m_stateFlags,StateFlags::RenderBufferDirty,false);
		UpdateMatrices();
		// Update Render Buffer
		auto wpRenderBuffer = GetRenderBuffer();
		if(wpRenderBuffer.expired() == false)
		{
			auto renderBuffer = wpRenderBuffer.lock();
			Vector4 color(1.f,1.f,1.f,1.f);
			auto pColorComponent = GetEntity().GetComponent<CColorComponent>();
			if(pColorComponent.valid())
				color = pColorComponent->GetColor().ToVector4();

			auto renderFlags = pragma::ShaderEntity::InstanceData::RenderFlags::None;
			auto &pMdlComponent = GetModelComponent();
			auto bWeighted = pMdlComponent.valid() && static_cast<const pragma::CModelComponent&>(*pMdlComponent).IsWeighted();
			if(bWeighted == true)
				renderFlags |= pragma::ShaderEntity::InstanceData::RenderFlags::Weighted;
			auto &m = GetTransformationMatrix();
			pragma::ShaderEntity::InstanceData instanceData {m,color,renderFlags};
			if constexpr(USE_HOST_MEMORY_FOR_RENDER_DATA)
				renderBuffer->Write(0ull,sizeof(instanceData),&instanceData);
			else
				drawCmd->RecordUpdateGenericShaderReadBuffer(*renderBuffer,0ull,sizeof(instanceData),&instanceData); // Note: Command buffer mustn't be in active render pass!
		}
	}

	CEOnUpdateRenderBuffers evData {drawCmd};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_BUFFERS,evData);
}
void CRenderComponent::UpdateRenderDataMT(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const CSceneComponent &scene,const CCameraComponent &cam,const Mat4 &vp)
{
	// Note: This is called from the render thread, which is why we can't update the render buffers here
	auto frameId = c_engine->GetRenderContext().GetLastFrameId();
	if(m_lastRender == frameId)
		return; // Only update once per frame
	m_lastRender = frameId;
	auto &ent = static_cast<CBaseEntity&>(GetEntity());

	auto &mdlC = GetModelComponent();
	if(mdlC.valid())
		mdlC->UpdateLOD(scene,cam,vp); // TODO: Don't update this every frame for every entity!

	CEOnUpdateRenderData evData {drawCmd};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_DATA_MT,evData);

	auto pAttComponent = ent.GetComponent<CAttachableComponent>();
	if(pAttComponent.valid())
	{
		auto *attInfo = pAttComponent->GetAttachmentData();
		if(attInfo != nullptr && (attInfo->flags &FAttachmentMode::UpdateEachFrame) != FAttachmentMode::None && attInfo->parent.valid())
			pAttComponent->UpdateAttachmentOffset(false);
	}
}

void CRenderComponent::SetRenderMode(RenderMode mode)
{
	if(mode == **m_renderMode)
		return;
	*m_renderMode = mode;

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
}
void CRenderComponent::InitializeRenderBuffers()
{
	// Initialize render buffer if it doesn't exist
	if(m_renderBuffer != nullptr || pragma::ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE.IsValid() == false)
		return;
	m_renderBuffer = s_instanceBuffer->AllocateBuffer();
	m_renderDescSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE);
	m_renderDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(
		*m_renderBuffer,umath::to_integral(pragma::ShaderTextured3DBase::InstanceBinding::Instance)
	);
	UpdateBoneBuffer();
	m_renderDescSetGroup->GetDescriptorSet()->Update();

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
	auto wpBoneBuffer = static_cast<pragma::CAnimatedComponent&>(*pAnimComponent).GetBoneBuffer();
	if(wpBoneBuffer.expired())
		return;
	m_renderDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(
		*wpBoneBuffer.lock(),umath::to_integral(pragma::ShaderTextured3DBase::InstanceBinding::BoneMatrices)
	);
}
void CRenderComponent::ClearRenderBuffers()
{
	m_renderBuffer = nullptr;
	m_renderDescSetGroup = nullptr;
}
const util::PEnumProperty<RenderMode> &CRenderComponent::GetRenderModeProperty() const {return m_renderMode;}
RenderMode CRenderComponent::GetRenderMode() const
{
	if(*m_renderMode == RenderMode::Auto)
	{
		auto &mdlComponent = GetModelComponent();
		auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
		if(mdl == nullptr)
			return RenderMode::None;
		return RenderMode::World;
	}
	return *m_renderMode;
}
bool CRenderComponent::ShouldDraw(const Vector3 &camOrigin) const
{
	auto &ent = static_cast<const CBaseEntity&>(GetEntity());
	auto &mdlComponent = GetModelComponent();
	if(mdlComponent.expired() || mdlComponent->HasModel() == false)
		return false;
	CEShouldDraw evData {camOrigin};
	InvokeEventCallbacks(EVENT_SHOULD_DRAW,evData);
	return (evData.shouldDraw == CEShouldDraw::ShouldDraw::No) ? false : true;
}
bool CRenderComponent::ShouldDrawShadow(const Vector3 &camOrigin) const
{
	if(GetCastShadows() == false)
		return false;
	CEShouldDraw evData {camOrigin};
	InvokeEventCallbacks(EVENT_SHOULD_DRAW_SHADOW,evData);
	return (evData.shouldDraw == CEShouldDraw::ShouldDraw::No) ? false : true;
}

RenderMeshGroup &CRenderComponent::GetLodRenderMeshGroup(uint32_t lod)
{
	auto &pMdlComponent = GetModelComponent();
	if(pMdlComponent.expired())
	{
		static RenderMeshGroup meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent&>(*pMdlComponent).GetLodRenderMeshGroup(lod);
}
const RenderMeshGroup &CRenderComponent::GetLodRenderMeshGroup(uint32_t lod) const {return const_cast<CRenderComponent*>(this)->GetLodRenderMeshGroup(lod);}
RenderMeshGroup &CRenderComponent::GetLodMeshGroup(uint32_t lod)
{
	auto &pMdlComponent = GetModelComponent();
	if(pMdlComponent.expired())
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
	auto &pMdlComponent = GetModelComponent();
	if(pMdlComponent.expired())
	{
		static std::vector<std::shared_ptr<ModelSubMesh>> meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent&>(*pMdlComponent).GetRenderMeshes();
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
	auto &pMdlComponent = GetModelComponent();
	if(pMdlComponent.expired())
	{
		static std::vector<std::shared_ptr<ModelMesh>> meshes {};
		return meshes;
	}
	return static_cast<pragma::CModelComponent&>(*pMdlComponent).GetLODMeshes();
}
bool CRenderComponent::RenderCallback(RenderObject *o,CBaseEntity *ent,pragma::CCameraComponent *cam,pragma::ShaderTextured3DBase *shader,Material *mat)
{
	auto pRenderComponent = ent->GetRenderComponent();
	return pRenderComponent && pRenderComponent->RenderCallback(o,cam,shader,mat);
}
bool CRenderComponent::RenderCallback(RenderObject*,pragma::CCameraComponent *cam,pragma::ShaderTextured3DBase*,Material*)
{
	return ShouldDraw(cam->GetEntity().GetPosition());
}
const std::vector<CRenderComponent*> &CRenderComponent::GetEntitiesExemptFromOcclusionCulling() {return s_ocExemptEntities;}
const std::shared_ptr<prosper::IUniformResizableBuffer> &CRenderComponent::GetInstanceBuffer() {return s_instanceBuffer;}
void CRenderComponent::ClearBuffers()
{
	s_instanceBuffer = nullptr;
	pragma::clear_articulated_buffers();
	CRaytracingComponent::ClearBuffers();
}

/////////////////

CEShouldDraw::CEShouldDraw(const Vector3 &camOrigin)
	: camOrigin{camOrigin}
{}
void CEShouldDraw::PushArguments(lua_State *l)
{
	Lua::Push<Vector3>(l,camOrigin);
}
uint32_t CEShouldDraw::GetReturnCount() {return 1u;}
void CEShouldDraw::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		shouldDraw = Lua::CheckBool(l,-1) ? ShouldDraw::Yes : ShouldDraw::No;
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
