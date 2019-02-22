#include "stdafx_client.h"

// TODO: Remove this file
#if 0
#include "pragma/game/c_game.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/rendering/lighting/c_light.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include <mathutil/umat.h>
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/uniformbinding.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/c_world.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_generic_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include <shaderinfo.h>
#include <prosper_util.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/model/model.h>

DEFINE_BASE_HANDLE(DLLCLIENT,CLightBase,CLightObject);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

CLightBase::MeshInfo::MeshInfo(CModelSubMesh *_mesh,uint32_t _renderFlags)
	: mesh(_mesh),renderFlags(_renderFlags)
{}
CLightBase::EntityInfo::EntityInfo(CLightBase *light,BaseEntity *ent,uint32_t _renderFlags)
	: hEntity(ent->GetHandle()),tLastMoved(0.0),renderFlags(_renderFlags)
{
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent.valid())
		tLastMoved = pTrComponent->GetLastMoveTime();
	auto mdlComponent = ent->GetModelComponent();
	if(mdlComponent.valid() && mdlComponent->HasModelMaterialsLoaded() == false)
	{
		auto pGenericComponent = ent->GetComponent<pragma::CGenericComponent>();
		if(pGenericComponent.valid())
		{
			hCbMaterialsLoaded = pGenericComponent->BindEventUnhandled(pragma::CModelComponent::EVENT_ON_MODEL_MATERIALS_LOADED,[light,pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				light->UpdateEntity(static_cast<CBaseEntity*>(&pGenericComponent->GetEntity()));
			});
		}
	}
}
CLightBase::EntityInfo::~EntityInfo()
{
	if(hCbMaterialsLoaded.IsValid())
		hCbMaterialsLoaded.Remove();
}

CLightBase::ParticleInfo::ParticleInfo(pragma::CParticleSystemComponent &pt)
	: hParticle(pt.GetHandle<pragma::CParticleSystemComponent>())
{}

std::vector<std::shared_ptr<CLightBase::EntityInfo>>::iterator CLightBase::ShadowInfoSet::FindEntity(CBaseEntity *ent)
{
	return std::find_if(meshInfo.begin(),meshInfo.end(),[ent](std::shared_ptr<EntityInfo> &info) {
		return (info->hEntity.IsValid() && info->hEntity.get() == ent) ? true : false;
	});
}

//////////////////////////////////
/*
CLightBase::BufferUpdateInfo::BufferUpdateInfo(const Vulkan::SwapBufferObject *swapBuffer,const std::function<void(const Vulkan::Context&,const Vulkan::Buffer&)> &f)
	: m_fUpdate(f),m_swapBuffer(const_cast<Vulkan::SwapBufferObject*>(swapBuffer))
{
	auto &context = c_engine->GetRenderContext();
	auto numBuffers = context.GetSwapBufferRequirementCount();
	m_swapchainUpdateFlags = (1<<numBuffers) -1;
}

bool CLightBase::BufferUpdateInfo::ExecSwapchainUpdate()
{
	auto &context = c_engine->GetRenderContext();
	auto swapchainId = context.GetFrameSwapIndex();
	auto &buf = *m_swapBuffer->GetBuffer(swapchainId);
	auto swapFlag = 1<<swapchainId;
	m_swapchainUpdateFlags &= ~swapFlag;
	m_fUpdate(context,buf);
	return IsComplete();
}
bool CLightBase::BufferUpdateInfo::IsComplete() const {return (m_swapchainUpdateFlags == 0) ? true : false;}
*/ // prosper TODO
//////////////////////////////////

decltype(CLightBase::s_lightCount) CLightBase::s_lightCount = 0;
decltype(CLightBase::s_instanceBuffer) CLightBase::s_instanceBuffer = nullptr;
decltype(CLightBase::s_shadowBuffer) CLightBase::s_shadowBuffer = nullptr;
decltype(CLightBase::s_bufferLights) CLightBase::s_bufferLights = {};
decltype(CLightBase::s_shadowBufferLights) CLightBase::s_shadowBufferLights = {};
decltype(CLightBase::s_maxLightCount) CLightBase::s_maxLightCount = 0;
decltype(CLightBase::s_maxShadowCount) CLightBase::s_maxShadowCount = 0;

