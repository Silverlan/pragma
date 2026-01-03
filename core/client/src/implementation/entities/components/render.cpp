// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.render;
import :engine;
import :entities.components.color;
import :entities.components.game_occlusion_culler;
import :entities.components.raytracing;
import :entities.components.soft_body;
import :entities.components.transform;
import :entities.components.vertex_animated;
import :entities.components.world;
import :game;

using namespace pragma;

static std::shared_ptr<prosper::IUniformResizableBuffer> s_instanceBuffer = nullptr;
decltype(CRenderComponent::s_ocExemptEntities) CRenderComponent::s_ocExemptEntities = {};
ComponentEventId cRenderComponent::EVENT_ON_UPDATE_RENDER_DATA_MT = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_ON_RENDER_MODE_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_SHOULD_DRAW = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_SHOULD_DRAW_SHADOW = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_ON_UPDATE_RENDER_BUFFERS = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_UPDATE_INSTANTIABILITY = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_ON_CLIP_PLANE_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId cRenderComponent::EVENT_ON_DEPTH_BIAS_CHANGED = INVALID_COMPONENT_ID;
void CRenderComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	cRenderComponent::EVENT_ON_UPDATE_RENDER_DATA_MT = registerEvent("ON_UPDATE_RENDER_DATA_MT", ComponentEventInfo::Type::Explicit);
	cRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED = registerEvent("ON_RENDER_BUFFERS_INITIALIZED", ComponentEventInfo::Type::Broadcast);
	cRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED = registerEvent("ON_RENDER_BOUNDS_CHANGED", ComponentEventInfo::Type::Broadcast);
	cRenderComponent::EVENT_ON_RENDER_MODE_CHANGED = registerEvent("ON_RENDER_MODE_CHANGED", ComponentEventInfo::Type::Broadcast);
	cRenderComponent::EVENT_SHOULD_DRAW = registerEvent("SHOULD_DRAW", ComponentEventInfo::Type::Explicit);
	cRenderComponent::EVENT_SHOULD_DRAW_SHADOW = registerEvent("SHOULD_DRAW_SHADOW", ComponentEventInfo::Type::Explicit);
	cRenderComponent::EVENT_ON_UPDATE_RENDER_BUFFERS = registerEvent("ON_UPDATE_RENDER_BUFFERS", ComponentEventInfo::Type::Explicit);
	cRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES = registerEvent("ON_UPDATE_RENDER_MATRICES", ComponentEventInfo::Type::Explicit);
	cRenderComponent::EVENT_UPDATE_INSTANTIABILITY = registerEvent("UPDATE_INSTANTIABILITY", ComponentEventInfo::Type::Broadcast);
	cRenderComponent::EVENT_ON_CLIP_PLANE_CHANGED = registerEvent("ON_CLIP_PLANE_CHANGED", ComponentEventInfo::Type::Broadcast);
	cRenderComponent::EVENT_ON_DEPTH_BIAS_CHANGED = registerEvent("ON_DEPTH_BIAS_CHANGED", ComponentEventInfo::Type::Broadcast);
}
void CRenderComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = CRenderComponent;

	{
		using TVisible = bool;
		auto memberInfo = create_component_member_info<T, TVisible, +[](const ComponentMemberInfo &, T &component, const TVisible &value) { component.SetHidden(!value); }, +[](const ComponentMemberInfo &, T &component, TVisible &value) { value = !component.IsHidden(); }>("visible", true);
		registerMember(std::move(memberInfo));
	}
}
CRenderComponent::CRenderComponent(ecs::BaseEntity &ent) : BaseRenderComponent(ent), m_renderGroups {util::TEnumProperty<rendering::RenderGroup>::Create(rendering::RenderGroup::None)}, m_renderPass {util::TEnumProperty<rendering::SceneRenderPass>::Create(rendering::SceneRenderPass::World)}
{
}
void CRenderComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CRenderComponent::InitializeBuffers()
{
	auto instanceSize = sizeof(rendering::InstanceData);
	auto instanceCount = 32'768u;
	auto maxInstanceCount = instanceCount * 100u;
	prosper::util::BufferCreateInfo createInfo {};
	if constexpr(USE_HOST_MEMORY_FOR_RENDER_DATA) {
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::HostAccessable | prosper::MemoryFeatureFlags::HostCoherent;
		createInfo.flags = createInfo.flags | prosper::util::BufferCreateInfo::Flags::Persistent;
	}
	else
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	createInfo.size = instanceSize * instanceCount;
	createInfo.usageFlags = prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#if ENTITY_RENDER_BUFFER_USE_STORAGE_BUFFER == 0
	createInfo.usageFlags = createInfo.usageFlags | prosper::BufferUsageFlags::UniformBufferBit;
#endif
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags = createInfo.usageFlags | prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::StorageBufferBit;
#endif
	constexpr prosper::DeviceSize alignment = 256; // See https://vulkan.gpuinfo.org/displaydevicelimit.php?name=minUniformBufferOffsetAlignment
	auto internalAlignment = get_cengine()->GetRenderContext().CalcBufferAlignment(prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::StorageBufferBit);
	if(internalAlignment > alignment)
		throw std::runtime_error {"Unsupported minimum uniform buffer alignment (" + std::to_string(internalAlignment) + "!"};
	s_instanceBuffer = get_cengine()->GetRenderContext().CreateUniformResizableBuffer(createInfo, instanceSize, instanceSize * maxInstanceCount, 0.1f, nullptr, alignment);
	s_instanceBuffer->SetDebugName("entity_instance_data_buf");
	if constexpr(USE_HOST_MEMORY_FOR_RENDER_DATA)
		s_instanceBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit | prosper::IBuffer::MapFlags::Unsynchronized);

	initialize_articulated_buffers();
	initialize_vertex_animation_buffer();
}
const prosper::IBuffer *CRenderComponent::GetRenderBuffer() const { return m_renderBuffer.get(); }
std::optional<RenderBufferIndex> CRenderComponent::GetRenderBufferIndex() const { return m_renderBuffer ? m_renderBuffer->GetBaseIndex() : std::optional<RenderBufferIndex> {}; }
prosper::IDescriptorSet *CRenderComponent::GetRenderDescriptorSet() const { return (m_renderDescSetGroup != nullptr) ? m_renderDescSetGroup->GetDescriptorSet() : nullptr; }
CRenderComponent::StateFlags CRenderComponent::GetStateFlags() const { return m_stateFlags; }
util::EventReply CRenderComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(eventId == baseChildComponent::EVENT_ON_PARENT_CHANGED) {
		UpdateAncestorHiddenState();
		PropagateHiddenState();
	}
	return BaseRenderComponent::HandleEvent(eventId, evData);
}
void CRenderComponent::SetDepthPassEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::EnableDepthPass, enabled); }
bool CRenderComponent::IsDepthPassEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::EnableDepthPass); }
void CRenderComponent::SetRenderClipPlane(const Vector4 &plane)
{
	if(plane == m_renderClipPlane)
		return;
	m_renderClipPlane = plane;
	BroadcastEvent(cRenderComponent::EVENT_ON_CLIP_PLANE_CHANGED);
}
void CRenderComponent::ClearRenderClipPlane()
{
	if(!m_renderClipPlane.has_value())
		return;
	m_renderClipPlane = {};
	BroadcastEvent(cRenderComponent::EVENT_ON_CLIP_PLANE_CHANGED);
}
const Vector4 *CRenderComponent::GetRenderClipPlane() const { return m_renderClipPlane.has_value() ? &*m_renderClipPlane : nullptr; }
void CRenderComponent::SetDepthBias(float d, float delta)
{
	if(m_depthBias.has_value() && m_depthBias->x == d && m_depthBias->y == delta)
		return;
	m_depthBias = {d, delta};
	BroadcastEvent(cRenderComponent::EVENT_ON_DEPTH_BIAS_CHANGED);
}
void CRenderComponent::ClearDepthBias()
{
	if(!m_depthBias.has_value())
		return;
	m_depthBias = {};
	BroadcastEvent(cRenderComponent::EVENT_ON_DEPTH_BIAS_CHANGED);
}
const Vector2 *CRenderComponent::GetDepthBias() const { return m_depthBias.has_value() ? &*m_depthBias : nullptr; }
void CRenderComponent::SetReceiveShadows(bool enabled) { math::set_flag(m_stateFlags, StateFlags::DisableShadows, !enabled); }
bool CRenderComponent::IsReceivingShadows() const { return !math::is_flag_set(m_stateFlags, StateFlags::DisableShadows); }
void CRenderComponent::Initialize()
{
	BaseRenderComponent::Initialize();

	BindEventUnhandled(cAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateBoneBuffer(); });
	BindEventUnhandled(cColorComponent::EVENT_ON_COLOR_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { SetRenderBufferDirty(); });
	BindEventUnhandled(cModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		m_localRenderBounds = {};
		m_absoluteRenderBounds = {};
		m_localRenderSphere = {};
		m_absoluteRenderSphere = {};

		auto &ent = GetEntity();
		auto *mdlComponent = GetModelComponent();
		auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
		if(mdl == nullptr) {
			UpdateRenderMeshes();
			BroadcastEvent(cRenderComponent::EVENT_ON_RENDER_MODE_CHANGED);
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
		BroadcastEvent(cRenderComponent::EVENT_ON_RENDER_MODE_CHANGED);
	});
	UpdateInstantiability();
	UpdateAncestorHiddenState();
	PropagateHiddenState();
}
CRenderComponent::~CRenderComponent()
{
	auto it = std::find(s_ocExemptEntities.begin(), s_ocExemptEntities.end(), this);
	if(it != s_ocExemptEntities.end())
		s_ocExemptEntities.erase(it);

	if(m_renderBuffer != nullptr)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderBuffer);
	if(m_renderDescSetGroup != nullptr)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderDescSetGroup);
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

	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<COcclusionCullerComponent>>();
	for(auto *ent : entIt) {
		auto occlusionCullerC = ent->GetComponent<COcclusionCullerComponent>();
		occlusionCullerC->AddEntity(static_cast<ecs::CBaseEntity &>(GetEntity()));
	}

	InitializeRenderBuffers();
}
void CRenderComponent::UpdateAbsoluteRenderBounds()
{
	if(math::is_flag_set(m_stateFlags, StateFlags::RenderBoundsDirty) == false)
		return;
	math::set_flag(m_stateFlags, StateFlags::RenderBoundsDirty, false);
	UpdateAbsoluteAABBRenderBounds();
	UpdateAbsoluteSphereRenderBounds();
}
void CRenderComponent::UpdateAbsoluteSphereRenderBounds() { m_absoluteRenderSphere = CalcAbsoluteRenderSphere(); }
void CRenderComponent::UpdateAbsoluteAABBRenderBounds() { m_absoluteRenderBounds = CalcAbsoluteRenderBounds(); }
const bounding_volume::AABB &CRenderComponent::GetLocalRenderBounds() const { return m_localRenderBounds; }
const math::Sphere &CRenderComponent::GetLocalRenderSphere() const { return m_localRenderSphere; }

