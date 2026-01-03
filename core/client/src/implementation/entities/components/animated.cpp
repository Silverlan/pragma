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
ComponentEventId cAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED = INVALID_COMPONENT_ID;
void CAnimatedComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseAnimatedComponent::RegisterEvents(componentManager, registerEvent);
	cAnimatedComponent::EVENT_ON_SKELETON_UPDATED = registerEvent("ON_SKELETON_UPDATED", ComponentEventInfo::Type::Explicit);
	cAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED = registerEvent("ON_BONE_MATRICES_UPDATED", ComponentEventInfo::Type::Explicit);
	cAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED = registerEvent("ON_BONE_BUFFER_INITIALIZED", ComponentEventInfo::Type::Broadcast);
}
void CAnimatedComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseAnimatedComponent)); }
void CAnimatedComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
static std::shared_ptr<prosper::IUniformResizableBuffer> s_instanceBoneBuffer = nullptr;
const std::shared_ptr<prosper::IUniformResizableBuffer> &pragma::get_instance_bone_buffer() { return s_instanceBoneBuffer; }
void pragma::initialize_articulated_buffers()
{
	auto instanceSize = math::to_integral(GameLimits::MaxBones) * sizeof(Mat4);
	auto instanceCount = 512u;
	auto maxInstanceCount = instanceCount * 4u;
	prosper::util::BufferCreateInfo createInfo {};

	if constexpr(CRenderComponent::USE_HOST_MEMORY_FOR_RENDER_DATA) {
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::HostAccessable | prosper::MemoryFeatureFlags::HostCoherent;
		createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	}
	else
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	createInfo.size = instanceSize * maxInstanceCount;
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	s_instanceBoneBuffer = get_cengine()->GetRenderContext().CreateUniformResizableBuffer(createInfo, instanceSize, instanceSize * maxInstanceCount, 0.05f);
	s_instanceBoneBuffer->SetDebugName("entity_anim_bone_buf");

	if constexpr(CRenderComponent::USE_HOST_MEMORY_FOR_RENDER_DATA)
		s_instanceBoneBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit | prosper::IBuffer::MapFlags::Unsynchronized);
}
void pragma::clear_articulated_buffers() { s_instanceBoneBuffer = nullptr; }

void CAnimatedComponent::SetBoneBufferDirty() { math::set_flag(m_stateFlags, StateFlags::BoneBufferDirty); }
void CAnimatedComponent::SetSkeletonUpdateCallbacksEnabled(bool enabled) { math::set_flag(m_stateFlags, StateFlags::EnableSkeletonUpdateCallbacks, enabled); }
bool CAnimatedComponent::AreSkeletonUpdateCallbacksEnabled() const { return math::is_flag_set(m_stateFlags, StateFlags::EnableSkeletonUpdateCallbacks); }

void CAnimatedComponent::Initialize()
{
	BaseAnimatedComponent::Initialize();
	InitializeBoneBuffer();

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
	BindEventUnhandled(cRenderComponent::EVENT_ON_UPDATE_RENDER_BUFFERS, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto isDirty = math::is_flag_set(m_stateFlags, StateFlags::BoneBufferDirty);
		math::set_flag(m_stateFlags, StateFlags::BoneBufferDirty, false);
		UpdateBoneBuffer(*static_cast<CEOnUpdateRenderBuffers &>(evData.get()).commandBuffer, isDirty);
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
	}
}

void CAnimatedComponent::OnRemove()
{
	BaseAnimatedComponent::OnRemove();
	auto pRenderComponent = GetEntity().GetComponent<CRenderComponent>();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderBufferDirty();
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

void CAnimatedComponent::ResetAnimation(const std::shared_ptr<asset::Model> &mdl)
{
	BaseAnimatedComponent::ResetAnimation(mdl);
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

const prosper::IBuffer *CAnimatedComponent::GetBoneBuffer() const { return m_boneBuffer.get(); }
void CAnimatedComponent::InitializeBoneBuffer()
{
	if(m_boneBuffer != nullptr)
		return;
	m_boneBuffer = get_instance_bone_buffer()->AllocateBuffer();

	CEOnBoneBufferInitialized evData {m_boneBuffer};
	BroadcastEvent(cAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED, evData);
}
void CAnimatedComponent::UpdateBoneBuffer(prosper::IPrimaryCommandBuffer &commandBuffer, bool flagAsDirty)
{
	auto numBones = GetBoneCount();
	if(m_boneBuffer && flagAsDirty && numBones > 0u && m_boneMatrices.empty() == false) {
		constexpr auto pipelineStages = prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::ComputeShaderBit | prosper::PipelineStageFlags::GeometryShaderBit;
		commandBuffer.RecordBufferBarrier(*m_boneBuffer, pipelineStages, prosper::PipelineStageFlags::TransferBit, prosper::AccessFlags::UniformReadBit, prosper::AccessFlags::TransferWriteBit);
		commandBuffer.RecordUpdateBuffer(*m_boneBuffer, 0ull, numBones * sizeof(Mat4), m_boneMatrices.data());
		commandBuffer.RecordBufferBarrier(*m_boneBuffer, prosper::PipelineStageFlags::TransferBit, pipelineStages, prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::UniformReadBit);
	}
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
	if(numBones != m_processedBones.size()) {
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

CEOnBoneBufferInitialized::CEOnBoneBufferInitialized(const std::shared_ptr<prosper::IBuffer> &buffer) : buffer {buffer} {}
void CEOnBoneBufferInitialized::PushArguments(lua::State *l) { Lua::Push<std::shared_ptr<Lua::Vulkan::Buffer>>(l, buffer); }

void CAnimatedComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseAnimatedComponent::RegisterLuaBindings(l, modEnts);
	auto defCAnimated = pragma::LuaCore::create_entity_component_class<CAnimatedComponent, BaseAnimatedComponent>("AnimatedComponent");
	defCAnimated.def(
	  "GetBoneBuffer", +[](lua::State *l, CAnimatedComponent &hAnim) -> std::optional<std::shared_ptr<prosper::IBuffer>> {
		  auto *buf = hAnim.GetBoneBuffer();
		  if(!buf)
			  return {};
		  return const_cast<prosper::IBuffer *>(buf)->shared_from_this();
	  });
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
	defCAnimated.add_static_constant("EVENT_ON_BONE_BUFFER_INITIALIZED", cAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED);
	modEnts[defCAnimated];
}