const prosper::UniformResizableBuffer &CLightBase::GetGlobalRenderBuffer() {return *s_instanceBuffer;}
const prosper::UniformResizableBuffer &CLightBase::GetGlobalShadowBuffer() {return *s_shadowBuffer;}
uint32_t CLightBase::GetMaxLightCount() {return s_maxLightCount;}
uint32_t CLightBase::GetMaxShadowCount() {return s_maxShadowCount;}
uint32_t CLightBase::GetLightCount() {return s_lightCount;}
void CLightBase::InitializeBuffers()
{
	if(s_instanceBuffer != nullptr)
		return;
	auto &limits = c_engine->GetDevice().get_physical_device_properties().core_vk1_0_properties_ptr->limits;

	auto lightDataSize = sizeof(BufferData);
	auto numLights = static_cast<uint32_t>(umath::min(static_cast<uint64_t>(limits.max_storage_buffer_range /lightDataSize),static_cast<uint64_t>(GameLimits::MaxAbsoluteLights)));
	s_maxLightCount = numLights;

	auto shadowDataSize = sizeof(ShadowBufferData);
	auto numShadows = static_cast<uint32_t>(umath::min(static_cast<uint64_t>(limits.max_storage_buffer_range /shadowDataSize),static_cast<uint64_t>(GameLimits::MaxAbsoluteShadowLights)));
	s_maxShadowCount = numShadows;

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.size = s_maxLightCount *lightDataSize;
	s_instanceBuffer = prosper::util::create_uniform_resizable_buffer(*c_engine,createInfo,lightDataSize,createInfo.size);

	createInfo.size = s_maxShadowCount *shadowDataSize;
	s_shadowBuffer = prosper::util::create_uniform_resizable_buffer(*c_engine,createInfo,shadowDataSize,createInfo.size);

	s_bufferLights.resize(s_maxLightCount,nullptr);
	s_shadowBufferLights.resize(s_maxShadowCount,nullptr);
}
CLightBase *CLightBase::GetLightByBufferIndex(uint32_t idx) {return (idx < s_bufferLights.size()) ? s_bufferLights.at(idx) : nullptr;}
CLightBase *CLightBase::GetLightByShadowBufferIndex(uint32_t idx) {return (idx < s_shadowBufferLights.size()) ? s_shadowBufferLights.at(idx) : nullptr;}
void CLightBase::ClearBuffers() {s_instanceBuffer = nullptr; s_shadowBuffer = nullptr;}

CLightBase::CLightBase(LightType type,const Vector3 &pos,const Color &col,ShadowType shadowType)
	: m_bTurnedOn(false),m_bStaticUpdateRequired(true),
	m_handle(new PtrCLightObject(this)),
	m_bFullUpdateRequired(true),
	m_cbShouldRender(nullptr),m_shadowTypes(shadowType),
	m_nextDynamicUpdate(0),m_bUseDualTextureSet(true)
{
	reinterpret_cast<Vector3&>(m_bufferData.position) = pos;
	switch(type)
	{
		case LightType::Spot:
			m_bufferData.flags |= Flags::TypeSpot;
			break;
		case LightType::Point:
			m_bufferData.flags |= Flags::TypePoint;
			break;
		case LightType::Directional:
			m_bufferData.flags |= Flags::TypeDirectional;
			break;
	}
	if(m_bufferData.direction.x == 0.f && m_bufferData.direction.y == 0.f && m_bufferData.direction.z == 0.f)
		m_bufferData.direction.z = 1.f;
	m_color.AddCallback([this](std::reference_wrapper<const Color> oldColor,std::reference_wrapper<const Color> newColor) {
		m_bufferData.color = newColor.get().ToVector4();
		if(m_renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,color),m_bufferData.color);
	});
	m_color = col;

	m_innerCutoffAngle.AddCallback([this](std::reference_wrapper<const float> oldAng,std::reference_wrapper<const float> newAng) {
		m_bufferData.cutoffInner = static_cast<float>(umath::cos(umath::deg_to_rad(newAng.get())));
		if(m_renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,cutoffInner),m_bufferData.cutoffInner);
	});
	m_outerCutoffAngle.AddCallback([this](std::reference_wrapper<const float> oldAng,std::reference_wrapper<const float> newAng) {
		m_bufferData.cutoffOuter = static_cast<float>(umath::cos(umath::deg_to_rad(newAng.get())));
		if(m_renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,cutoffOuter),m_bufferData.cutoffOuter);
	});

	++s_lightCount;
	m_onEntitySpawned = c_game->AddCallback("OnEntitySpawned",FunctionCallback<void,BaseEntity*>::Create([this](BaseEntity *ent) {
		UpdateEntity(static_cast<CBaseEntity*>(ent));
	}));
	m_onModelChanged = c_game->AddCallback("UpdateEntityModel",FunctionCallback<void,CBaseEntity*>::Create([this](CBaseEntity *ent) {
		for(auto &info : m_shadowInfoSets)
		{
			auto it = std::find_if(info.meshInfo.begin(),info.meshInfo.end(),[this,ent](std::shared_ptr<EntityInfo> &info) {
				return (info->hEntity.IsValid() && info->hEntity.get() == ent) ? true : false;
			});
			if(it != info.meshInfo.end())
				info.meshInfo.erase(it);
		}
		UpdateEntity(ent);
	}));
}