const bounding_volume::AABB &CRenderComponent::GetUpdatedAbsoluteRenderBounds() const
{
	const_cast<CRenderComponent *>(this)->UpdateAbsoluteRenderBounds();
	return GetAbsoluteRenderBounds();
}
const math::Sphere &CRenderComponent::GetUpdatedAbsoluteRenderSphere() const
{
	const_cast<CRenderComponent *>(this)->UpdateAbsoluteRenderBounds();
	return GetAbsoluteRenderSphere();
}

const bounding_volume::AABB &CRenderComponent::GetAbsoluteRenderBounds() const { return m_absoluteRenderBounds; }
const math::Sphere &CRenderComponent::GetAbsoluteRenderSphere() const { return m_absoluteRenderSphere; }

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
		if(physType == physics::PhysicsType::Dynamic || physType == physics::PhysicsType::Static)
			pose.SetOrigin(pose.GetOrigin() + pPhysComponent->GetLocalOrigin());
	}
	absBounds = absBounds.Transform(pose);
	return absBounds;
}
math::Sphere CRenderComponent::CalcAbsoluteRenderSphere() const
{
	auto r = m_localRenderSphere;

	auto &ent = GetEntity();
	auto pose = ent.GetPose();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent) {
		auto physType = pPhysComponent->GetPhysicsType();
		if(physType == physics::PhysicsType::Dynamic || physType == physics::PhysicsType::Static)
			pose.SetOrigin(pose.GetOrigin() + pPhysComponent->GetLocalOrigin());
	}
	auto &scale = pose.GetScale();
	r.radius *= math::abs_max(scale.x, scale.y, scale.z);
	r.pos = pose * r.pos;
	return r;
}
void CRenderComponent::SetLocalRenderBounds(Vector3 min, Vector3 max)
{
	uvec::to_min_max(min, max);

	if(min == m_localRenderBounds.min && max == m_localRenderBounds.max)
		return;
	math::set_flag(m_stateFlags, StateFlags::RenderBoundsDirty);
	GetEntity().SetStateFlag(ecs::BaseEntity::StateFlags::RenderBoundsChanged);

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
	BroadcastEvent(cRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED);
}

