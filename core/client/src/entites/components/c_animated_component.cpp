#include "stdafx_client.h"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <pragma/model/model.h>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
ComponentEventId CAnimatedComponent::EVENT_ON_SKELETON_UPDATED = INVALID_COMPONENT_ID;
ComponentEventId CAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED = INVALID_COMPONENT_ID;
ComponentEventId CAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED = INVALID_COMPONENT_ID;
ComponentEventId CAnimatedComponent::EVENT_ON_EYEBALLS_UPDATED = INVALID_COMPONENT_ID;
void CAnimatedComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	BaseAnimatedComponent::RegisterEvents(componentManager);
	EVENT_ON_SKELETON_UPDATED = componentManager.RegisterEvent("ON_SKELETON_UPDATED",std::type_index(typeid(CAnimatedComponent)));
	EVENT_ON_BONE_MATRICES_UPDATED = componentManager.RegisterEvent("ON_BONE_MATRICES_UPDATED",std::type_index(typeid(CAnimatedComponent)));
	EVENT_ON_EYEBALLS_UPDATED = componentManager.RegisterEvent("ON_EYEBALLS_UPDATED",std::type_index(typeid(CAnimatedComponent)));
	EVENT_ON_BONE_BUFFER_INITIALIZED = componentManager.RegisterEvent("ON_BONE_BUFFER_INITIALIZED");
}
void CAnimatedComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const {outTypeIndex = std::type_index(typeid(BaseAnimatedComponent));}
luabind::object CAnimatedComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CAnimatedComponentHandleWrapper>(l);}
static std::shared_ptr<prosper::UniformResizableBuffer> s_instanceBoneBuffer = nullptr;
const std::shared_ptr<prosper::UniformResizableBuffer> &pragma::get_instance_bone_buffer() {return s_instanceBoneBuffer;}
void pragma::initialize_articulated_buffers()
{
	auto instanceSize = umath::to_integral(GameLimits::MaxBones) *sizeof(Mat4);
	auto instanceCount = 512u;
	auto maxInstanceCount = instanceCount *4u;
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.size = instanceSize *maxInstanceCount;
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_SRC_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT;
#endif
	s_instanceBoneBuffer = prosper::util::create_uniform_resizable_buffer(*c_engine,createInfo,instanceSize,instanceSize *maxInstanceCount,0.05f);
	s_instanceBoneBuffer->SetDebugName("entity_anim_bone_buf");
}
void pragma::clear_articulated_buffers() {s_instanceBoneBuffer = nullptr;}

void CAnimatedComponent::SetBoneBufferDirty() {umath::set_flag(m_stateFlags,StateFlags::BoneBufferDirty);}

void CAnimatedComponent::Initialize()
{
	BaseAnimatedComponent::Initialize();
	InitializeBoneBuffer();

	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto pRenderComponent = GetEntity().GetComponent<CRenderComponent>();
		if(pRenderComponent.valid())
			pRenderComponent->UpdateRenderBounds();
	});
	BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(umath::is_flag_set(m_stateFlags,StateFlags::BoneBufferDirty) == false)
			return;
		umath::set_flag(m_stateFlags,StateFlags::BoneBufferDirty,false);
		UpdateBoneBuffer(*static_cast<pragma::CEOnUpdateRenderData&>(evData.get()).commandBuffer);
	});
	auto &ent = GetEntity();
	ent.AddComponent<LogicComponent>();
	auto pRenderComponent = ent.GetComponent<CRenderComponent>();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderBufferDirty();
}

void CAnimatedComponent::OnRemove()
{
	BaseAnimatedComponent::OnRemove();
	auto pRenderComponent = GetEntity().GetComponent<CRenderComponent>();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderBufferDirty();
}

void CAnimatedComponent::ReceiveData(NetPacket &packet)
{
	int anim = packet->Read<int>();
	float cycle = packet->Read<float>();
	PlayAnimation(anim);
	SetCycle(cycle);
}

std::optional<Mat4> CAnimatedComponent::GetVertexTransformMatrix(const ModelSubMesh &subMesh,uint32_t vertexId) const
{
	auto t = BaseAnimatedComponent::GetVertexTransformMatrix(subMesh,vertexId);
	if(t.has_value() == false)
		return {};
	Vector3 vertexOffset;
	auto pVertexAnimatedComponent = GetEntity().GetComponent<pragma::CVertexAnimatedComponent>();
	if(pVertexAnimatedComponent.expired() || pVertexAnimatedComponent->GetLocalVertexPosition(subMesh,vertexId,vertexOffset) == false)
		return t;
	return *t *glm::translate(umat::identity(),vertexOffset); // TODO: Confirm order!
}