CLightBase::~CLightBase()
{
	m_handle.Invalidate();
	m_onModelChanged.Remove();
	m_onEntitySpawned.Remove();
	--s_lightCount;
	DestroyRenderBuffer();
	DestroyShadowBuffer();
}

void CLightBase::InitializeRenderBuffer()
{
	if(m_renderBuffer != nullptr)
		return;
	m_renderBuffer = s_instanceBuffer->AllocateBuffer(&m_bufferData);
	s_bufferLights.at(m_renderBuffer->GetBaseIndex()) = this;
}

void CLightBase::InitializeShadowBuffer()
{
	if(m_shadowBuffer != nullptr)
		return;
	m_shadowBuffer = s_shadowBuffer->AllocateBuffer(m_shadowBufferData.get());
	s_shadowBufferLights.at(m_shadowBuffer->GetBaseIndex()) = this;
}

void CLightBase::DestroyRenderBuffer()
{
	//m_bufferUpdateInfo.clear(); // prosper TODO
	if(m_renderBuffer == nullptr)
		return;
	s_bufferLights.at(m_renderBuffer->GetBaseIndex()) = nullptr;
	const auto flags = Flags::None;
	c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,flags),flags);
	m_renderBuffer = nullptr;
}

void CLightBase::DestroyShadowBuffer()
{
	if(m_shadowBuffer == nullptr)
		return;
	s_shadowBufferLights.at(m_shadowBuffer->GetBaseIndex()) = nullptr;
	m_shadowBuffer = nullptr;
}
/*
const Vulkan::Buffer &CLightBase::GetRenderBuffer() const
{
	if(m_renderBuffer == nullptr)
	{
		static Vulkan::Buffer n(nullptr);
		return n;
	}
	auto &context = c_engine->GetRenderContext();
	return *m_renderBuffer->GetBuffer(context.GetFrameSwapIndex());
}
const Vulkan::SwapBuffer &CLightBase::GetRenderSwapBuffer() const {return m_renderBuffer;}

const Vulkan::Buffer &CLightBase::GetShadowBuffer() const
{
	if(m_shadowBuffer == nullptr)
	{
		static Vulkan::Buffer n(nullptr);
		return n;
	}
	auto &context = c_engine->GetRenderContext();
	return *m_shadowBuffer->GetBuffer(context.GetFrameSwapIndex());
}
const Vulkan::SwapBuffer &CLightBase::GetShadowSwapBuffer() const {return m_shadowBuffer;}

void CLightBase::ScheduleBufferUpdate(DataSlot offsetId,const std::function<void(const Vulkan::Context&,const Vulkan::Buffer&)> &f)
{
	auto &buf = (offsetId >= DataSlot::ShadowStart) ? m_shadowBuffer : m_renderBuffer;
	if(buf == nullptr)
		return;
	auto offset = umath::to_integral(offsetId);
	if(offset >= m_bufferUpdateInfo.size())
		m_bufferUpdateInfo.resize(offset +1);
	auto &updateInfo = m_bufferUpdateInfo.at(offset) = std::unique_ptr<BufferUpdateInfo>(new BufferUpdateInfo(buf,f));
	if(updateInfo->ExecSwapchainUpdate() == true) // Update buffer for current swapchain index immediately
		m_bufferUpdateInfo.erase(m_bufferUpdateInfo.begin() +offset);
}
*/ // prosper TODO
void CLightBase::ExecSwapchainUpdate()
{
	/*for(auto it=m_bufferUpdateInfo.begin();it!=m_bufferUpdateInfo.end();)
	{
		auto &updateInfo = *it;
		if(updateInfo != nullptr)
		{
			if(m_renderBuffer == nullptr || updateInfo->ExecSwapchainUpdate() == true)
			{
				it = m_bufferUpdateInfo.erase(it);
				continue;
			}
		}
		++it;
	}*/ // prosper TODO
}

void CLightBase::UpdateViewMatrices() {}

bool CLightBase::ShouldRender() {return true;}

