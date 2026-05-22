// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.animated;
import :engine;
import :entities.components.particle_system;
import :entities.components.render;
import :entities.components.vertex_animated;
import :scripting.lua.libraries.vulkan;

using namespace pragma;

ComponentEventId cAnimatedComponent::EVENT_ON_SKELETON_UPDATED = INVALID_COMPONENT_ID;
ComponentEventId cAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED = INVALID_COMPONENT_ID;
void CAnimatedComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseAnimatedComponent::RegisterEvents(componentManager, registerEvent);
	cAnimatedComponent::EVENT_ON_SKELETON_UPDATED = registerEvent("ON_SKELETON_UPDATED", ComponentEventInfo::Type::Explicit);
	cAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED = registerEvent("ON_BONE_MATRICES_UPDATED", ComponentEventInfo::Type::Explicit);
}
void CAnimatedComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseAnimatedComponent)); }
void CAnimatedComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
static std::shared_ptr<prosper::LinearBuffer> s_baseBoneBuffer = nullptr;
//static std::shared_ptr<prosper::FrameScopedRingBuffer> s_boneRingBuffer = nullptr;
const std::shared_ptr<prosper::LinearBuffer> &pragma::get_instance_bone_buffer() { return s_baseBoneBuffer; }
//const std::shared_ptr<prosper::FrameScopedRingBuffer> &pragma::get_bone_ring_buffer() { return s_boneRingBuffer; }
constexpr size_t SIZEOF_BONE_BUFFER_BONE = sizeof(Mat4);
// Start with 1024 bones total per frame. The buffer will re-allocate automatically if more are required.
constexpr size_t SIZEOF_INITIAL_BONE_RING_BUFFER_PER_FRAME_IN_FLIGHT = SIZEOF_BONE_BUFFER_BONE * 1'024;
static prosper::BufferUsageFlags get_bone_buffer_usage_flags() { return prosper::BufferUsageFlags::StorageBufferBit; }
void pragma::initialize_articulated_buffers()
{
	// Memory usage (approximately): MAX_BASE_NUMBER_OF_SKINNED_INSTANCES *MAX_BASE_NUMBER_OF_BONES_PER_INSTANCE *SIZEOF_BONE_BUFFER_BONE
	// 200 *64 *64 = 800 KiB
	// Depending on the device's alignment requirements, the actual memory usage may be larger.

	// TODO
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = get_bone_buffer_usage_flags();

	auto &context = get_cengine()->GetRenderContext();
	if constexpr(CRenderComponent::USE_HOST_MEMORY_FOR_RENDER_DATA) {
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
		createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	}
	else
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	createInfo.size = SIZEOF_INITIAL_BONE_RING_BUFFER_PER_FRAME_IN_FLIGHT * context.GetMaxNumberOfFramesInFlight();
	createInfo.debugName = "entity_anim_bone_buf";
	auto baseBuffer = context.CreateBuffer(createInfo);
	auto alignment = context.CalcBufferAlignment(baseBuffer->GetUsageFlags());
	s_baseBoneBuffer = prosper::LinearBuffer::Create(*baseBuffer, alignment);

	if constexpr(CRenderComponent::USE_HOST_MEMORY_FOR_RENDER_DATA)
		s_baseBoneBuffer->GetBaseBuffer().SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit | prosper::IBuffer::MapFlags::Unsynchronized);

	//s_boneRingBuffer = prosper::FrameScopedRingBuffer::Create(*s_baseBoneBuffer);
}
void pragma::clear_articulated_buffers()
{
	s_baseBoneBuffer = nullptr;
	//s_boneRingBuffer = nullptr;
}

void CAnimatedComponent::SetBoneBufferDirty() { math::set_flag(m_stateFlags, StateFlags::BoneBufferDirty); }
void CAnimatedComponent::SetSkeletonUpdateCallbacksEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::EnableSkeletonUpdateCallbacks, enabled); }
bool CAnimatedComponent::AreSkeletonUpdateCallbacksEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::EnableSkeletonUpdateCallbacks); }