void CAnimatedComponent::OnModelChanged(const std::shared_ptr<Model> &mdl)
{
	BaseAnimatedComponent::OnModelChanged(mdl);
	m_boneMatrices.clear();
	m_eyeballData.clear();
	if(mdl == nullptr || GetBoneCount() == 0)
		return;
	m_boneMatrices.resize(mdl->GetBoneCount(),umat::identity());
	m_eyeballData.resize(mdl->GetEyeballCount());
	UpdateBoneMatrices();
	SetBoneBufferDirty();

	// Attach particles defined in the model
	auto &ent = GetEntity();
	auto pParentComponent = ent.GetComponent<CParentComponent>();
	for(auto &objAttachment : mdl->GetObjectAttachments())
	{
		switch(objAttachment.type)
		{
			case ObjectAttachment::Type::Model:
				Con::cwar<<"WARNING: Unsupported object attachment type '"<<umath::to_integral(objAttachment.type)<<"'!"<<Con::endl;
				break;
			case ObjectAttachment::Type::ParticleSystem:
				auto itParticleFile = objAttachment.keyValues.find("particle_file");
				if(itParticleFile != objAttachment.keyValues.end())
					CParticleSystemComponent::Precache(itParticleFile->second);
				auto itParticle = objAttachment.keyValues.find("particle");
				if(itParticle != objAttachment.keyValues.end())
				{
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

					auto *pt = CParticleSystemComponent::Create(itParticle->second);
					if(pt != nullptr)
					{
						auto &entPt = pt->GetEntity();
						auto pAttachableComponent = entPt.AddComponent<CAttachableComponent>();
						if(pAttachableComponent.valid())
						{
							auto pTrComponentPt = entPt.GetTransformComponent();
							if(pTrComponentPt.valid())
							{
								pTrComponentPt->SetPosition(translation);
								pTrComponentPt->SetOrientation(rotation);
							}
							pAttachableComponent->AttachToAttachment(&ent,objAttachment.attachment);
						}
						pt->Start();
					}
				}
				break;
		}
	}
}

std::weak_ptr<prosper::Buffer> CAnimatedComponent::GetBoneBuffer() const {return m_boneBuffer;}
void CAnimatedComponent::InitializeBoneBuffer()
{
	if(m_boneBuffer != nullptr)
		return;
	m_boneBuffer = pragma::get_instance_bone_buffer()->AllocateBuffer();

	CEOnBoneBufferInitialized evData{m_boneBuffer};
	BroadcastEvent(EVENT_ON_BONE_BUFFER_INITIALIZED,evData);
}
void CAnimatedComponent::UpdateBoneBuffer(prosper::PrimaryCommandBuffer &commandBuffer)
{
	// Update Bone Buffer
	auto wpBoneBuffer = GetBoneBuffer();
	auto numBones = GetBoneCount();
	if(wpBoneBuffer.expired() == false && numBones > 0u && m_boneMatrices.empty() == false)
	{
		// Update bone buffer
		auto buffer = wpBoneBuffer.lock();
		prosper::util::record_update_generic_shader_read_buffer(*commandBuffer,*buffer,0ull,GetBoneCount() *sizeof(Mat4),m_boneMatrices.data());
	}
}
const std::vector<Mat4> &CAnimatedComponent::GetBoneMatrices() const {return const_cast<CAnimatedComponent*>(this)->GetBoneMatrices();}
std::vector<Mat4> &CAnimatedComponent::GetBoneMatrices() {return m_boneMatrices;}

bool CAnimatedComponent::MaintainAnimations(double dt)
{
#pragma message("TODO: Undo this and do it properly!")
	//if(BaseEntity::MaintainAnimations() == false)
	//	return false;
	if(ShouldUpdateBones() == false)
		return false;
	BaseAnimatedComponent::MaintainAnimations(dt);

	UpdateBoneMatrices();
	SetBoneBufferDirty();
	return true;
}

void CAnimatedComponent::UpdateBoneMatrices()
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	if(m_boneMatrices.empty())
		return;
	UpdateSkeleton(); // Costly
	auto physRootBoneId = OnSkeletonUpdated();

	CEOnSkeletonUpdated evData{physRootBoneId};
	InvokeEventCallbacks(EVENT_ON_SKELETON_UPDATED,evData);

	auto &refFrame = mdl->GetReference();
	for(unsigned int i=0;i<GetBoneCount();i++)
	{
		auto &t = m_processedBones.at(i);
		auto &pos = t.GetOrigin();
		auto &orientation = t.GetRotation();
		auto &scale = t.GetScale();

		auto *posBind = refFrame.GetBonePosition(i);
		auto *rotBind = refFrame.GetBoneOrientation(i);
		if(posBind != nullptr && rotBind != nullptr)
		{
			auto &mat = m_boneMatrices[i];
			if(i != physRootBoneId)
			{
				physics::Transform tBindPose {*posBind,*rotBind};
				tBindPose = tBindPose.GetInverse();
				physics::ScaledTransform tBone {pos,orientation,scale};

				mat = (tBone *tBindPose).ToMatrix();
			}
			else
				mat = umat::identity();
		}
		else
			Con::cwar<<"WARNING: Attempted to update bone "<<i<<" in "<<mdl->GetName()<<" which doesn't exist in the reference pose! Ignoring..."<<Con::endl;
	}
	InvokeEventCallbacks(EVENT_ON_BONE_MATRICES_UPDATED);
}

uint32_t CAnimatedComponent::OnSkeletonUpdated() {return std::numeric_limits<uint32_t>::max();}

//////////////

CEOnSkeletonUpdated::CEOnSkeletonUpdated(uint32_t &physRootBoneId)
	: physRootBoneId{physRootBoneId}
{}
void CEOnSkeletonUpdated::PushArguments(lua_State *l)
{
	Lua::PushInt(l,physRootBoneId);
}
uint32_t CEOnSkeletonUpdated::GetReturnCount() {return 1u;}
void CEOnSkeletonUpdated::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		physRootBoneId = Lua::CheckInt(l,-1);
}

//////////////

CEOnBoneBufferInitialized::CEOnBoneBufferInitialized(const std::shared_ptr<prosper::Buffer> &buffer)
	: buffer{buffer}
{}
void CEOnBoneBufferInitialized::PushArguments(lua_State *l)
{
	Lua::Push<std::shared_ptr<Lua::Vulkan::Buffer>>(l,buffer);
}
#pragma optimize("",on)