bool CLightBase::ShouldPass(const Model &mdl,const CModelSubMesh &mesh)
{
	auto &materials = mdl.GetMaterials();
	auto texId = mesh.GetTexture();
	if(texId >= materials.size() || !materials[texId].IsValid()) // Ignore meshes with invalid materials
		return false;
	auto &mat = materials[texId];
	auto *info = mat.get()->GetShaderInfo();
	if(info == nullptr || const_cast<ShaderInfo*>(info)->GetShader() == nullptr) // Ignore meshes with nodraw (Or invalid) shaders
		return false;
	return true;
}

static auto cvShadowQuality = GetClientConVar("cl_render_shadow_quality");
void CLightBase::UpdateMeshes(CBaseEntity *ent,std::vector<MeshInfo> &meshes)
{
	auto mdlComponent = ent->GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	auto pRenderComponent = ent->GetRenderComponent();
	if(mdl == nullptr || pRenderComponent.expired())
		return;
	if(pRenderComponent->GetLastRenderFrame() != c_engine->GetLastFrameId())
		pRenderComponent->UpdateRenderData();
	meshes.clear();

	uint32_t lod = 0;
	auto quality = cvShadowQuality->GetInt();
	switch(quality)
	{
		case 1:
			lod = 10'000;
			break;
		case 2:
			lod = (ent->IsWorld() == true) ? (WORLD_BASE_LOD +WORLD_LOD_OFFSET *1) : 2;
			break;
		case 3:
			lod = (ent->IsWorld() == true) ? WORLD_BASE_LOD : 1;
			break;
		default:
			break;
	};
	auto meshIds = mdl->GetBaseMeshes();
	mdl->TranslateLODMeshes(lod,meshIds);

	std::vector<std::shared_ptr<ModelMesh>> lodMeshes;
	mdl->GetMeshes(meshIds,lodMeshes);
	meshes.reserve(lodMeshes.size());
	for(auto it=lodMeshes.begin();it!=lodMeshes.end();++it)
	{
		auto *mesh = static_cast<CModelMesh*>(it->get());
		uint32_t meshRenderFlags = 0;
		if(ShouldPass(*ent,*mesh,meshRenderFlags) == true)
		{
			auto &subMeshes = mesh->GetSubMeshes();
			meshes.reserve(meshes.size() +subMeshes.size());
			for(auto it=subMeshes.begin();it!=subMeshes.end();++it)
			{
				auto *subMesh = it->get();
				if(ShouldPass(*mdl,*static_cast<CModelSubMesh*>(subMesh)) == false)
					continue;
				meshes.push_back(MeshInfo{static_cast<CModelSubMesh*>(it->get()),meshRenderFlags});
			}
		}
	}
}

