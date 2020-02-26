#include "stdafx_client.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/c_softbody_component.hpp"
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

using namespace pragma;

namespace pragma
{
	using ::operator|=;
};

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
static std::shared_ptr<prosper::UniformResizableBuffer> s_instanceBuffer = nullptr;
decltype(CRenderComponent::s_viewEntities) CRenderComponent::s_viewEntities = {};
ComponentEventId CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_SHOULD_DRAW = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_SHOULD_DRAW_SHADOW = INVALID_COMPONENT_ID;
ComponentEventId CRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES = INVALID_COMPONENT_ID;
void CRenderComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_UPDATE_RENDER_DATA = componentManager.RegisterEvent("ON_UPDATE_RENDER_DATA",std::type_index(typeid(CRenderComponent)));
	EVENT_ON_RENDER_BUFFERS_INITIALIZED = componentManager.RegisterEvent("ON_RENDER_BUFFERS_INITIALIZED");
	EVENT_ON_RENDER_BOUNDS_CHANGED = componentManager.RegisterEvent("ON_RENDER_BUFFERS_INITIALIZED");
	EVENT_SHOULD_DRAW = componentManager.RegisterEvent("SHOULD_DRAW",std::type_index(typeid(CRenderComponent)));
	EVENT_SHOULD_DRAW_SHADOW = componentManager.RegisterEvent("SHOULD_DRAW_SHADOW",std::type_index(typeid(CRenderComponent)));
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
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.size = instanceSize *instanceCount;
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_SRC_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT;
#endif
	s_instanceBuffer = prosper::util::create_uniform_resizable_buffer(*c_engine,createInfo,instanceSize,instanceSize *maxInstanceCount,0.1f);
	s_instanceBuffer->SetDebugName("entity_instance_data_buf");

	pragma::initialize_articulated_buffers();
}
std::weak_ptr<prosper::Buffer> CRenderComponent::GetRenderBuffer() const {return m_renderBuffer;}
prosper::DescriptorSet *CRenderComponent::GetRenderDescriptorSet() const {return (m_renderDescSetGroup != nullptr) ? m_renderDescSetGroup->GetDescriptorSet() : nullptr;}
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
		auto lorigin = pPhysComponent.valid() ? pPhysComponent->GetLocalOrigin() : Vector3{};
		rMin += lorigin;
		rMax += lorigin;
		SetRenderBounds(rMin,rMax);
		UpdateRenderBounds();

		UpdateRenderMeshes();
		m_renderMode->InvokeCallbacks();

		// TODO Build LOD cache!
		/*RenderMode renderMode = GetRenderMode();
		for(unsigned int i=0;i<model->GetLODCount();i++)
		{
			RenderInstance *instance = RenderSystem::CreateRenderInstance(renderMode);
			instance->userData = this;
			instance->SetRenderCallback(&CBaseEntity::RenderCallback);
			std::vector<ModelMesh*> meshes;
			model->GetMeshes(i,&meshes);
			for(unsigned int i=0;i<meshes.size();i++)
			{
				CModelMesh *mesh = static_cast<CModelMesh*>(meshes[i]);
				GLMesh &glMesh = mesh->GetGLMesh();
				unsigned int texture = mesh->GetTexture();
				Material *mat = model->GetMaterial(GetSkin(),texture);
				if(mat != NULL)
				{
					void *data = mat->GetUserData();
					Shader3DTexturedBase *shader = NULL;
					if(data != NULL)
						shader = dynamic_cast<Shader3DTexturedBase*>(static_cast<ShaderBase*>(data));
					if(shader != NULL)
						instance->CreateRenderObject(shader,mat,&glMesh);
				}
			}
			if(i > 0)
				instance->SetEnabled(false);
			m_renderInstances.insert(std::unordered_map<unsigned int,RenderInstance*>::value_type(model->GetLOD(i),instance));
		}*/ // Vulkan TODO
	});
}
CRenderComponent::~CRenderComponent()
{
	ClearRenderObjects();
	auto it = std::find(s_viewEntities.begin(),s_viewEntities.end(),this);
	if(it != s_viewEntities.end())
		s_viewEntities.erase(it);

	if(m_renderBuffer != nullptr)
		c_engine->KeepResourceAliveUntilPresentationComplete(m_renderBuffer);
	if(m_renderDescSetGroup != nullptr)
		c_engine->KeepResourceAliveUntilPresentationComplete(m_renderDescSetGroup);
}
void CRenderComponent::OnEntitySpawn()
{
	BaseRenderComponent::OnEntitySpawn();
	UpdateRenderMeshes();
}
Sphere CRenderComponent::GetRenderSphereBounds() const
{
	auto r = m_renderSphere;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.valid())
	{
		auto scale = pTrComponent->GetScale();
		r.radius *= umath::abs_max(scale.x,scale.y,scale.z);
	}
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.expired())
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
	physics::Transform pose;
	GetEntity().GetPose(pose);
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
	if(ptrComponent.valid())
	{
		auto &scale = ptrComponent->GetScale();
		*min *= scale;
		*max *= scale;
	}
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.expired())
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
	if(ptrComponent.valid())
	{
		auto &scale = ptrComponent->GetScale();
		*min = scale;
		*max = scale;
	}
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.expired())
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
	auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || pPhysComponent->GetPhysicsType() != PHYSICSTYPE::SOFTBODY || !phys->IsSoftBody())
		AABB::GetRotatedBounds(m_renderMin,m_renderMax,GetRotationMatrix(),&m_renderMinRot,&m_renderMaxRot); // TODO: Use orientation
	else
	{
		phys->GetAABB(m_renderMin,m_renderMax);
		m_renderMinRot = m_renderMin;
		m_renderMaxRot = m_renderMax;
	}
}