void CAnimatedComponent::Initialize()
{
	BaseAnimatedComponent::Initialize();

	/*BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto pRenderComponent = GetEntity().GetComponent<CRenderComponent>();
		if(pRenderComponent.valid())
			pRenderComponent->UpdateRenderBounds();
	});*/
	BindEventUnhandled(cRenderComponent::EVENT_ON_UPDATE_RENDER_DATA_MT, [this](std::reference_wrapper<ComponentEvent> evData) {
		if(AreSkeletonUpdateCallbacksEnabled())
			return; // Bone matrices will be updated from main thread
		UpdateBoneMatricesMT();
	});
	BindEventUnhandled(cRenderComponent::EVENT_ON_UPDATE_RENDER_BUFFERS_MT, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto isDirty = math::is_flag_set(m_stateFlags, StateFlags::BoneBufferDirty);
		math::set_flag(m_stateFlags, StateFlags::BoneBufferDirty, false);
		UpdateBoneBuffer(isDirty);
	});
	BindEvent(cRenderComponent::EVENT_UPDATE_INSTANTIABILITY, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		// TODO: Allow instantiability for animated entities
		static_cast<CEUpdateInstantiability &>(evData.get()).instantiable = false;
		return util::EventReply::Handled;
	});
	auto &ent = GetEntity();
	auto pRenderComponent = ent.GetComponent<CRenderComponent>();
	if(pRenderComponent.valid()) {
		pRenderComponent->SetRenderBufferDirty();
		pRenderComponent->UpdateInstantiability();
		pRenderComponent->UpdateRenderDescriptorSetDynamicOffsets();
	}
}

void CAnimatedComponent::OnRemove()
{
	BaseAnimatedComponent::OnRemove();
	auto pRenderComponent = GetEntity().GetComponent<CRenderComponent>();
	if(pRenderComponent.valid()) {
		pRenderComponent->SetRenderBufferDirty();
		pRenderComponent->UpdateRenderDescriptorSetDynamicOffsets();
	}
}

void CAnimatedComponent::PlayAnimation(int animation, FPlayAnim flags)
{
	auto curAnim = m_baseAnim.animation;
	BaseAnimatedComponent::PlayAnimation(animation, flags);
	if((curAnim == -1 && m_baseAnim.animation != -1) || (curAnim != -1 && m_baseAnim.animation == -1)) {
		// Render buffer needs to be updated if our animation state changed
		auto pRenderComponent = GetEntity().GetComponent<CRenderComponent>();
		if(pRenderComponent.valid())
			pRenderComponent->SetRenderBufferDirty();
	}
}

void CAnimatedComponent::ReceiveData(NetPacket &packet)
{
	int anim = packet->Read<int>();
	float cycle = packet->Read<float>();
	PlayAnimation(anim);
	SetCycle(cycle);
}

bool CAnimatedComponent::GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, math::ScaledTransform &outPose) const { return BaseAnimatedComponent::GetVertexTransformMatrix(subMesh, vertexId, outPose); }
std::optional<Mat4> CAnimatedComponent::GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId) const { return GetVertexTransformMatrix(subMesh, vertexId, nullptr, nullptr); }
std::optional<Mat4> CAnimatedComponent::GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, Vector3 *optOutNormalOffset, float *optOutDelta) const
{
	if(optOutNormalOffset)
		*optOutNormalOffset = {};
	if(optOutDelta)
		*optOutDelta = 0.f;
	auto t = BaseAnimatedComponent::GetVertexTransformMatrix(subMesh, vertexId);
	if(t.has_value() == false)
		return {};
	Vector3 vertexOffset;
	auto pVertexAnimatedComponent = GetEntity().GetComponent<CVertexAnimatedComponent>();
	if(pVertexAnimatedComponent.expired() || pVertexAnimatedComponent->GetLocalVertexPosition(subMesh, vertexId, vertexOffset, optOutNormalOffset, optOutDelta) == false)
		return t;
	return *t * glm::gtc::translate(umat::identity(), vertexOffset); // TODO: Confirm order!
}

void CAnimatedComponent::OnModelChanged(const std::shared_ptr<asset::Model> &mdl) { BaseAnimatedComponent::OnModelChanged(mdl); }

bool CAnimatedComponent::HasBones() const { return !m_boneMatrices.empty(); }

size_t CAnimatedComponent::GetBoneBufferSize() const { return m_boneMatrices.size() * SIZEOF_BONE_BUFFER_BONE; }