void CLightBase::UpdateCulledMeshes()
{
	UpdateAllParticleSystems(); // TODO: Don't update all particles every frame
	if(m_bFullUpdateRequired == true)
		UpdateAllEntities();
	else
	{
		auto &pos = GetPosition();
		for(auto i=decltype(m_shadowInfoSets.size()){0};i<m_shadowInfoSets.size();++i)
		{
			auto &info = m_shadowInfoSets[i];
			auto rp = static_cast<CLightBase::RenderPass>(i);
			auto sz = info.meshInfo.size();
			for(auto i=0;i<sz;)
			{
				auto &entInfo = info.meshInfo[i];
				if(!entInfo->hEntity.IsValid())
				{
					info.meshInfo.erase(info.meshInfo.begin() +i);
					sz--;
				}
				else
				{
					auto *ent = static_cast<CBaseEntity*>(entInfo->hEntity.get());
					auto pRenderComponent = ent->GetRenderComponent();
					if(pRenderComponent.expired() || pRenderComponent->ShouldDrawShadow(pos) == false)
					{
						info.meshInfo.erase(info.meshInfo.begin() +i);
						sz--;
					}
					else
					{
						auto pTrComponent = ent->GetTransformComponent();
						auto t = pTrComponent.valid() ? pTrComponent->GetLastMoveTime() : 0.0;
						if(t > entInfo->tLastMoved)
						{
							ScheduleRender();
							UpdateMeshes(ent,entInfo->meshes);
							entInfo->tLastMoved = t;
							if(rp == CLightBase::RenderPass::Static)
								m_bStaticUpdateRequired = true;
						}
						else
						{
							auto pAnimComponent = ent->GetAnimatedComponent();
							if(pAnimComponent.valid() || ent->HasStateFlag(BaseEntity::StateFlags::RotationChanged))
								ScheduleRender();
						}
						i++;
					}
				}
			}
		}

		EntityIterator entIt {*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
		for(auto *ent : entIt)
		{
			//if(ent != nullptr)// && ent->HasChangeFlag(FENTITY_CHANGED_POSITION))
			auto pRenderComponent = static_cast<CBaseEntity*>(ent)->GetRenderComponent();
			if(pRenderComponent->ShouldDrawShadow(pos) == false)
				continue;
			auto bUpdate = ent->IsDynamic();
			if(bUpdate == false)
			{
				auto &set = m_shadowInfoSets[umath::to_integral(CLightBase::RenderPass::Static)];
				auto it = set.FindEntity(static_cast<CBaseEntity*>(ent));
				auto pTrComponent = ent->GetTransformComponent();
				auto t = pTrComponent.valid() ? pTrComponent->GetLastMoveTime() : 0.0;
				bUpdate = (it == set.meshInfo.end() || t > (*it)->tLastMoved) ? true : false;
				//if(bUpdate == true)
				//{
				//	Con::cwar<<"Static Position Change: "<<ent->GetClass()<<Con::endl;
				//	if(it != set.meshInfo.end())
				//		(*it)->tLastMoved = t;
				//}
			}
			if(bUpdate == true)
				UpdateEntity(static_cast<CBaseEntity*>(ent));
		}
	}
}

void CLightBase::UpdateParticleSystem(pragma::CParticleSystemComponent &pt)
{
	if(pt.IsActive() == false || pt.GetCastShadows() == false)
		return;
	m_particleInfo.push_back(std::make_unique<ParticleInfo>(pt));
}

void CLightBase::UpdateAllParticleSystems()
{
	m_particleInfo.clear();
	EntityIterator itParticles {*c_game};
	itParticles.AttachFilter<TEntityIteratorFilterComponent<pragma::CParticleSystemComponent>>();
	for(auto *ent : itParticles)
	{
		auto pt = ent->GetComponent<pragma::CParticleSystemComponent>();
		if(pt.expired())
			continue;
		UpdateParticleSystem(*pt);
	}
}

void CLightBase::UpdateAllEntities()
{
	m_bFullUpdateRequired = false;
	for(auto &info : m_shadowInfoSets)
		info.meshInfo.clear();
	std::vector<BaseEntity*> *ents;
	c_game->GetEntities(&ents);
	for(auto it=ents->begin();it!=ents->end();++it)
	{
		auto *ent = static_cast<CBaseEntity*>(*it);
		if(ent != nullptr)
			UpdateEntity(ent);
	}
}

void CLightBase::SetShouldPassCallback(const std::function<bool(const CBaseEntity&)> &callback)
{
	m_cbShouldRender = callback;
}

bool CLightBase::ShouldPass(const CBaseEntity &ent,uint32_t&) {return (m_cbShouldRender != nullptr) ? m_cbShouldRender(ent) : true;}
bool CLightBase::ShouldPass(const CBaseEntity&,const CModelMesh&,uint32_t&) {return true;}

void CLightBase::UpdateEntity(CBaseEntity *ent)
{
	auto mdlComponent = ent->GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	std::array<bool,2> visible = {false,false};
	auto &pos = GetPosition();
	auto pTrComponent = ent->GetRenderComponent();
	if(pTrComponent.valid() && pTrComponent->ShouldDrawShadow(pos))
	{
		if(mdl != nullptr)
		{
			uint32_t renderFlags = 0;
			if(ShouldPass(*ent,renderFlags) == true)
			{
				ScheduleRender();
				auto bDynamic = (m_bUseDualTextureSet == true && ent->IsDynamic()) ? true : false;
				auto rp = (bDynamic == true) ? RenderPass::Dynamic : RenderPass::Static;
				auto &info = m_shadowInfoSets[umath::to_integral(rp)];
				visible[umath::to_integral(rp)] = true;
				auto it = info.FindEntity(ent);
				if(it == info.meshInfo.end())
				{
					info.meshInfo.push_back(std::make_unique<EntityInfo>(this,ent,renderFlags));
					it = info.meshInfo.end() -1;
					UpdateMeshes(ent,(*it)->meshes);
				}
				else
				{
					(*it)->renderFlags = renderFlags;
					auto pTrComponent = ent->GetTransformComponent();
					(*it)->tLastMoved = pTrComponent.valid() ? pTrComponent->GetLastMoveTime() : 0.0;
				}
				if(bDynamic == false)
					m_bStaticUpdateRequired = true;
			}
		}
	}
	for(auto i=decltype(visible.size()){0};i<visible.size();++i)
	{
		if(visible[i] == true)
			continue;
		auto rp = static_cast<RenderPass>(i);
		auto &info = m_shadowInfoSets[i];
		auto it = info.FindEntity(ent);
		if(it != info.meshInfo.end())
		{
			info.meshInfo.erase(it);
			if(visible[umath::to_integral(RenderPass::Dynamic)] == false)
				m_bStaticUpdateRequired = true;
		}
	}
}

void CLightBase::SetStaticResolved(bool b)
{
	m_bStaticUpdateRequired = !b;
	m_nextDynamicUpdate = 0;
}

static CVar cvUpdateFrequency = GetClientConVar("cl_render_shadow_update_frequency");
void CLightBase::PostRenderShadow()
{
	m_nextDynamicUpdate = c_engine->GetLastFrameId() +cvUpdateFrequency->GetInt() +1; // +1 because this won't start having an effect until the NEXT frame
}

bool CLightBase::ShouldUpdateRenderPass(RenderPass rp) const
{
	if(rp == RenderPass::Static)
		return m_bStaticUpdateRequired;
	return (c_engine->GetLastFrameId() >= m_nextDynamicUpdate) ? true : false;
}

std::vector<std::shared_ptr<CLightBase::EntityInfo>> &CLightBase::GetCulledMeshes(RenderPass rp) {return m_shadowInfoSets[umath::to_integral(rp)].meshInfo;}
std::vector<std::shared_ptr<CLightBase::ParticleInfo>> &CLightBase::GetCulledParticleSystems() {return m_particleInfo;}

void CLightBase::TurnOn()
{
	auto b = IsTurnedOn();
	m_bTurnedOn = true;
	if(b == true)
		return;
	m_bufferData.flags |= Flags::TurnedOn;
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,flags),m_bufferData.flags);
	else
		InitializeRenderBuffer();
	// TODO: This will update all light and shadow buffers for this light source.
	// This shouldn't be necessary, but without light sources seem to have incorrect buffer
	// data when turned on. Once the cause for this has been found and dealt with, this
	// line can be removed!
	UpdateBuffers();
}
void CLightBase::UpdateBuffers()
{
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,0ull,m_bufferData);
	if(m_shadowBuffer != nullptr && m_shadowBufferData != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_shadowBuffer,0ull,m_shadowBufferData);
}
void CLightBase::TurnOff()
{
	auto b = IsTurnedOn();
	m_bTurnedOn = false;
	if(b == false)
		return;
	m_bufferData.flags &= ~Flags::TurnedOn;
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,flags),m_bufferData.flags);
	m_tTurnedOff = c_game->RealTime();
}
void CLightBase::Toggle()
{
	if(IsTurnedOn() == true)
		TurnOff();
	else
		TurnOn();
}
bool CLightBase::IsTurnedOn() {return (m_bufferData.flags &Flags::TurnedOn) != Flags::None;}
void CLightBase::UpdateShadowTypes()
{
	auto b = ShouldCastShadows();
	auto shadowIndex = 0u;
	if(b == true)
	{
		InitializeShadowBuffer();
		if(m_shadowBuffer != nullptr)
		{
			shadowIndex = m_shadowBuffer->GetBaseIndex() +1u;
			if(m_bufferData.shadowIndex == shadowIndex)
				return;
		}
	}
	else
	{
		DestroyShadowBuffer();
		if(m_bufferData.shadowIndex == 0)
			return;
	}
	m_bufferData.shadowIndex = shadowIndex;
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,shadowIndex),m_bufferData.shadowIndex);
}
bool CLightBase::ShouldCastShadows() const {return (GetShadowType() != ShadowType::None) ? true : false;}
void CLightBase::SetShadowType(ShadowType type)
{
	if(type == m_shadowTypes)
		return;
	m_shadowTypes = type;
	if(type != ShadowType::None)
		InitializeShadowMap();
	else if(m_shadow != nullptr)
		m_shadow = nullptr;
	UpdateShadowTypes(); // Has to be called AFTER the shadowmap has been initialized!
}
ShadowType CLightBase::GetShadowType() const {return (m_shadow == nullptr || !m_shadow->IsValid()) ? ShadowType::None : m_shadowTypes;}