Mat4 &CRenderComponent::GetModelMatrix() {return m_matModel;}
Mat4 &CRenderComponent::GetTranslationMatrix() {return m_matTranslation;}
Mat4 &CRenderComponent::GetRotationMatrix() {return m_matRotation;}
Mat4 &CRenderComponent::GetTransformationMatrix() {return m_matTransformation;}
void CRenderComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseRenderComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(pragma::CTransformComponent))
	{
		FlagCallbackForRemoval(static_cast<pragma::CTransformComponent&>(component).GetPosProperty()->AddCallback([this](std::reference_wrapper<const Vector3> oldPos,std::reference_wrapper<const Vector3> pos) {
			SetRenderBufferDirty();
		}),CallbackType::Component,&component);
		FlagCallbackForRemoval(static_cast<pragma::CTransformComponent&>(component).GetOrientationProperty()->AddCallback([this](std::reference_wrapper<const Quat> oldRot,std::reference_wrapper<const Quat> rot) {
			SetRenderBufferDirty();
		}),CallbackType::Component,&component);
		FlagCallbackForRemoval(static_cast<pragma::CTransformComponent&>(component).GetScaleProperty()->AddCallback([this](std::reference_wrapper<const Vector3> oldScale,std::reference_wrapper<const Vector3> scale) {
			SetRenderBufferDirty();
		}),CallbackType::Component,&component);
	}
	else if(typeid(component) == typeid(pragma::CModelComponent))
		m_mdlComponent = component.GetHandle<CModelComponent>();
}
void CRenderComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseRenderComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(pragma::CModelComponent))
		m_mdlComponent = {};
}
util::WeakHandle<CModelComponent> &CRenderComponent::GetModelComponent() const {return m_mdlComponent;}
void CRenderComponent::UpdateMatrices()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto orientation = pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.expired() || pPhysComponent->GetPhysicsType() != PHYSICSTYPE::SOFTBODY)
	{
		m_matTranslation = glm::translate(umat::identity(),pPhysComponent.valid() ? pPhysComponent->GetOrigin() : pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{});
		m_matRotation = umat::create(orientation);
	}
	else
	{
		m_matTranslation = umat::identity();
		m_matRotation = umat::identity();
	}
	auto scale = pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};
	m_matTransformation = m_matTranslation *m_matRotation *glm::scale(umat::identity(),scale);

	CEOnUpdateRenderMatrices evData{m_matTranslation,m_matRotation,m_matTransformation};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_MATRICES,evData);
}
unsigned long long &CRenderComponent::GetLastRenderFrame() {return m_lastRender;}
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
void CRenderComponent::PostRender(RenderMode) {}
bool CRenderComponent::Render(pragma::ShaderTextured3DBase*,Material*,CModelSubMesh*) {return false;}
void CRenderComponent::ReceiveData(NetPacket &packet)
{
	m_renderFlags = packet->Read<decltype(m_renderFlags)>();
}
std::optional<Intersection::LineMeshResult> CRenderComponent::CalcRayIntersection(const Vector3 &start,const Vector3 &dir) const
{
	auto &lodMeshes = GetLODMeshes();
	if(lodMeshes.empty())
		return {};
	physics::Transform pose;
	GetEntity().GetPose(pose);
	auto invPose = pose.GetInverse();

	// Move ray into entity space
	auto lstart = invPose *start;
	auto ldir = dir;
	uvec::rotate(&ldir,invPose.GetRotation());

	// Cheap line-aabb check
	Vector3 min,max;
	GetRenderBounds(&min,&max);
	auto n = ldir;
	auto d = uvec::length(n);
	n /= d;
	float dIntersect;
	if(Intersection::LineAABB(lstart,n,min,max,&dIntersect) == Intersection::Result::NoIntersection || dIntersect > d)
		return {};

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
void CRenderComponent::SetExemptFromOcclusionCulling(bool exempt) {umath::set_flag(m_stateFlags,StateFlags::ExemptFromOcclusionCulling,exempt);}
bool CRenderComponent::IsExemptFromOcclusionCulling() const {return umath::is_flag_set(m_stateFlags,StateFlags::ExemptFromOcclusionCulling);}
void CRenderComponent::SetRenderBufferDirty() {umath::set_flag(m_stateFlags,StateFlags::RenderBufferDirty);}
void CRenderComponent::UpdateRenderData(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,bool bForceBufferUpdate)
{
	InitializeRenderBuffers();

	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto frameId = c_engine->GetLastFrameId();

	auto firstFrame = (frameId != m_lastRender);
	if(firstFrame)
	{
		auto pFlexComponent = ent.GetComponent<pragma::CFlexComponent>();
		if(pFlexComponent.valid())
			pFlexComponent->UpdateFlexWeights(); // TODO: Move this to CFlexComponent code

		auto pVertexAnimatedComponent = ent.GetComponent<pragma::CVertexAnimatedComponent>();
		if(pVertexAnimatedComponent.valid())
			pVertexAnimatedComponent->UpdateVertexAnimationBuffer(drawCmd); // TODO: Move this to CVertexAnimatedComponent code
		auto pAttComponent = ent.GetComponent<CAttachableComponent>();
		if(pAttComponent.valid())
		{
			auto *attInfo = pAttComponent->GetAttachmentData();
			if(attInfo != nullptr && (attInfo->flags &FAttachmentMode::UpdateEachFrame) != FAttachmentMode::None && attInfo->parent.valid())
				pAttComponent->UpdateAttachmentOffset();
		}
	}

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
			auto pColorComponent = ent.GetComponent<CColorComponent>();
			if(pColorComponent.valid())
				color = pColorComponent->GetColor().ToVector4();

			auto renderFlags = pragma::ShaderEntity::InstanceData::RenderFlags::None;
			auto &pMdlComponent = GetModelComponent();
			auto bWeighted = pMdlComponent.valid() && static_cast<const pragma::CModelComponent&>(*pMdlComponent).IsWeighted();
			if(bWeighted == true)
				renderFlags |= pragma::ShaderEntity::InstanceData::RenderFlags::Weighted;
			auto &m = GetTransformationMatrix();
			pragma::ShaderEntity::InstanceData instanceData {m,color,renderFlags};
			prosper::util::record_update_generic_shader_read_buffer(**drawCmd,*renderBuffer,0ull,sizeof(instanceData),&instanceData);
		}
	}
	m_lastRender = frameId;

	CEOnUpdateRenderData evData {drawCmd,updateRenderBuffer,firstFrame};
	InvokeEventCallbacks(EVENT_ON_UPDATE_RENDER_DATA,evData);
}