#if 0
void CRenderComponent::UpdateRenderBounds()
{
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	auto *phys = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || pPhysComponent->GetPhysicsType() != pragma::physics::PhysicsType::SoftBody || !phys->IsSoftBody())
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
const math::ScaledTransform &CRenderComponent::GetRenderPose() const { return m_renderPose; }
void CRenderComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseRenderComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CTransformComponent)) {
		FlagCallbackForRemoval(static_cast<CTransformComponent &>(component).AddEventCallback(cTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			                         SetRenderBufferDirty();
			                         SetRenderBoundsDirty();
			                         return util::EventReply::Unhandled;
		                         }),
		  CallbackType::Component, &component);
	}
	else if(typeid(component) == typeid(CAttachmentComponent))
		m_attachmentComponent = static_cast<CAttachmentComponent *>(&component);
	else if(typeid(component) == typeid(CAnimatedComponent))
		m_animComponent = static_cast<CAnimatedComponent *>(&component);
	else if(typeid(component) == typeid(CLightMapReceiverComponent)) {
		m_stateFlags = m_stateFlags | StateFlags::RenderBufferDirty;
		m_lightMapReceiverComponent = static_cast<CLightMapReceiverComponent *>(&component);
	}
}
void CRenderComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseRenderComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(CAttachmentComponent))
		m_attachmentComponent = nullptr;
	else if(typeid(component) == typeid(CAnimatedComponent))
		m_animComponent = nullptr;
	else if(typeid(component) == typeid(CLightMapReceiverComponent)) {
		m_stateFlags = m_stateFlags & ~StateFlags::RenderBufferDirty;
		m_lightMapReceiverComponent = nullptr;
	}
}
bool CRenderComponent::IsInstantiable() const { return math::is_flag_set(m_stateFlags, StateFlags::IsInstantiable); }
void CRenderComponent::SetInstaniationEnabled(bool enabled)
{
	math::set_flag(m_stateFlags, StateFlags::InstantiationDisabled, !enabled);
	UpdateInstantiability();
}
void CRenderComponent::UpdateInstantiability()
{
	math::set_flag(m_stateFlags, StateFlags::IsInstantiable, false);
	if(m_renderBuffer == nullptr || math::is_flag_set(m_stateFlags, StateFlags::InstantiationDisabled))
		return;
	auto instantiable = true;
	BroadcastEvent(cRenderComponent::EVENT_UPDATE_INSTANTIABILITY, CEUpdateInstantiability {instantiable});
	math::set_flag(m_stateFlags, StateFlags::IsInstantiable, instantiable);
}
void CRenderComponent::UpdateShouldDrawState()
{
	auto shouldDraw = true;
	BroadcastEvent(cRenderComponent::EVENT_SHOULD_DRAW, CEShouldDraw {shouldDraw});
	math::set_flag(m_stateFlags, StateFlags::ShouldDraw, shouldDraw);

	UpdateShouldDrawShadowState();
}
void CRenderComponent::UpdateShouldDrawShadowState()
{
	auto shouldDraw = GetCastShadows();
	if(shouldDraw)
		BroadcastEvent(cRenderComponent::EVENT_SHOULD_DRAW_SHADOW, CEShouldDraw {shouldDraw});
	math::set_flag(m_stateFlags, StateFlags::ShouldDrawShadow, shouldDraw);
}
CModelComponent *CRenderComponent::GetModelComponent() const { return static_cast<CModelComponent *>(GetEntity().GetModelComponent()); }
CAttachmentComponent *CRenderComponent::GetAttachmentComponent() const { return m_attachmentComponent; }
CAnimatedComponent *CRenderComponent::GetAnimatedComponent() const { return m_animComponent; }
CLightMapReceiverComponent *CRenderComponent::GetLightMapReceiverComponent() const { return m_lightMapReceiverComponent; }
void CRenderComponent::SetRenderOffsetTransform(const math::ScaledTransform &t)
{
	m_renderOffset = t;
	SetRenderBufferDirty();
}
void CRenderComponent::ClearRenderOffsetTransform()
{
	m_renderOffset = {};
	SetRenderBufferDirty();
}
const math::ScaledTransform *CRenderComponent::GetRenderOffsetTransform() const { return m_renderOffset.has_value() ? &*m_renderOffset : nullptr; }
bool CRenderComponent::IsInPvs(const Vector3 &camPos) const
{
	for(auto &c : get_cgame()->GetWorldComponents()) {
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
	math::ScaledTransform pose {};
	if(pPhysComponent == nullptr || pPhysComponent->GetPhysicsType() != physics::PhysicsType::SoftBody) {
		pose.SetOrigin(pPhysComponent != nullptr ? pPhysComponent->GetOrigin() : pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {});
		pose.SetRotation(orientation);
	}
	if(pTrComponent != nullptr)
		pose.SetScale(pTrComponent->GetScale());
	if(m_renderOffset.has_value())
		pose = *m_renderOffset * pose;
	m_matTransformation = pose.ToMatrix();

	CEOnUpdateRenderMatrices evData {pose, m_matTransformation};
	InvokeEventCallbacks(cRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES, evData);
}
uint64_t CRenderComponent::GetLastRenderFrame() const { return m_lastRender; }
void CRenderComponent::SetLastRenderFrame(unsigned long long &t) { m_lastRender = t; }

void CRenderComponent::UpdateRenderMeshes()
{
	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	if(!ent.IsSpawned())
		return;
	get_cgame()->UpdateEntityModel(&ent);
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
std::optional<math::intersection::LineMeshResult> CRenderComponent::CalcRayIntersection(const Vector3 &start, const Vector3 &dir, bool precise) const
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("render_component_calc_ray_intersection");
	pragma::util::ScopeGuard sg {[]() { ::debug::get_domain().EndTask(); }};
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
	if(math::intersection::line_aabb(lstart, n, aabb.min, aabb.max, &dIntersect) == math::intersection::Result::NoIntersection || dIntersect > d)
		return {};

	auto *mdlC = GetModelComponent();
	auto *intersectionHandlerC = mdlC ? mdlC->GetIntersectionHandlerComponent() : nullptr;
	if(intersectionHandlerC) {
		auto res = intersectionHandlerC->IntersectionTest(lstart, n, math::CoordinateSpace::Object, 0.f, d);
		if(!res.has_value())
			return {};
		math::intersection::LineMeshResult result {};
		result.hitPos = start + uvec::get_normal(dir) * res->distance;
		result.hitValue = res->distance;
		result.result = math::intersection::Result::Intersect;
		if(precise) {
			result.precise = pragma::util::make_shared<math::intersection::LineMeshResult::Precise>();
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
			physics::Hitbox *closestHitbox = nullptr;
			auto closestHitboxDistance = std::numeric_limits<float>::max();
			uint32_t closestHitboxBoneId = std::numeric_limits<uint32_t>::max();
			for(auto &hb : hitboxes) {
				Vector3 min, max, origin;
				Quat rot;
				if(mdlC->GetHitboxBounds(hb.first, min, max, origin, rot) == false || uvec::length_sqr(min) < 0.001f || uvec::length_sqr(max) < 0.001f)
					continue;
				float dist;
				if(math::intersection::line_obb(start, dir, min, max, &dist, origin, rot) == false || dist >= closestHitboxDistance)
					continue;
				closestHitboxDistance = dist;
				closestHitbox = &hb.second;
				closestHitboxBoneId = hb.first;
			}
			if(closestHitbox == nullptr)
				return {};
			if(precise == false) {
				math::intersection::LineMeshResult result {};
				result.hitPos = start + dir * closestHitboxDistance;
				result.hitValue = closestHitboxDistance;
				result.result = math::intersection::Result::Intersect;
				result.hitbox = closestHitbox;
				result.boneId = closestHitboxBoneId;
				return result;
			}
		}
	}

	std::optional<math::intersection::LineMeshResult> bestResult = {};
	for(auto &mesh : lodMeshes) {
		Vector3 min, max;
		mesh->GetBounds(min, max);
		if(math::intersection::line_aabb(lstart, n, min, max, &dIntersect) == math::intersection::Result::NoIntersection || dIntersect > d)
			continue;
		for(auto &subMesh : mesh->GetSubMeshes()) {
			subMesh->GetBounds(min, max);
			if(math::intersection::line_aabb(lstart, n, min, max, &dIntersect) == math::intersection::Result::NoIntersection || dIntersect > d)
				continue;
			math::intersection::LineMeshResult result;
			if(pragma::math::intersection::line_with_mesh(lstart, ldir, *subMesh, result, true) == false)
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
	math::set_flag(m_stateFlags, StateFlags::ExemptFromOcclusionCulling, exempt);
	auto it = std::find(s_ocExemptEntities.begin(), s_ocExemptEntities.end(), this);
	if(exempt) {
		if(it == s_ocExemptEntities.end())
			s_ocExemptEntities.push_back(this);
	}
	else if(it != s_ocExemptEntities.end())
		s_ocExemptEntities.erase(it);
}
bool CRenderComponent::IsExemptFromOcclusionCulling() const { return math::is_flag_set(m_stateFlags, StateFlags::ExemptFromOcclusionCulling); }
void CRenderComponent::SetRenderBufferDirty() { math::set_flag(m_stateFlags, StateFlags::RenderBufferDirty); }
void CRenderComponent::SetRenderBoundsDirty() { math::set_flag(m_stateFlags, StateFlags::RenderBoundsDirty); }
void CRenderComponent::UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, bool bForceBufferUpdate)
{
	// Commented because render buffers must not be initialized on a non-main thread
	// InitializeRenderBuffers();
	auto updateRenderBuffer = math::is_flag_set(m_stateFlags, StateFlags::RenderBufferDirty) || bForceBufferUpdate;
	auto bufferDirty = false;
	if(updateRenderBuffer) {
		math::set_flag(m_stateFlags, StateFlags::RenderBufferDirty, false);
		UpdateMatrices();

		// Update Render Buffer
		Vector4 color(1.f, 1.f, 1.f, 1.f);
		auto pColorComponent = GetEntity().GetComponent<CColorComponent>();
		if(pColorComponent.valid())
			color = pColorComponent->GetColor();
		color = Vector4 {image::linear_to_srgb(reinterpret_cast<Vector3 &>(color)), color.w};

		auto renderFlags = rendering::InstanceData::RenderFlags::None;
		auto *pMdlComponent = GetModelComponent();
		auto bWeighted = pMdlComponent && static_cast<const CModelComponent &>(*pMdlComponent).IsWeighted();
		auto *animC = GetAnimatedComponent();

		// Note: If the RenderFlags::Weighted flag is set, 'GetShaderPipelineSpecialization' must not return
		// something other than GameShaderSpecialization::Animated, otherwise there may be rendering artifacts.
		// (Usually z-fighting because the prepass and lighting pass shaders will perform different calculations.)
		if(bWeighted == true && animC && !m_lightMapReceiverComponent) // && animC->ShouldUpdateBones())
			renderFlags = renderFlags | rendering::InstanceData::RenderFlags::Weighted;
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
	InvokeEventCallbacks(cRenderComponent::EVENT_ON_UPDATE_RENDER_BUFFERS, evData);
}
const rendering::InstanceData &CRenderComponent::GetInstanceData() const { return m_instanceData; }
void CRenderComponent::UpdateRenderDataMT(const CSceneComponent &scene, const CCameraComponent &cam, const Mat4 &vp)
{
	m_renderDataMutex.lock();
	// Note: This is called from the render thread, which is why we can't update the render buffers here
	auto frameId = get_cengine()->GetRenderContext().GetLastFrameId();
	if(m_lastRender == frameId) {
		m_renderDataMutex.unlock();
		return; // Only update once per frame
	}
	m_lastRender = frameId;
	m_renderDataMutex.unlock();

	UpdateAbsoluteRenderBounds();

	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	auto *mdlC = GetModelComponent();
	if(mdlC)
		mdlC->UpdateLOD(scene, cam, vp); // TODO: Don't update this every frame for every entity!

	CEOnUpdateRenderData evData {};
	InvokeEventCallbacks(cRenderComponent::EVENT_ON_UPDATE_RENDER_DATA_MT, evData);

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
const util::PEnumProperty<rendering::SceneRenderPass> &CRenderComponent::GetSceneRenderPassProperty() const { return m_renderPass; }
rendering::SceneRenderPass CRenderComponent::GetSceneRenderPass() const { return *m_renderPass; }
void CRenderComponent::SetHidden(bool hidden)
{
	if(hidden == IsHidden())
		return;
	math::set_flag(m_stateFlags, StateFlags::Hidden, hidden);
	PropagateHiddenState();
	UpdateVisibility();
}
bool CRenderComponent::IsHidden() const
{
	if(ShouldIgnoreAncestorVisibility())
		return math::is_flag_set(m_stateFlags, StateFlags::Hidden);
	return math::is_flag_set(m_stateFlags, StateFlags::Hidden | StateFlags::AncestorHidden);
}
bool CRenderComponent::IsVisible() const { return !IsHidden() && *m_renderPass != rendering::SceneRenderPass::None; }
void CRenderComponent::SetIgnoreAncestorVisibility(bool ignoreVisibility)
{
	math::set_flag(m_stateFlags, StateFlags::IgnoreAncestorVisibility, ignoreVisibility);
	PropagateHiddenState();
	UpdateVisibility();
}
bool CRenderComponent::ShouldIgnoreAncestorVisibility() const { return math::is_flag_set(m_stateFlags, StateFlags::IgnoreAncestorVisibility); }
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
	if(math::is_flag_set(m_stateFlags, StateFlags::AncestorHidden) == parentHidden)
		return;
	math::set_flag(m_stateFlags, StateFlags::AncestorHidden, parentHidden);
	UpdateVisibility();
}
void CRenderComponent::PropagateHiddenState()
{
	auto hidden = IsHidden();
	std::function<void(ecs::BaseEntity &)> propagate = nullptr;
	propagate = [&propagate, hidden](ecs::BaseEntity &ent) {
		auto parentC = ent.GetComponent<ParentComponent>();
		if(parentC.expired())
			return;
		for(auto &hChild : parentC->GetChildren()) {
			if(hChild.expired())
				continue;
			auto renderC = hChild->GetEntity().GetComponent<CRenderComponent>();
			if(renderC.valid()) {
				math::set_flag(renderC->m_stateFlags, StateFlags::AncestorHidden, hidden);
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
void CRenderComponent::SetSceneRenderPass(rendering::SceneRenderPass pass)
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
bool CRenderComponent::IsInRenderGroup(rendering::RenderGroup group) const { return math::is_flag_set(GetRenderGroups(), group); }
void CRenderComponent::AddToRenderGroup(rendering::RenderGroup group) { SetRenderGroups(GetRenderGroups() | group); }
bool CRenderComponent::RemoveFromRenderGroup(const std::string &name)
{
	auto mask = static_cast<CGame *>(GetEntity().GetNetworkState()->GetGameState())->GetRenderMask(name);
	if(!mask.has_value())
		return false;
	RemoveFromRenderGroup(*mask);
	return true;
}
void CRenderComponent::RemoveFromRenderGroup(rendering::RenderGroup group) { SetRenderGroups(GetRenderGroups() & ~group); }

void CRenderComponent::SetRenderGroups(rendering::RenderGroup mode)
{
	if(mode == *m_renderGroups)
		return;
	*m_renderGroups = mode;

	UpdateShouldDrawState();
	BroadcastEvent(cRenderComponent::EVENT_ON_RENDER_MODE_CHANGED);
}
void CRenderComponent::InitializeRenderBuffers()
{
	// Initialize render buffer if it doesn't exist
	if(m_renderBuffer != nullptr || ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE.IsValid() == false || *m_renderPass == rendering::SceneRenderPass::None)
		return;

	get_cengine()->GetRenderContext().WaitIdle();
	math::set_flag(m_stateFlags, StateFlags::RenderBufferDirty);
	m_renderBuffer = s_instanceBuffer->AllocateBuffer();
	if(!m_renderBuffer)
		return;
	m_renderDescSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderGameWorldLightingPass::DESCRIPTOR_SET_INSTANCE);
	m_renderDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(*m_renderBuffer, math::to_integral(ShaderGameWorldLightingPass::InstanceBinding::Instance));
	UpdateBoneBuffer();
	m_renderDescSetGroup->GetDescriptorSet()->Update();
	UpdateInstantiability();

	BroadcastEvent(cRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED);
}
void CRenderComponent::UpdateBoneBuffer()
{
	if(m_renderBuffer == nullptr)
		return;
	auto &ent = GetEntity();
	auto pAnimComponent = ent.GetAnimatedComponent();
	if(pAnimComponent.expired())
		return;
	auto *buf = static_cast<CAnimatedComponent &>(*pAnimComponent).GetBoneBuffer();
	if(!buf)
		return;
	get_cengine()->GetRenderContext().WaitIdle();
	m_renderDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(const_cast<prosper::IBuffer &>(*buf), math::to_integral(ShaderGameWorldLightingPass::InstanceBinding::BoneMatrices));
	m_renderDescSetGroup->GetDescriptorSet()->Update();
}
void CRenderComponent::ClearRenderBuffers()
{
	if(m_renderBuffer)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderBuffer);
	m_renderBuffer = nullptr;

	if(m_renderDescSetGroup)
		get_cengine()->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_renderDescSetGroup);
	m_renderDescSetGroup = nullptr;
}
rendering::RenderGroup CRenderComponent::GetRenderGroups() const { return *m_renderGroups; }
const util::PEnumProperty<rendering::RenderGroup> &CRenderComponent::GetRenderGroupsProperty() const { return m_renderGroups; }
bool CRenderComponent::ShouldDraw() const { return m_renderBuffer && math::is_flag_set(m_stateFlags, StateFlags::ShouldDraw); }
bool CRenderComponent::ShouldDrawShadow() const
{
	// TODO: Streamline this! We only need one flag!
	return m_renderBuffer && math::is_flag_set(m_stateFlags, StateFlags::ShouldDrawShadow) && !math::is_flag_set(m_stateFlags, StateFlags::DisableShadows) && GetCastShadows();
}

rendering::RenderMeshGroup &CRenderComponent::GetLodRenderMeshGroup(uint32_t lod)
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static rendering::RenderMeshGroup meshes {};
		return meshes;
	}
	return static_cast<CModelComponent &>(*pMdlComponent).GetLodRenderMeshGroup(lod);
}
const rendering::RenderMeshGroup &CRenderComponent::GetLodRenderMeshGroup(uint32_t lod) const { return const_cast<CRenderComponent *>(this)->GetLodRenderMeshGroup(lod); }
rendering::RenderMeshGroup &CRenderComponent::GetLodMeshGroup(uint32_t lod)
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static rendering::RenderMeshGroup meshes {};
		return meshes;
	}
	return static_cast<CModelComponent &>(*pMdlComponent).GetLodMeshGroup(lod);
}
const rendering::RenderMeshGroup &CRenderComponent::GetLodMeshGroup(uint32_t lod) const { return const_cast<CRenderComponent *>(this)->GetLodMeshGroup(lod); }
const std::vector<std::shared_ptr<geometry::ModelSubMesh>> &CRenderComponent::GetRenderMeshes() const { return const_cast<CRenderComponent *>(this)->GetRenderMeshes(); }
std::vector<std::shared_ptr<geometry::ModelSubMesh>> &CRenderComponent::GetRenderMeshes()
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static std::vector<std::shared_ptr<geometry::ModelSubMesh>> meshes {};
		return meshes;
	}
	return static_cast<CModelComponent &>(*pMdlComponent).GetRenderMeshes();
}
std::vector<rendering::RenderBufferData> &CRenderComponent::GetRenderBufferData()
{
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static std::vector<rendering::RenderBufferData> renderBufferData {};
		return renderBufferData;
	}
	return static_cast<CModelComponent &>(*pMdlComponent).GetRenderBufferData();
}
const std::vector<std::shared_ptr<geometry::ModelMesh>> &CRenderComponent::GetLODMeshes() const { return const_cast<CRenderComponent *>(this)->GetLODMeshes(); }
std::vector<std::shared_ptr<geometry::ModelMesh>> &CRenderComponent::GetLODMeshes()
{
	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	auto pSoftBodyComponent = ent.GetComponent<CSoftBodyComponent>();
	if(pSoftBodyComponent.valid()) {
		static std::vector<std::shared_ptr<geometry::ModelMesh>> meshes {};
		return meshes;
		// TODO
		//auto *pSoftBodyData = pSoftBodyComponent->GetSoftBodyData();
		//if(pSoftBodyData != nullptr)
		//	return pSoftBodyData->meshes;
	}
	auto *pMdlComponent = GetModelComponent();
	if(!pMdlComponent) {
		static std::vector<std::shared_ptr<geometry::ModelMesh>> meshes {};
		return meshes;
	}
	return static_cast<CModelComponent &>(*pMdlComponent).GetLODMeshes();
}
const std::vector<CRenderComponent *> &CRenderComponent::GetEntitiesExemptFromOcclusionCulling() { return s_ocExemptEntities; }
const std::shared_ptr<prosper::IUniformResizableBuffer> &CRenderComponent::GetInstanceBuffer() { return s_instanceBuffer; }
void CRenderComponent::ClearBuffers()
{
	s_instanceBuffer = nullptr;
	clear_articulated_buffers();
	clear_vertex_animation_buffer();
	CRaytracingComponent::ClearBuffers();
}
void CRenderComponent::SetTranslucencyPassDistanceOverride(double distance) { m_translucencyPassDistanceOverrideSqr = math::pow2(distance); }
void CRenderComponent::ClearTranslucencyPassDistanceOverride() { m_translucencyPassDistanceOverrideSqr = {}; }
const std::optional<double> &CRenderComponent::GetTranslucencyPassDistanceOverrideSqr() const { return m_translucencyPassDistanceOverrideSqr; }