void CAnimatedComponent::ResetAnimation(const std::shared_ptr<asset::Model> &mdl)
{
	BaseAnimatedComponent::ResetAnimation(mdl);

	auto numBonesPrev = m_boneMatrices.size();
	util::ScopeGuard sg {[&]() {
		if(m_boneMatrices.size() == numBonesPrev)
			return;
		// Number of bone matrices has changed, we need to inform the render component that the
		// descriptor set needs updating
		auto &ent = GetEntity();
		auto pRenderComponent = ent.GetComponent<CRenderComponent>();
		if(pRenderComponent.valid())
			pRenderComponent->UpdateAnimationBufferDescriptorBinding();
	}};

	m_boneMatrices.clear();
	if(mdl == nullptr || GetBoneCount() == 0)
		return;
	m_boneMatrices.resize(mdl->GetBoneCount(), umat::identity());
	UpdateBoneMatricesMT();
	SetBoneBufferDirty();

	// Attach particles defined in the model
	auto &ent = GetEntity();
	for(auto &objAttachment : mdl->GetObjectAttachments()) {
		switch(objAttachment.type) {
		case asset::ObjectAttachment::Type::Model:
			spdlog::warn("Unsupported object attachment type '{}' for model '{}'!", magic_enum::enum_name(objAttachment.type), mdl->GetName());
			break;
		case asset::ObjectAttachment::Type::ParticleSystem:
			auto itParticleFile = objAttachment.keyValues.find("particle_file");
			if(itParticleFile != objAttachment.keyValues.end())
				ecs::CParticleSystemComponent::Precache(itParticleFile->second);
			auto itParticle = objAttachment.keyValues.find("particle");
			if(itParticle != objAttachment.keyValues.end()) {
				Vector3 translation {};
				auto rotation = uquat::identity();

				auto itTranslation = objAttachment.keyValues.find("translation");
				if(itTranslation != objAttachment.keyValues.end())
					translation = uvec::create(itTranslation->second);

				auto itRotation = objAttachment.keyValues.find("rotation");
				if(itRotation != objAttachment.keyValues.end())
					rotation = uquat::create(itRotation->second);

				auto itAngles = objAttachment.keyValues.find("angles");
				if(itAngles != objAttachment.keyValues.end())
					rotation = uquat::create(EulerAngles(itAngles->second));

				auto *pt = ecs::CParticleSystemComponent::Create(itParticle->second);
				if(pt != nullptr) {
					auto &entPt = pt->GetEntity();
					auto pAttachableComponent = entPt.AddComponent<CAttachmentComponent>();
					if(pAttachableComponent.valid()) {
						auto pTrComponentPt = entPt.GetTransformComponent();
						if(pTrComponentPt) {
							pTrComponentPt->SetPosition(translation);
							pTrComponentPt->SetRotation(rotation);
						}
						pAttachableComponent->AttachToAttachment(&ent, objAttachment.attachment);
					}
					pt->Start();
				}
			}
			break;
		}
	}
}

void CAnimatedComponent::SetCurrentFrameBoneBufferOffset(std::optional<prosper::LinearBuffer::BufferOffset> offset)
{
	if(offset == m_boneBufferOffset)
		return;
	m_boneBufferOffset = offset;
	auto renderC = GetEntity().GetComponent<CRenderComponent>();
	if(renderC.expired())
		return;
	renderC->UpdateRenderDescriptorSetDynamicOffsets();
}

void CAnimatedComponent::UpdateBoneBuffer(bool flagAsDirty)
{
	auto size = GetBoneBufferSize();
	if(size == 0) {
		SetCurrentFrameBoneBufferOffset({});
		return;
	}

	SetCurrentFrameBoneBufferOffset(s_baseBoneBuffer->Allocate(size, m_boneMatrices.data()));
}
const std::vector<Mat4> &CAnimatedComponent::GetBoneMatrices() const { return const_cast<CAnimatedComponent *>(this)->GetBoneMatrices(); }
std::vector<Mat4> &CAnimatedComponent::GetBoneMatrices() { return m_boneMatrices; }

bool CAnimatedComponent::MaintainAnimations(double dt)
{
#pragma message("TODO: Undo this and do it properly!")
	//if(BaseEntity::MaintainAnimations() == false)
	//	return false;
	if(ShouldUpdateBones() == false)
		return false;
	BaseAnimatedComponent::MaintainAnimations(dt);
	SetBoneBufferDirty(); // TODO: Only if anything has actually changed
	return true;
}

bool CAnimatedComponent::UpdateBonePoses()
{
	auto updated = BaseAnimatedComponent::UpdateBonePoses();
	if(updated)
		SetBoneBufferDirty();
	return updated;
}

void CAnimatedComponent::UpdateBoneMatricesMT()
{
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto *bindPose = GetBindPose();
	if(m_boneMatrices.empty() || bindPose == nullptr)
		return;
	UpdateSkeleton();
	auto physRootBoneId = OnSkeletonUpdated();

	auto callbacksEnabled = AreSkeletonUpdateCallbacksEnabled();
	if(callbacksEnabled) {
		CEOnSkeletonUpdated evData {physRootBoneId};
		InvokeEventCallbacks(cAnimatedComponent::EVENT_ON_SKELETON_UPDATED, evData);
	}

	auto &refFrame = *bindPose;
	auto numBones = GetBoneCount();
	if(numBones != m_processedBones.size() || numBones != m_boneMatrices.size()) {
		Con::CWAR << "Bone count mismatch between processed bones and actual bones for entity " << GetEntity() << Con::endl;
		return;
	}
	for(unsigned int i = 0; i < GetBoneCount(); i++) {
		auto &t = m_processedBones.at(i);
		auto &pos = t.GetOrigin();
		auto &orientation = t.GetRotation();
		auto &scale = t.GetScale();

		auto *posBind = refFrame.GetBonePosition(i);
		auto *rotBind = refFrame.GetBoneOrientation(i);
		if(posBind != nullptr && rotBind != nullptr) {
			auto &mat = m_boneMatrices[i];
			if(i != physRootBoneId) {
				math::Transform tBindPose {*posBind, *rotBind};
				tBindPose = tBindPose.GetInverse();
				math::ScaledTransform tBone {pos, orientation, scale};

				mat = tBone.ToMatrix() * tBindPose.ToMatrix();
				//mat = (tBone *tBindPose).ToMatrix();
			}
			else
				mat = umat::identity();
		}
		else
			Con::CWAR << "Attempted to update bone " << i << " in " << mdl->GetName() << " which doesn't exist in the reference pose! Ignoring..." << Con::endl;
	}
	if(callbacksEnabled)
		InvokeEventCallbacks(cAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED);
}