void CRenderComponent::Render(RenderMode) {}
void CRenderComponent::SetRenderMode(RenderMode mode)
{
	if(mode == **m_renderMode)
		return;
	*m_renderMode = mode;

	auto it = std::find(s_viewEntities.begin(),s_viewEntities.end(),this);
	if(mode == RenderMode::View)
	{
		if(it == s_viewEntities.end())
			s_viewEntities.push_back(this);
	}
	else if(it != s_viewEntities.end())
		s_viewEntities.erase(it);

	if(mode == RenderMode::None)
		ClearRenderBuffers();
	/*std::unordered_map<unsigned int,RenderInstance*>::iterator it;
	for(it=m_renderInstances.begin();it!=m_renderInstances.end();it++)
	{
		RenderInstance *instance = it->second;
		instance->SetRenderMode(mode);
	}*/ // Vulkan TODO
}
void CRenderComponent::InitializeRenderBuffers()
{
	// Initialize render buffer if it doesn't exist
	if(m_renderBuffer != nullptr || pragma::ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE.IsValid() == false)
		return;
	auto &dev = c_engine->GetDevice();
	m_renderBuffer = s_instanceBuffer->AllocateBuffer();
	m_renderDescSetGroup = prosper::util::create_descriptor_set_group(dev,pragma::ShaderTextured3DBase::DESCRIPTOR_SET_INSTANCE);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		*m_renderDescSetGroup->GetDescriptorSet(),*m_renderBuffer,umath::to_integral(pragma::ShaderTextured3DBase::InstanceBinding::Instance)
	);
	UpdateBoneBuffer();
	(*m_renderDescSetGroup)->get_descriptor_set(0u)->update();

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
	auto &dev = c_engine->GetDevice();
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		*m_renderDescSetGroup->GetDescriptorSet(),*wpBoneBuffer.lock(),umath::to_integral(pragma::ShaderTextured3DBase::InstanceBinding::BoneMatrices)
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