/////////////////

CEUpdateInstantiability::CEUpdateInstantiability(bool &instantiable) : instantiable {instantiable} {}
void CEUpdateInstantiability::PushArguments(lua::State *l) { Lua::PushBool(l, instantiable); }
uint32_t CEUpdateInstantiability::GetReturnCount() { return 1u; }
void CEUpdateInstantiability::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -1))
		instantiable = Lua::CheckBool(l, -1);
}

/////////////////

CEShouldDraw::CEShouldDraw(bool &shouldDraw) : shouldDraw {shouldDraw} {}
void CEShouldDraw::PushArguments(lua::State *l) { Lua::PushBool(l, shouldDraw); }
uint32_t CEShouldDraw::GetReturnCount() { return 1u; }
void CEShouldDraw::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -1))
		shouldDraw = Lua::CheckBool(l, -1);
}

/////////////////

CEOnUpdateRenderMatrices::CEOnUpdateRenderMatrices(math::ScaledTransform &pose, Mat4 &transformation) : pose {pose}, transformation {transformation} {}
void CEOnUpdateRenderMatrices::PushArguments(lua::State *l)
{
	Lua::Push<math::ScaledTransform>(l, pose);
	Lua::Push<Mat4>(l, transformation);
}
uint32_t CEOnUpdateRenderMatrices::GetReturnCount() { return 3; }
void CEOnUpdateRenderMatrices::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -2))
		pose = Lua::Check<math::ScaledTransform>(l, -2);
	if(Lua::IsSet(l, -1))
		transformation = Lua::Check<Mat4>(l, -1);
}