void CLightBase::SetShadowMapIndex(uint32_t idx)
{
	idx = (idx == std::numeric_limits<uint32_t>::max()) ? 0u : (idx +1);
	if(idx == m_bufferData.shadowMapIndex)
		return;
	m_bufferData.shadowMapIndex = idx;
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,shadowMapIndex),m_bufferData.shadowMapIndex);
}

void CLightBase::InitializeShadowMap(ShadowMap &sm)
{
	sm.Initialize();
	sm.SetTextureReloadCallback([this]() {
		UpdateShadowTypes();
		ClearCache();
	});
	UpdateShadowTypes();
}

void CLightBase::InitializeShadowMap()
{
	if(m_shadow != nullptr || m_shadowTypes == ShadowType::None)
		return;
	m_shadow = std::make_unique<ShadowMap>();
	InitializeShadowMap(*m_shadow);
}

void CLightBase::Initialize() {InitializeShadowMap();}
void CLightBase::Think()
{
	auto frameId = c_engine->GetLastFrameId();
	if(m_lastThink == frameId)
		return;
	m_lastThink = frameId;

	if(m_renderBuffer != nullptr && IsTurnedOn() == false && c_game->RealTime() -m_tTurnedOff > 30.0)
		DestroyRenderBuffer(); // Free buffer if light hasn't been on in 30 seconds

	ExecSwapchainUpdate();
}