uint32_t CAnimatedComponent::OnSkeletonUpdated() { return std::numeric_limits<uint32_t>::max(); }

//////////////

CEOnSkeletonUpdated::CEOnSkeletonUpdated(uint32_t &physRootBoneId) : physRootBoneId {physRootBoneId} {}
void CEOnSkeletonUpdated::PushArguments(lua::State *l) { Lua::PushInt(l, physRootBoneId); }
uint32_t CEOnSkeletonUpdated::GetReturnCount() { return 1u; }
void CEOnSkeletonUpdated::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -1))
		physRootBoneId = Lua::CheckInt(l, -1);
}

//////////////

CEOnBoneBufferInitialized::CEOnBoneBufferInitialized(const std::shared_ptr<prosper::SwapBuffer> &buffer) : buffer {buffer} {}
void CEOnBoneBufferInitialized::PushArguments(lua::State *l) { Lua::Push<std::shared_ptr<Lua::Vulkan::SwapBuffer>>(l, buffer); }

void CAnimatedComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseAnimatedComponent::RegisterLuaBindings(l, modEnts);
	auto defCAnimated = pragma::LuaCore::create_entity_component_class<CAnimatedComponent, BaseAnimatedComponent>("AnimatedComponent");
	defCAnimated.def("GetBoneRenderMatrices", static_cast<const std::vector<Mat4> &(CAnimatedComponent::*)() const>(&CAnimatedComponent::GetBoneMatrices));
	defCAnimated.def("GetBoneRenderMatrix", static_cast<std::optional<Mat4> (*)(lua::State *, CAnimatedComponent &, uint32_t)>([](lua::State *l, CAnimatedComponent &hAnim, uint32_t boneIndex) -> std::optional<Mat4> {
		auto &mats = hAnim.GetBoneMatrices();
		if(boneIndex >= mats.size())
			return {};
		return mats.at(boneIndex);
	}));
	defCAnimated.def("SetBoneRenderMatrix", static_cast<void (*)(lua::State *, CAnimatedComponent &, uint32_t, const Mat4 &)>([](lua::State *l, CAnimatedComponent &hAnim, uint32_t boneIndex, const Mat4 &m) {
		auto &mats = hAnim.GetBoneMatrices();
		if(boneIndex >= mats.size())
			return;
		mats.at(boneIndex) = m;
	}));
	defCAnimated.def("GetLocalVertexPosition",
	  static_cast<std::optional<Vector3> (*)(lua::State *, CAnimatedComponent &, std::shared_ptr<geometry::ModelSubMesh> &, uint32_t)>([](lua::State *l, CAnimatedComponent &hAnim, std::shared_ptr<geometry::ModelSubMesh> &subMesh, uint32_t vertexId) -> std::optional<Vector3> {
		  Vector3 pos, n;
		  if(vertexId >= subMesh->GetVertexCount())
			  return {};
		  auto &v = subMesh->GetVertices()[vertexId];
		  pos = v.position;
		  n = v.normal;
		  auto b = hAnim.GetLocalVertexPosition(static_cast<geometry::CModelSubMesh &>(*subMesh), vertexId, pos, n);
		  if(b == false)
			  return {};
		  return pos;
	  }));
	defCAnimated.def("AreSkeletonUpdateCallbacksEnabled", &CAnimatedComponent::AreSkeletonUpdateCallbacksEnabled);
	defCAnimated.def("SetSkeletonUpdateCallbacksEnabled", &CAnimatedComponent::SetSkeletonUpdateCallbacksEnabled);
	defCAnimated.add_static_constant("EVENT_ON_SKELETON_UPDATED", cAnimatedComponent::EVENT_ON_SKELETON_UPDATED);
	defCAnimated.add_static_constant("EVENT_ON_BONE_MATRICES_UPDATED", cAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED);
	modEnts[defCAnimated];
}