/////////////////

CEOnUpdateRenderData::CEOnUpdateRenderData() {}
void CEOnUpdateRenderData::PushArguments(lua::State *l) { throw std::runtime_error {"Lua callbacks of multi-threaded events are not allowed!"}; }

/////////////////

CEOnUpdateRenderBuffers::CEOnUpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &commandBuffer) : commandBuffer {commandBuffer} {}
void CEOnUpdateRenderBuffers::PushArguments(lua::State *l) {}

/////////////////

CEOnRenderBoundsChanged::CEOnRenderBoundsChanged(const Vector3 &min, const Vector3 &max, const math::Sphere &sphere) : min {min}, max {max}, sphere {sphere} {}
void CEOnRenderBoundsChanged::PushArguments(lua::State *l)
{
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
	Lua::Push<Vector3>(l, sphere.pos);
	Lua::PushNumber(l, sphere.radius);
}

/////////////////

void rendering::RenderBufferData::SetDepthPrepassEnabled(bool enabled) { math::set_flag(stateFlags, StateFlags::EnableDepthPrepass, enabled); }
bool rendering::RenderBufferData::IsDepthPrepassEnabled() const { return math::is_flag_set(stateFlags, StateFlags::EnableDepthPrepass); }

void rendering::RenderBufferData::SetGlowPassEnabled(bool enabled) { math::set_flag(stateFlags, StateFlags::EnableGlowPass, enabled); }
bool rendering::RenderBufferData::IsGlowPassEnabled() const { return math::is_flag_set(stateFlags, StateFlags::EnableGlowPass); }