uint64_t CLightBase::GetLastTimeShadowRendered() const {return m_lastShadowRendered;}
void CLightBase::SetLastTimeShadowRendered(uint64_t t) {m_lastShadowRendered = t;}

CLightObjectHandle CLightBase::GetHandle() {return m_handle;}
CLightObjectHandle *CLightBase::CreateHandle() {return m_handle.Copy();}

LightType CLightBase::GetType() const {return LightType::Invalid;}
ShadowMap *CLightBase::GetShadowMap() {return m_shadow.get();}

void CLightBase::SetPosition(const Vector3 &pos)
{
	if(uvec::cmp(pos,reinterpret_cast<Vector3&>(m_bufferData.position)) == true)
		return;
	reinterpret_cast<Vector3&>(m_bufferData.position) = pos;
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,position),m_bufferData.position);
	m_bFullUpdateRequired = true;
	SetStaticResolved(false);
}
void CLightBase::ClearCache()
{
	m_bFullUpdateRequired = true;
	SetStaticResolved(false);
	for(auto &info : m_shadowInfoSets)
		info.meshInfo.clear();
}
void CLightBase::SetColor(const Color &color)
{
	if(color == m_color)
		return;
	m_color = color;
	if(color.a == 0 || (color.r == 0 && color.g == 0 && color.b == 0))
		m_bufferData.flags &= ~Flags::TurnedOn;
	else if(m_bTurnedOn == true)
		m_bufferData.flags |= Flags::TurnedOn;
}

void CLightBase::Remove()
{
	CGame *game = client->GetGameState();
	game->RemoveLight(this);
}

const Vector3 &CLightBase::GetPosition() const {return reinterpret_cast<const Vector3&>(m_bufferData.position);}
Color &CLightBase::GetColor() {return m_color;}

void CLightBase::SetDirection(const Vector3 &dir)
{
	if(uvec::cmp(dir,reinterpret_cast<Vector3&>(m_bufferData.direction)) == true)
		return;
	reinterpret_cast<Vector3&>(m_bufferData.direction) = dir;
	if(m_bufferData.direction.x == 0.f && m_bufferData.direction.y == 0.f && m_bufferData.direction.z == 0.f)
		m_bufferData.direction.z = 1.f;
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,direction),m_bufferData.direction);
	ScheduleRender();
}
const Vector3 &CLightBase::GetDirection() const {return reinterpret_cast<const Vector3&>(m_bufferData.direction);}
void CLightBase::SetDistance(float dist)
{
	if(dist == m_bufferData.position.w)
		return;
	m_bufferData.position.w = dist;
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(BufferData,position) +offsetof(Vector4,w),m_bufferData.position.w);
	ScheduleRender();
}
float CLightBase::GetDistance() const {return m_bufferData.position.w;}
void CLightBase::SetOuterCutoffAngle(float ang)
{
	if(ang == m_bufferData.cutoffOuter)
		return;
	m_outerCutoffAngle = ang;
}
float CLightBase::GetOuterCutoffAngle() const {return m_outerCutoffAngle;}
void CLightBase::SetInnerCutoffAngle(float ang)
{
	if(ang == m_bufferData.cutoffInner)
		return;
	m_innerCutoffAngle = ang;
}
float CLightBase::GetInnerCutoffAngle() const {return m_innerCutoffAngle;}
const std::shared_ptr<prosper::Buffer> &CLightBase::GetRenderBuffer() const {return m_renderBuffer;}
const std::shared_ptr<prosper::Buffer> &CLightBase::GetShadowBuffer() const {return m_shadowBuffer;}

////////////////////////////////

CLight::CLight(LightType type,const Vector3 &pos,const Color &col,ShadowType shadowType)
	: CLightBase(type,pos,col,shadowType)
{}

Mat4 &CLight::GetTransformationMatrix(unsigned int j) {return MVPBias<1>::GetTransformationMatrix(j);}