const std::vector<std::shared_ptr<ModelMesh>> &CRenderComponent::GetLODMeshes() const {return const_cast<CRenderComponent*>(this)->GetLODMeshes();}
std::vector<std::shared_ptr<ModelMesh>> &CRenderComponent::GetLODMeshes()
{
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pSoftBodyComponent = ent.GetComponent<pragma::CSoftBodyComponent>();
	if(pSoftBodyComponent.valid())
	{
		auto *pSoftBodyData = pSoftBodyComponent->GetSoftBodyData();
		if(pSoftBodyData != nullptr)
			return pSoftBodyData->meshes;
	}
	auto &pMdlComponent = GetModelComponent();
	if(pMdlComponent.expired())
	{
		static std::vector<std::shared_ptr<ModelMesh>> meshes {};
		meshes.clear();
		return meshes;
	}
	return static_cast<pragma::CModelComponent&>(*pMdlComponent).GetLODMeshes();
}
bool CRenderComponent::RenderCallback(RenderObject *o,CBaseEntity *ent,pragma::CCameraComponent *cam,pragma::ShaderTextured3DBase *shader,Material *mat)
{
	auto pRenderComponent = ent->GetRenderComponent();
	return pRenderComponent.valid() && pRenderComponent->RenderCallback(o,cam,shader,mat);
}
bool CRenderComponent::RenderCallback(RenderObject*,pragma::CCameraComponent *cam,pragma::ShaderTextured3DBase*,Material*)
{
	return ShouldDraw(cam->GetEntity().GetPosition());
}
void CRenderComponent::PreRender()
{
	/*if(m_parent == NULL || (m_parent->flags &FPARENT_UPDATE_EACH_FRAME) != FPARENT_UPDATE_EACH_FRAME || !m_parent->parent->IsValid())
		return;
	UpdateParentOffset();
	UpdateMatrices();*/ // Obsolete // Vulkan TODO
}
const std::vector<CRenderComponent*> &CRenderComponent::GetViewEntities() {return s_viewEntities;}
const std::shared_ptr<prosper::UniformResizableBuffer> &CRenderComponent::GetInstanceBuffer() {return s_instanceBuffer;}
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

CEOnUpdateRenderMatrices::CEOnUpdateRenderMatrices(Mat4 &translation,Mat4 &rotation,Mat4 &transformation)
	: translation{translation},rotation{rotation},transformation{transformation}
{}
void CEOnUpdateRenderMatrices::PushArguments(lua_State *l)
{
	Lua::Push<Mat4>(l,translation);
	Lua::Push<Mat4>(l,rotation);
	Lua::Push<Mat4>(l,transformation);
}
uint32_t CEOnUpdateRenderMatrices::GetReturnCount() {return 3;}
void CEOnUpdateRenderMatrices::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-3))
		translation = *Lua::CheckMat4(l,-3);
	if(Lua::IsSet(l,-2))
		translation = *Lua::CheckMat4(l,-2);
	if(Lua::IsSet(l,-1))
		translation = *Lua::CheckMat4(l,-1);
}

/////////////////

CEOnUpdateRenderData::CEOnUpdateRenderData(const std::shared_ptr<prosper::PrimaryCommandBuffer> &commandBuffer,bool bufferUpdateRequired,bool firstUpdateThisFrame)
	: bufferUpdateRequired{bufferUpdateRequired},commandBuffer{commandBuffer},firstUpdateThisFrame{firstUpdateThisFrame}
{}
void CEOnUpdateRenderData::PushArguments(lua_State *l)
{
	Lua::PushBool(l,bufferUpdateRequired);
	Lua::PushBool(l,firstUpdateThisFrame);
}

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
#pragma optimize("",on)