static void debug_entity_render_buffer(NetworkState *state, BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	auto ents = pragma::console::find_target_entity(state, *charComponent, argv);
	if(ents.empty()) {
		Con::CWAR << "No target entity found!" << Con::endl;
		return;
	}
	auto *ent = ents.front();
	auto mdlC = ent->GetComponent<CModelComponent>();
	if(mdlC.expired()) {
		Con::CWAR << "Target entity has no model component!" << Con::endl;
	}
	else {
		auto vdata = mdlC->GetRenderBufferData();
		if(vdata.empty()) {
			Con::CWAR << "No render buffer data found!" << Con::endl;
			return;
		}
		size_t lod = 0;
		for(auto &data : vdata) {
			Con::COUT << "Render buffer data for LOD " << lod << ": " << Con::endl;
			Con::COUT << "pipelineSpecializationFlags: " << magic_enum::enum_name(data.pipelineSpecializationFlags) << Con::endl;
			Con::COUT << "material: ";
			if(data.material)
				Con::COUT << data.material->GetName();
			else
				Con::COUT << "NULL";
			Con::COUT << Con::endl;
			Con::COUT << "stateFlags: " << magic_enum::enum_name(data.stateFlags) << Con::endl;
		}
		Con::COUT << Con::endl;
	}

	auto renderC = ent->GetComponent<CRenderComponent>();
	if(renderC.expired()) {
		Con::CWAR << "Target entity has no render component!" << Con::endl;
	}
	else {
		auto printInstanceData = [](const rendering::InstanceData &instanceData) {
			Con::COUT << "modelMatrix: " << umat::to_string(instanceData.modelMatrix) << Con::endl;
			Con::COUT << "color: " << instanceData.color << Con::endl;
			Con::COUT << "renderFlags: " << magic_enum::enum_name(instanceData.renderFlags) << Con::endl;
			Con::COUT << "entityIndex: " << instanceData.entityIndex << Con::endl;
			Con::COUT << "padding: " << instanceData.padding << Con::endl;
		};
		auto &instanceData = renderC->GetInstanceData();
		Con::COUT << "Instance data:" << Con::endl;
		printInstanceData(instanceData);

		auto *buf = renderC->GetRenderBuffer();
		rendering::InstanceData bufData;
		if(!buf || !buf->Read(0, sizeof(bufData), &bufData))
			Con::CWAR << "Failed to read buffer data!" << Con::endl;
		else {
			if(memcmp(&instanceData, &bufData, sizeof(bufData)) != 0) {
				Con::CWAR << "Instance data does not match data in buffer! Data in buffer:" << Con::endl;
				printInstanceData(bufData);
			}
		}
		Con::COUT << Con::endl;
	}
}
namespace {
	auto UVN = console::client::register_command("debug_entity_render_buffer", &debug_entity_render_buffer, console::ConVarFlags::None, "Prints debug information about an entity's render buffer.");
}
namespace Lua::Render {
	void CalcRayIntersection(lua::State *l, CRenderComponent &hComponent, const Vector3 &start, const Vector3 &dir, bool precise)
	{

		auto result = hComponent.CalcRayIntersection(start, dir, precise);
		if(result.has_value() == false) {
			PushInt(l, pragma::math::to_integral(pragma::math::intersection::Result::NoIntersection));
			return;
		}
		Push(l, pragma::math::to_integral(result->result));

		auto t = CreateTable(l);

		PushString(l, "position");             /* 1 */
		Lua::Push<Vector3>(l, result->hitPos); /* 2 */
		SetTableValue(l, t);                   /* 0 */

		PushString(l, "distance");       /* 1 */
		PushNumber(l, result->hitValue); /* 2 */
		SetTableValue(l, t);             /* 0 */

		if(precise && result->precise) {
			PushString(l, "uv");                                                         /* 1 */
			Lua::Push<::Vector2>(l, ::Vector2 {result->precise->u, result->precise->v}); /* 2 */
			SetTableValue(l, t);                                                         /* 0 */
			return;
		}

		PushString(l, "boneId");    /* 1 */
		PushInt(l, result->boneId); /* 2 */
		SetTableValue(l, t);        /* 0 */
	}
	void CalcRayIntersection(lua::State *l, CRenderComponent &hComponent, const Vector3 &start, const Vector3 &dir) { CalcRayIntersection(l, hComponent, start, dir, false); }
	void GetTransformationMatrix(lua::State *l, CRenderComponent &hEnt)
	{

		::Mat4 mat = hEnt.GetTransformationMatrix();
		luabind::object(l, mat).push(l);
	}