void CLight::Initialize()
{
	auto &biasMatrix = GetBiasTransformationMatrix();
	if(m_shadowBufferData != nullptr)
	{
		m_shadowBufferData->view = GetViewMatrix();
		m_shadowBufferData->projection = GetProjectionMatrix();
		m_shadowBufferData->depthVP = biasMatrix;
	}
	if(m_shadowBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_shadowBuffer,offsetof(ShadowBufferData,depthVP),biasMatrix); // Bias Transformation Matrix
	CLightBase::Initialize();
}

void CLight::UpdateTransformationMatrix(unsigned int j)
{
	MVPBias<1>::UpdateTransformationMatrix(j);
	auto &biasMatrix = GetBiasTransformationMatrix();
	if(m_shadowBufferData != nullptr)
	{
		m_shadowBufferData->view = GetViewMatrix();
		m_shadowBufferData->projection = GetProjectionMatrix();
		m_shadowBufferData->depthVP = biasMatrix;
	}
	if(m_shadowBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_shadowBuffer,offsetof(ShadowBufferData,depthVP),biasMatrix);
}

///////////////////

void Console::commands::debug_light_sources(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	auto &lights = c_game->GetLights();
	auto numLights = 0u;
	auto numTurnedOn = 0u;
	std::vector<size_t> discrepancies {};
	for(auto *l : lights)
	{
		if(l == nullptr)
			continue;
		++numLights;
		if(l->IsTurnedOn() == true)
			++numTurnedOn;
	}

	auto lightId = 0u;
	for(auto *l : lights)
	{
		if(l == nullptr)
			continue;
		Con::cout<<"Light #"<<lightId<<":"<<Con::endl;
		Con::cout<<"\tType: ";
		switch(l->GetType())
		{
			case LightType::Directional:
				Con::cout<<"Directional";
				break;
			case LightType::Point:
				Con::cout<<"Point";
				break;
			case LightType::Spot:
				Con::cout<<"Spot";
				break;
			default:
				Con::cout<<"Unknown";
				break;
		}
		Con::cout<<Con::endl;

		auto &buf = l->GetRenderBuffer();
		if(buf == nullptr)
			Con::cout<<"\tBuffer: NULL"<<Con::endl;
		else
		{
			CLightBase::BufferData data;
			buf->Read(0ull,sizeof(data),&data);
			std::string type = "Unknown";
			if((data.flags &CLightBase::Flags::TypeSpot) != CLightBase::Flags::None)
				type = "Spot";
			else if((data.flags &CLightBase::Flags::TypePoint) != CLightBase::Flags::None)
				type = "Point";
			else if((data.flags &CLightBase::Flags::TypeDirectional) != CLightBase::Flags::None)
				type = "Directional";
			Con::cout<<"\t\tPosition: ("<<data.position.x<<","<<data.position.y<<","<<data.position.z<<")"<<Con::endl;
			Con::cout<<"\t\tShadow Index: "<<data.shadowIndex<<Con::endl;
			Con::cout<<"\t\tShadow Map Index: "<<data.shadowMapIndex<<Con::endl;
			Con::cout<<"\t\tType: "<<type<<Con::endl;
			Con::cout<<"\t\tColor: ("<<data.color.r<<","<<data.color.g<<","<<data.color.b<<","<<data.color.a<<")"<<Con::endl;
			Con::cout<<"\t\tDirection: ("<<data.direction.x<<","<<data.direction.y<<","<<data.direction.z<<")"<<Con::endl;
			Con::cout<<"\t\tDistance: "<<data.position.w<<Con::endl;
			Con::cout<<"\t\tOuter cutoff angle: "<<data.cutoffOuter<<Con::endl;
			Con::cout<<"\t\tInner cutoff angle: "<<data.cutoffInner<<Con::endl;
			Con::cout<<"\t\tAttenuation: "<<data.attenuation<<Con::endl;
			Con::cout<<"\t\tFlags: "<<umath::to_integral(data.flags)<<Con::endl;
			Con::cout<<"\t\tTurned On: "<<(((data.flags &CLightBase::Flags::TurnedOn) == CLightBase::Flags::TurnedOn) ? "Yes" : "No")<<Con::endl;
		}
		++lightId;
	}
	Con::cout<<"Number of lights: "<<numLights<<Con::endl;
	Con::cout<<"Turned on: "<<numTurnedOn<<Con::endl;
	if(discrepancies.empty() == false)
	{
		Con::cwar<<"Discrepancies found in "<<discrepancies.size()<<" lights:"<<Con::endl;
		for(auto idx : discrepancies)
			Con::cout<<"\t"<<idx<<Con::endl;
	}
}
#endif