	void GetLocalRenderBounds(lua::State *l, CRenderComponent &hEnt)
	{

		auto &aabb = hEnt.GetLocalRenderBounds();
		Lua::Push<Vector3>(l, aabb.min);
		Lua::Push<Vector3>(l, aabb.max);
	}
	void GetLocalRenderSphereBounds(lua::State *l, CRenderComponent &hEnt)
	{

		auto &sphere = hEnt.GetLocalRenderSphere();
		Lua::Push<Vector3>(l, sphere.pos);
		PushNumber(l, sphere.radius);
	}
	void GetAbsoluteRenderBounds(lua::State *l, CRenderComponent &hEnt)
	{

		auto &aabb = hEnt.GetUpdatedAbsoluteRenderBounds();
		Lua::Push<Vector3>(l, aabb.min);
		Lua::Push<Vector3>(l, aabb.max);
	}
	void GetAbsoluteRenderSphereBounds(lua::State *l, CRenderComponent &hEnt)
	{

		auto &sphere = hEnt.GetUpdatedAbsoluteRenderSphere();
		Lua::Push<Vector3>(l, sphere.pos);
		PushNumber(l, sphere.radius);
	}

	void SetLocalRenderBounds(lua::State *l, CRenderComponent &hEnt, Vector3 &min, Vector3 &max) { hEnt.SetLocalRenderBounds(min, max); }

	/*void UpdateRenderBuffers(lua::State *l,pragma::CRenderComponent &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam,bool bForceBufferUpdate)
    {



        if(drawCmd->IsPrimary() == false)
            return;
        auto vp = hCam->GetProjectionMatrix() *hCam->GetViewMatrix();
        hEnt.UpdateRenderData(std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(drawCmd),*hScene,*hCam,vp,bForceBufferUpdate);
    }
    void UpdateRenderBuffers(lua::State *l,pragma::CRenderComponent &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam) {UpdateRenderBuffers(l,hEnt,drawCmd,hScene,hCam,false);}*/
	void GetRenderBuffer(lua::State *l, CRenderComponent &hEnt)
	{
		auto *buf = hEnt.GetRenderBuffer();
		if(buf == nullptr)
			return;
		Push(l, buf->shared_from_this());
	}
	void GetBoneBuffer(lua::State *l, CRenderComponent &hEnt)
	{

		auto *pAnimComponent = static_cast<CAnimatedComponent *>(hEnt.GetEntity().GetAnimatedComponent().get());
		if(pAnimComponent == nullptr)
			return;
		auto *buf = pAnimComponent->GetBoneBuffer();
		if(!buf)
			return;
		Push(l, buf->shared_from_this());
	}
};

void CRenderComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseRenderComponent::RegisterLuaBindings(l, modEnts);
	auto defCRender = pragma::LuaCore::create_entity_component_class<CRenderComponent, BaseRenderComponent>("RenderComponent");
	defCRender.def("GetTransformationMatrix", &Lua::Render::GetTransformationMatrix);
	defCRender.def("IsInPvs", static_cast<bool (CRenderComponent::*)(const Vector3 &, const CWorldComponent &) const>(&CRenderComponent::IsInPvs));
	defCRender.def("IsInPvs", static_cast<bool (CRenderComponent::*)(const Vector3 &) const>(&CRenderComponent::IsInPvs));
	defCRender.def("IsInRenderGroup", &CRenderComponent::IsInRenderGroup);
	defCRender.def("GetSceneRenderPass", &CRenderComponent::GetSceneRenderPass);
	defCRender.def("SetSceneRenderPass", &CRenderComponent::SetSceneRenderPass);
	defCRender.def("GetSceneRenderPassProperty", +[](lua::State *l, CRenderComponent &c) { Lua::Property::push(l, *c.GetSceneRenderPassProperty()); });
	defCRender.def("AddToRenderGroup", static_cast<bool (CRenderComponent::*)(const std::string &)>(&CRenderComponent::AddToRenderGroup));
	defCRender.def("AddToRenderGroup", static_cast<void (CRenderComponent::*)(rendering::RenderGroup)>(&CRenderComponent::AddToRenderGroup));
	defCRender.def("RemoveFromRenderGroup", static_cast<bool (CRenderComponent::*)(const std::string &)>(&CRenderComponent::RemoveFromRenderGroup));
	defCRender.def("RemoveFromRenderGroup", static_cast<void (CRenderComponent::*)(rendering::RenderGroup)>(&CRenderComponent::RemoveFromRenderGroup));
	defCRender.def("SetRenderGroups", &CRenderComponent::SetRenderGroups);
	defCRender.def("GetRenderGroups", &CRenderComponent::GetRenderGroups);
	defCRender.def("GetRenderGroupsProperty", +[](lua::State *l, CRenderComponent &c) { Lua::Property::push(l, *c.GetRenderGroupsProperty()); });
	defCRender.def("GetLocalRenderBounds", &Lua::Render::GetLocalRenderBounds);
	defCRender.def("GetLocalRenderSphereBounds", &Lua::Render::GetLocalRenderSphereBounds);
	defCRender.def("GetAbsoluteRenderBounds", &Lua::Render::GetAbsoluteRenderBounds);
	defCRender.def("GetAbsoluteRenderSphereBounds", &Lua::Render::GetAbsoluteRenderSphereBounds);
	defCRender.def("SetLocalRenderBounds", &Lua::Render::SetLocalRenderBounds);
	// defCRender.def("UpdateRenderBuffers",static_cast<void(*)(lua::State*,pragma::CRenderComponent&,std::shared_ptr<prosper::ICommandBuffer>&,CSceneHandle&,CCameraHandle&,bool)>(&Lua::Render::UpdateRenderBuffers));
	// defCRender.def("UpdateRenderBuffers",static_cast<void(*)(lua::State*,pragma::CRenderComponent&,std::shared_ptr<prosper::ICommandBuffer>&,CSceneHandle&,CCameraHandle&)>(&Lua::Render::UpdateRenderBuffers));
	defCRender.def("GetRenderBuffer", &Lua::Render::GetRenderBuffer);
	defCRender.def("GetBoneBuffer", &Lua::Render::GetBoneBuffer);
	defCRender.def("GetLODMeshes", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) {
		auto &lodMeshes = hComponent.GetLODMeshes();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &lodMesh : lodMeshes) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, lodMesh);
			Lua::SetTableValue(l, t);
		}
	}));
	defCRender.def("GetRenderMeshes", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) {
		auto &renderMeshes = hComponent.GetRenderMeshes();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &mesh : renderMeshes) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, mesh);
			Lua::SetTableValue(l, t);
		}
	}));
	defCRender.def("GetLodRenderMeshes", static_cast<void (*)(lua::State *, CRenderComponent &, uint32_t)>([](lua::State *l, CRenderComponent &hComponent, uint32_t lod) {
		auto &renderMeshes = hComponent.GetRenderMeshes();
		auto &renderMeshGroup = hComponent.GetLodRenderMeshGroup(lod);
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto i = renderMeshGroup.first; i < renderMeshGroup.second; ++i) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, renderMeshes[i]);
			Lua::SetTableValue(l, t);
		}
	}));
	defCRender.def("SetExemptFromOcclusionCulling", static_cast<void (*)(lua::State *, CRenderComponent &, bool)>([](lua::State *l, CRenderComponent &hComponent, bool exempt) { hComponent.SetExemptFromOcclusionCulling(exempt); }));
	defCRender.def("IsExemptFromOcclusionCulling", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) { Lua::PushBool(l, hComponent.IsExemptFromOcclusionCulling()); }));
	defCRender.def("SetReceiveShadows", static_cast<void (*)(lua::State *, CRenderComponent &, bool)>([](lua::State *l, CRenderComponent &hComponent, bool enabled) { hComponent.SetReceiveShadows(enabled); }));
	defCRender.def("IsReceivingShadows", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) { Lua::PushBool(l, hComponent.IsReceivingShadows()); }));
	defCRender.def("SetRenderBufferDirty", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) { hComponent.SetRenderBufferDirty(); }));
	/*defCRender.def("GetDepthBias",static_cast<void(*)(lua::State*,pragma::CRenderComponent&)>([](lua::State *l,pragma::CRenderComponent &hComponent) {

		float constantFactor,biasClamp,slopeFactor;
		hComponent.GetDepthBias(constantFactor,biasClamp,slopeFactor);
		Lua::PushNumber(l,constantFactor);
		Lua::PushNumber(l,biasClamp);
		Lua::PushNumber(l,slopeFactor);
	}));
	defCRender.def("SetDepthBias",static_cast<void(*)(lua::State*,pragma::CRenderComponent&,float,float,float)>([](lua::State *l,pragma::CRenderComponent &hComponent,float constantFactor,float biasClamp,float slopeFactor) {

		hComponent.SetDepthBias(constantFactor,biasClamp,slopeFactor);
	}));*/
	defCRender.def("CalcRayIntersection", static_cast<void (*)(lua::State *, CRenderComponent &, const Vector3 &, const Vector3 &, bool)>(&Lua::Render::CalcRayIntersection));
	defCRender.def("CalcRayIntersection", static_cast<void (*)(lua::State *, CRenderComponent &, const Vector3 &, const Vector3 &)>(&Lua::Render::CalcRayIntersection));
	defCRender.def("SetDepthPassEnabled", static_cast<void (*)(lua::State *, CRenderComponent &, bool)>([](lua::State *l, CRenderComponent &hComponent, bool depthPassEnabled) { hComponent.SetDepthPassEnabled(depthPassEnabled); }));
	defCRender.def("IsDepthPassEnabled", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) { Lua::PushBool(l, hComponent.IsDepthPassEnabled()); }));
	defCRender.def("GetRenderClipPlane", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) {
		auto *clipPlane = hComponent.GetRenderClipPlane();
		if(clipPlane == nullptr)
			return;
		Lua::Push(l, *clipPlane);
	}));
	defCRender.def("SetRenderClipPlane", static_cast<void (*)(lua::State *, CRenderComponent &, const Vector4 &)>([](lua::State *l, CRenderComponent &hComponent, const Vector4 &clipPlane) { hComponent.SetRenderClipPlane(clipPlane); }));
	defCRender.def("ClearRenderClipPlane", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) { hComponent.ClearRenderClipPlane(); }));
	defCRender.def("GetDepthBias", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) {
		auto *depthBias = hComponent.GetDepthBias();
		if(depthBias == nullptr)
			return;
		Lua::PushNumber(l, depthBias->x);
		Lua::PushNumber(l, depthBias->y);
	}));
	defCRender.def("SetDepthBias", static_cast<void (*)(lua::State *, CRenderComponent &, float, float)>([](lua::State *l, CRenderComponent &hComponent, float d, float delta) { hComponent.SetDepthBias(d, delta); }));
	defCRender.def("ClearDepthBias", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) { hComponent.ClearDepthBias(); }));
	defCRender.def("GetRenderPose", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) { Lua::Push(l, hComponent.GetRenderPose()); }));
	defCRender.def("SetRenderOffsetTransform", static_cast<void (*)(lua::State *, CRenderComponent &, const math::ScaledTransform &)>([](lua::State *l, CRenderComponent &hComponent, const math::ScaledTransform &pose) { hComponent.SetRenderOffsetTransform(pose); }));
	defCRender.def("ClearRenderOffsetTransform", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) { hComponent.ClearRenderOffsetTransform(); }));
	defCRender.def("GetRenderOffsetTransform", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) {
		auto *t = hComponent.GetRenderOffsetTransform();
		if(t == nullptr)
			return;
		Lua::Push(l, *t);
	}));
	defCRender.def("ShouldCastShadows", static_cast<bool (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) -> bool { return hComponent.GetCastShadows(); }));
	defCRender.def("ShouldDraw", static_cast<bool (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) -> bool { return hComponent.ShouldDraw(); }));
	defCRender.def("ShouldDrawShadow", static_cast<bool (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) -> bool { return hComponent.ShouldDrawShadow(); }));
	defCRender.def("ClearBuffers", static_cast<void (*)(lua::State *, CRenderComponent &)>([](lua::State *l, CRenderComponent &hComponent) { hComponent.ClearRenderBuffers(); }));
	defCRender.def("SetTranslucencyPassDistanceOverride", &CRenderComponent::SetTranslucencyPassDistanceOverride);
	defCRender.def("ClearTranslucencyPassDistanceOverride", &CRenderComponent::ClearTranslucencyPassDistanceOverride);
	defCRender.def("GetTranslucencyPassDistanceOverrideSqr", &CRenderComponent::GetTranslucencyPassDistanceOverrideSqr);

	defCRender.def("SetHidden", &CRenderComponent::SetHidden);
	defCRender.def("IsHidden", &CRenderComponent::IsHidden);
	defCRender.def("SetVisible", +[](CRenderComponent &renderC, bool visible) { renderC.SetHidden(!visible); });
	defCRender.def("IsVisible", &CRenderComponent::IsVisible);
	defCRender.def("SetIgnoreAncestorVisibility", &CRenderComponent::SetIgnoreAncestorVisibility);
	defCRender.def("ShouldIgnoreAncestorVisibility", &CRenderComponent::ShouldIgnoreAncestorVisibility);

	// defCRender.add_static_constant("EVENT_ON_UPDATE_RENDER_DATA",pragma::CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA);
	defCRender.add_static_constant("EVENT_ON_RENDER_BOUNDS_CHANGED", cRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED);
	defCRender.add_static_constant("EVENT_ON_RENDER_MODE_CHANGED", cRenderComponent::EVENT_ON_RENDER_MODE_CHANGED);
	defCRender.add_static_constant("EVENT_ON_RENDER_BUFFERS_INITIALIZED", cRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED);
	defCRender.add_static_constant("EVENT_SHOULD_DRAW", cRenderComponent::EVENT_SHOULD_DRAW);
	defCRender.add_static_constant("EVENT_SHOULD_DRAW_SHADOW", cRenderComponent::EVENT_SHOULD_DRAW_SHADOW);
	defCRender.add_static_constant("EVENT_ON_UPDATE_RENDER_MATRICES", cRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES);
	defCRender.add_static_constant("EVENT_UPDATE_INSTANTIABILITY", cRenderComponent::EVENT_UPDATE_INSTANTIABILITY);
	defCRender.add_static_constant("EVENT_ON_CLIP_PLANE_CHANGED", cRenderComponent::EVENT_ON_CLIP_PLANE_CHANGED);
	defCRender.add_static_constant("EVENT_ON_DEPTH_BIAS_CHANGED", cRenderComponent::EVENT_ON_DEPTH_BIAS_CHANGED);
	modEnts[defCRender];
}
