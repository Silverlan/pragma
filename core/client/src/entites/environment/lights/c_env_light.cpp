#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/rendering/lighting/c_light_data_buffer_manager.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_shaderinfo.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
CLightComponent::MeshInfo::MeshInfo(CModelSubMesh *_mesh,uint32_t _renderFlags)
	: mesh(_mesh),renderFlags(_renderFlags)
{}
CLightComponent::EntityInfo::EntityInfo(CLightComponent *light,BaseEntity *ent,uint32_t _renderFlags)
	: hEntity(ent->GetHandle()),tLastMoved(0.0),renderFlags(_renderFlags)
{
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent.valid())
		tLastMoved = pTrComponent->GetLastMoveTime();
	auto &mdlComponent = static_cast<CBaseEntity*>(ent)->GetRenderComponent()->GetModelComponent();
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
CLightComponent::EntityInfo::~EntityInfo()
{
	if(hCbMaterialsLoaded.IsValid())
		hCbMaterialsLoaded.Remove();
}

CLightComponent::ParticleInfo::ParticleInfo(pragma::CParticleSystemComponent &pt)
	: hParticle(pt.GetHandle<pragma::CParticleSystemComponent>())
{}

std::vector<std::shared_ptr<CLightComponent::EntityInfo>>::iterator CLightComponent::ShadowInfoSet::FindEntity(CBaseEntity *ent)
{
	return std::find_if(meshInfo.begin(),meshInfo.end(),[ent](std::shared_ptr<EntityInfo> &info) {
		return (info->hEntity.IsValid() && info->hEntity.get() == ent) ? true : false;
	});
}

//////////////////////////////////
/*
CLightComponent::BufferUpdateInfo::BufferUpdateInfo(const Vulkan::SwapBufferObject *swapBuffer,const std::function<void(const Vulkan::Context&,const Vulkan::Buffer&)> &f)
	: m_fUpdate(f),m_swapBuffer(const_cast<Vulkan::SwapBufferObject*>(swapBuffer))
{
	auto &context = c_engine->GetRenderContext();
	auto numBuffers = context.GetSwapBufferRequirementCount();
	m_swapchainUpdateFlags = (1<<numBuffers) -1;
}

bool CLightComponent::BufferUpdateInfo::ExecSwapchainUpdate()
{
	auto &context = c_engine->GetRenderContext();
	auto swapchainId = context.GetFrameSwapIndex();
	auto &buf = *m_swapBuffer->GetBuffer(swapchainId);
	auto swapFlag = 1<<swapchainId;
	m_swapchainUpdateFlags &= ~swapFlag;
	m_fUpdate(context,buf);
	return IsComplete();
}
bool CLightComponent::BufferUpdateInfo::IsComplete() const {return (m_swapchainUpdateFlags == 0) ? true : false;}
*/ // prosper TODO
//////////////////////////////////

decltype(CLightComponent::s_lightCount) CLightComponent::s_lightCount = 0u;
const prosper::UniformResizableBuffer &CLightComponent::GetGlobalRenderBuffer() {return pragma::LightDataBufferManager::GetInstance().GetGlobalRenderBuffer();}
const prosper::UniformResizableBuffer &CLightComponent::GetGlobalShadowBuffer() {return pragma::ShadowDataBufferManager::GetInstance().GetGlobalRenderBuffer();}
uint32_t CLightComponent::GetMaxLightCount() {return pragma::LightDataBufferManager::GetInstance().GetMaxCount();}
uint32_t CLightComponent::GetMaxShadowCount() {return pragma::ShadowDataBufferManager::GetInstance().GetMaxCount();}
uint32_t CLightComponent::GetLightCount() {return s_lightCount;}
ComponentEventId CLightComponent::EVENT_SHOULD_PASS_ENTITY = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_SHOULD_PASS_MESH = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_HANDLE_SHADOW_MAP = pragma::INVALID_COMPONENT_ID;
ComponentEventId CLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED = pragma::INVALID_COMPONENT_ID;
void CLightComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_SHOULD_PASS_ENTITY = componentManager.RegisterEvent("SHOULD_PASS_ENTITY",std::type_index(typeid(CLightComponent)));
	EVENT_SHOULD_PASS_ENTITY_MESH = componentManager.RegisterEvent("SHOULD_PASS_ENTITY_MESH",std::type_index(typeid(CLightComponent)));
	EVENT_SHOULD_PASS_MESH = componentManager.RegisterEvent("SHOULD_PASS_MESH",std::type_index(typeid(CLightComponent)));
	EVENT_SHOULD_UPDATE_RENDER_PASS = componentManager.RegisterEvent("SHOULD_UPDATE_RENDER_PASS",std::type_index(typeid(CLightComponent)));
	EVENT_GET_TRANSFORMATION_MATRIX = componentManager.RegisterEvent("GET_TRANSFORMATION_MATRIX",std::type_index(typeid(CLightComponent)));
	EVENT_HANDLE_SHADOW_MAP = componentManager.RegisterEvent("HANDLE_SHADOW_MAP");
	EVENT_ON_SHADOW_BUFFER_INITIALIZED = componentManager.RegisterEvent("ON_SHADOW_BUFFER_INITIALIZED");
}
void CLightComponent::InitializeBuffers()
{
	pragma::LightDataBufferManager::GetInstance().Initialize();
	pragma::ShadowDataBufferManager::GetInstance().Initialize();
}
CLightComponent *CLightComponent::GetLightByBufferIndex(uint32_t idx) {return LightDataBufferManager::GetInstance().GetLightByBufferIndex(idx);}
CLightComponent *CLightComponent::GetLightByShadowBufferIndex(uint32_t idx) {return ShadowDataBufferManager::GetInstance().GetLightByBufferIndex(idx);}
void CLightComponent::ClearBuffers()
{
	LightDataBufferManager::GetInstance().Reset();
	ShadowDataBufferManager::GetInstance().Reset();
}

CLightComponent::CLightComponent(BaseEntity &ent)
	: CBaseLightComponent(ent),m_stateFlags{StateFlags::StaticUpdateRequired | StateFlags::FullUpdateRequired | StateFlags::UseDualTextureSet | StateFlags::RenderScheduled | StateFlags::AddToGameScene}
{}
CLightComponent::~CLightComponent()
{
	m_onModelChanged.Remove();
	--s_lightCount;
	DestroyRenderBuffer();
	DestroyShadowBuffer();
}
luabind::object CLightComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CLightComponentHandleWrapper>(l);}
void CLightComponent::InitializeRenderBuffer()
{
	if(m_renderBuffer != nullptr)
		return;
	m_renderBuffer = LightDataBufferManager::GetInstance().Request(*this,m_bufferData);
}

void CLightComponent::InitializeShadowBuffer()
{
	if(m_shadowBuffer != nullptr)
		return;
	m_shadowBuffer = ShadowDataBufferManager::GetInstance().Request(*this,*m_shadowBufferData);
	BroadcastEvent(EVENT_ON_SHADOW_BUFFER_INITIALIZED,CEOnShadowBufferInitialized{*m_shadowBuffer});
}

void CLightComponent::DestroyRenderBuffer()
{
	//m_bufferUpdateInfo.clear(); // prosper TODO
	if(m_renderBuffer == nullptr)
		return;
	umath::set_flag(m_bufferData.flags,LightBufferData::BufferFlags::TurnedOn,false);
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,flags),m_bufferData.flags);

	LightDataBufferManager::GetInstance().Free(m_renderBuffer);
	m_renderBuffer = nullptr;
}

void CLightComponent::DestroyShadowBuffer()
{
	if(m_shadowBuffer == nullptr)
		return;
	ShadowDataBufferManager::GetInstance().Free(m_shadowBuffer);
	m_shadowBuffer = nullptr;
}
/*
const Vulkan::Buffer &CLightComponent::GetRenderBuffer() const
{
	if(m_renderBuffer == nullptr)
	{
		static Vulkan::Buffer n(nullptr);
		return n;
	}
	auto &context = c_engine->GetRenderContext();
	return *m_renderBuffer->GetBuffer(context.GetFrameSwapIndex());
}
const Vulkan::SwapBuffer &CLightComponent::GetRenderSwapBuffer() const {return m_renderBuffer;}

const Vulkan::Buffer &CLightComponent::GetShadowBuffer() const
{
	if(m_shadowBuffer == nullptr)
	{
		static Vulkan::Buffer n(nullptr);
		return n;
	}
	auto &context = c_engine->GetRenderContext();
	return *m_shadowBuffer->GetBuffer(context.GetFrameSwapIndex());
}
const Vulkan::SwapBuffer &CLightComponent::GetShadowSwapBuffer() const {return m_shadowBuffer;}

void CLightComponent::ScheduleBufferUpdate(DataSlot offsetId,const std::function<void(const Vulkan::Context&,const Vulkan::Buffer&)> &f)
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
void CLightComponent::ExecSwapchainUpdate()
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

bool CLightComponent::ShouldRender() {return true;}

bool CLightComponent::ShouldPass(const Model &mdl,const CModelSubMesh &mesh)
{
	auto &materials = mdl.GetMaterials();
	auto texId = mesh.GetTexture();
	if(texId >= materials.size() || !materials[texId].IsValid()) // Ignore meshes with invalid materials
		return false;
	auto &mat = materials[texId];
	auto *info = mat.get()->GetShaderInfo();
	if(info == nullptr || const_cast<util::ShaderInfo*>(info)->GetShader() == nullptr) // Ignore meshes with nodraw (Or invalid) shaders
		return false;
	CEShouldPassMesh evData {mdl,mesh};
	InvokeEventCallbacks(EVENT_SHOULD_PASS_MESH,evData);
	return evData.shouldPass;
}

static auto cvShadowQuality = GetClientConVar("cl_render_shadow_quality");
void CLightComponent::UpdateMeshes(CBaseEntity *ent,std::vector<MeshInfo> &meshes)
{
	auto pRenderComponent = ent->GetRenderComponent();
	if(pRenderComponent.expired())
		return;
	auto &mdlComponent = pRenderComponent->GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	if(pRenderComponent->GetLastRenderFrame() != c_engine->GetLastFrameId())
		pRenderComponent->UpdateRenderData(c_engine->GetDrawCommandBuffer());
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

void CLightComponent::UpdateCulledMeshes()
{
	UpdateAllParticleSystems(); // TODO: Don't update all particles every frame
	if((m_stateFlags &StateFlags::FullUpdateRequired) != StateFlags::None)
		UpdateAllEntities();
	else
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent.valid())
		{
			auto &pos = pTrComponent->GetPosition();
			for(auto i=decltype(m_shadowInfoSets.size()){0};i<m_shadowInfoSets.size();++i)
			{
				auto &info = m_shadowInfoSets[i];
				auto rp = static_cast<CLightComponent::RenderPass>(i);
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
								umath::set_flag(m_stateFlags,StateFlags::RenderScheduled,true);
								UpdateMeshes(ent,entInfo->meshes);
								entInfo->tLastMoved = t;
								if(rp == CLightComponent::RenderPass::Static)
									m_stateFlags |= StateFlags::StaticUpdateRequired;
							}
							else
							{
								auto pAnimComponent = ent->GetAnimatedComponent();
								if(pAnimComponent.valid() || ent->HasStateFlag(BaseEntity::StateFlags::RotationChanged))
									umath::set_flag(m_stateFlags,StateFlags::RenderScheduled,true);
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
					auto &set = m_shadowInfoSets[umath::to_integral(CLightComponent::RenderPass::Static)];
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
}

void CLightComponent::UpdateParticleSystem(pragma::CParticleSystemComponent &pt)
{
	if(pt.IsActive() == false || pt.GetCastShadows() == false)
		return;
	m_particleInfo.push_back(std::make_unique<ParticleInfo>(pt));
}

void CLightComponent::InitializeLight(BaseEntityComponent &component)
{
	auto &scene = c_game->GetScene();
	if(scene == nullptr)
		return;
	if(m_hLight.valid())
	{
		auto pLightComponent = m_hLight->GetEntity().GetComponent<CLightComponent>();
		if(pLightComponent.valid())
			scene->RemoveLight(pLightComponent.get());
	}
	CBaseLightComponent::InitializeLight(component);
	if(m_hLight.expired())
		return;
	if(umath::is_flag_set(m_stateFlags,StateFlags::AddToGameScene))
	{
		auto pLightComponent = m_hLight->GetEntity().GetComponent<CLightComponent>();
		if(pLightComponent.valid())
			scene->AddLight(pLightComponent.get());
	}
}

void CLightComponent::UpdateAllParticleSystems()
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

void CLightComponent::UpdateAllEntities()
{
	m_stateFlags &= ~StateFlags::FullUpdateRequired;
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

bool CLightComponent::ShouldPass(const CBaseEntity &ent,uint32_t &renderFlags)
{
	if(m_shadow == nullptr)
		return false;
	CEShouldPassEntity evData {ent,renderFlags};
	if(InvokeEventCallbacks(EVENT_SHOULD_PASS_ENTITY,evData) == util::EventReply::Handled)
		return evData.shouldPass;
	return true;
}
bool CLightComponent::ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags)
{
	if(m_shadow == nullptr)
		return false;
	CEShouldPassEntityMesh evData {ent,mesh,renderFlags};
	InvokeEventCallbacks(EVENT_SHOULD_PASS_ENTITY_MESH,evData);
	return evData.shouldPass;
}

bool CLightComponent::IsInCone(const CBaseEntity &ent,const Vector3 &dir,float angle) const
{
	auto pRenderComponent = ent.GetRenderComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentThis = GetEntity().GetTransformComponent();
	if(pRenderComponent.expired() || pTrComponent.expired() || pTrComponentThis.expired())
		return false;
	auto &start = pTrComponentThis->GetPosition();
	auto sphere = pRenderComponent->GetRenderSphereBounds();
	return Intersection::SphereCone(pTrComponent->GetPosition() +sphere.pos,sphere.radius,start,dir,angle);
}
bool CLightComponent::IsInRange(const CBaseEntity &ent) const
{
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto pRenderComponent = ent.GetRenderComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentThis = GetEntity().GetTransformComponent();
	if(pRadiusComponent.expired() || pRenderComponent.expired() || pTrComponent.expired() || pTrComponentThis.expired())
		return false;
	auto &origin = pTrComponentThis->GetPosition();
	auto sphere = pRenderComponent->GetRenderSphereBounds();
	auto &pos = pTrComponent->GetPosition();
	auto radius = pRadiusComponent->GetRadius();
	return (uvec::distance(pos +sphere.pos,origin) <= (radius +sphere.radius)) ? true : false;
}
bool CLightComponent::IsInRange(const CBaseEntity &ent,const CModelMesh &mesh) const
{
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentThis = GetEntity().GetTransformComponent();
	if(pRadiusComponent.expired() || pTrComponent.expired() || pTrComponentThis.expired())
		return false;
	auto &origin = pTrComponentThis->GetPosition();
	auto radius = pRadiusComponent->GetRadius();
	auto &pos = pTrComponent->GetPosition();
	Vector3 min;
	Vector3 max;
	mesh.GetBounds(min,max);
	min += pos;
	max += pos;
	return Intersection::AABBSphere(min,max,origin,radius);
}
/*
bool CLightRanged::ShouldPass(const CBaseEntity &ent)
{

}
bool CLightRanged::ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t&)
*/

void CLightComponent::UpdateEntity(CBaseEntity *ent)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.expired())
		return;
	std::array<bool,2> visible = {false,false};
	auto &pos = pTrComponent->GetPosition();
	auto pRenderComponent = ent->GetRenderComponent();
	if(pRenderComponent.valid() && pRenderComponent->ShouldDrawShadow(pos))
	{
		auto &mdlComponent = pRenderComponent->GetModelComponent();
		auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
		if(mdl != nullptr)
		{
			uint32_t renderFlags = 0;
			if(ShouldPass(*ent,renderFlags) == true)
			{
				umath::set_flag(m_stateFlags,StateFlags::RenderScheduled,true);
				auto bDynamic = ((m_stateFlags &StateFlags::UseDualTextureSet) != StateFlags::None && ent->IsDynamic()) ? true : false;
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
					m_stateFlags |= StateFlags::StaticUpdateRequired;
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
				m_stateFlags |= StateFlags::StaticUpdateRequired;
		}
	}
}

void CLightComponent::SetStaticResolved(bool b)
{
	umath::set_flag(m_stateFlags,StateFlags::StaticUpdateRequired,!b);
	m_nextDynamicUpdate = 0;
}

static CVar cvUpdateFrequency = GetClientConVar("cl_render_shadow_update_frequency");
void CLightComponent::PostRenderShadow()
{
	m_nextDynamicUpdate = c_engine->GetLastFrameId() +cvUpdateFrequency->GetInt() +1; // +1 because this won't start having an effect until the NEXT frame
}

bool CLightComponent::ShouldUpdateRenderPass(RenderPass rp) const
{
	CEShouldUpdateRenderPass evData {};
	if(InvokeEventCallbacks(EVENT_SHOULD_UPDATE_RENDER_PASS,evData) == util::EventReply::Handled)
		return evData.shouldUpdate;
	if(rp == RenderPass::Static)
		return umath::is_flag_set(m_stateFlags,StateFlags::StaticUpdateRequired);
	return (c_engine->GetLastFrameId() >= m_nextDynamicUpdate) ? true : false;
}

std::vector<std::shared_ptr<CLightComponent::EntityInfo>> &CLightComponent::GetCulledMeshes(RenderPass rp) {return m_shadowInfoSets[umath::to_integral(rp)].meshInfo;}
std::vector<std::shared_ptr<CLightComponent::ParticleInfo>> &CLightComponent::GetCulledParticleSystems() {return m_particleInfo;}

void CLightComponent::UpdateBuffers()
{
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,0ull,m_bufferData);
	if(m_shadowBuffer != nullptr && m_shadowBufferData != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_shadowBuffer,0ull,m_shadowBufferData);
}
void CLightComponent::UpdateShadowTypes()
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
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,shadowIndex),m_bufferData.shadowIndex);
}
bool CLightComponent::ShouldCastShadows() const {return (GetShadowType() != ShadowType::None) ? true : false;}
void CLightComponent::SetShadowType(ShadowType type)
{
	if(type == GetShadowType())
		return;
	CBaseLightComponent::SetShadowType(type);
	if(type != ShadowType::None)
		InitializeShadowMap();
	else if(m_shadow != nullptr)
		m_shadow = nullptr;
	UpdateShadowTypes(); // Has to be called AFTER the shadowmap has been initialized!
}

void CLightComponent::SetFalloffExponent(float falloffExponent)
{
	if(falloffExponent == m_bufferData.falloffExponent)
		return;
	BaseEnvLightComponent::SetFalloffExponent(falloffExponent);
	m_bufferData.falloffExponent = falloffExponent;
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,falloffExponent),m_bufferData.falloffExponent);
}

uint32_t CLightComponent::GetShadowMapIndex() const {return m_bufferData.shadowMapIndex;}

void CLightComponent::SetShadowMapIndex(uint32_t idx)
{
	idx = (idx == std::numeric_limits<uint32_t>::max()) ? 0u : (idx +1);
	if(idx == m_bufferData.shadowMapIndex)
		return;
	m_bufferData.shadowMapIndex = idx;
	if(m_renderBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,shadowMapIndex),m_bufferData.shadowMapIndex);
}

void CLightComponent::InitializeShadowMap(ShadowMap &sm)
{
	sm.Initialize();
	sm.SetTextureReloadCallback([this]() {
		UpdateShadowTypes();
		ClearCache();
	});
	UpdateShadowTypes();
}

void CLightComponent::InitializeShadowMap()
{
	if(m_shadow != nullptr || GetShadowType() == ShadowType::None)
		return;
	CEHandleShadowMap ceData {m_shadow};
	if(BroadcastEvent(EVENT_HANDLE_SHADOW_MAP,ceData) == util::EventReply::Unhandled)
		m_shadow = std::make_unique<ShadowMap>();
	InitializeShadowMap(*m_shadow);
}

void CLightComponent::SetStateFlag(StateFlags flag,bool enabled) {umath::set_flag(m_stateFlags,flag,enabled);}

void CLightComponent::Initialize()
{
	CBaseLightComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent<LogicComponent>();

	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_ON,[this](std::reference_wrapper<ComponentEvent> evData) {
		umath::set_flag(m_bufferData.flags,LightBufferData::BufferFlags::TurnedOn,true);
		if(m_renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,flags),m_bufferData.flags);
		else
			InitializeRenderBuffer();
		// TODO: This will update all light and shadow buffers for this light source.
		// This shouldn't be necessary, but without light sources seem to have incorrect buffer
		// data when turned on. Once the cause for this has been found and dealt with, this
		// line can be removed!
		UpdateBuffers();
	});
	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_OFF,[this](std::reference_wrapper<ComponentEvent> evData) {
		umath::set_flag(m_bufferData.flags,LightBufferData::BufferFlags::TurnedOn,false);
		if(m_renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,flags),m_bufferData.flags);
		m_tTurnedOff = c_game->RealTime();
	});
	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<ComponentEvent> evData) {
		auto frameId = c_engine->GetLastFrameId();
		if(m_lastThink == frameId)
			return;
		m_lastThink = frameId;

		if(m_renderBuffer != nullptr && c_game->RealTime() -m_tTurnedOff > 30.0)
		{
			auto pToggleComponent = GetEntity().GetComponent<CToggleComponent>();
			if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn() == false)
				DestroyRenderBuffer(); // Free buffer if light hasn't been on in 30 seconds
		}

		ExecSwapchainUpdate();
	});
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.valid())
		reinterpret_cast<Vector3&>(m_bufferData.position) = pTrComponent->GetPosition();
	if(m_bufferData.direction.x == 0.f && m_bufferData.direction.y == 0.f && m_bufferData.direction.z == 0.f)
		m_bufferData.direction.z = 1.f;

	++s_lightCount;
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
void CLightComponent::UpdateTransformationMatrix(const Mat4 &biasMatrix,const Mat4 &viewMatrix,const Mat4 &projectionMatrix)
{
	if(m_shadowBufferData != nullptr)
	{
		m_shadowBufferData->view = viewMatrix;
		m_shadowBufferData->projection = projectionMatrix;
		m_shadowBufferData->depthVP = biasMatrix;
	}
	if(m_shadowBuffer != nullptr)
		c_engine->ScheduleRecordUpdateBuffer(m_shadowBuffer,offsetof(ShadowBufferData,depthVP),biasMatrix);
}
void CLightComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	CBaseLightComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CTransformComponent))
	{
		FlagCallbackForRemoval(static_cast<CTransformComponent&>(component).GetPosProperty()->AddCallback([this](std::reference_wrapper<const Vector3> oldPos,std::reference_wrapper<const Vector3> pos) {
			if(uvec::cmp(pos.get(),reinterpret_cast<Vector3&>(m_bufferData.position)) == true)
				return;
			reinterpret_cast<Vector3&>(m_bufferData.position) = pos;
			if(m_renderBuffer != nullptr)
				c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,position),m_bufferData.position);
			m_stateFlags |= StateFlags::FullUpdateRequired;
			SetStaticResolved(false);
		}),CallbackType::Component,&component);
		FlagCallbackForRemoval(static_cast<CTransformComponent&>(component).GetOrientationProperty()->AddCallback([this](std::reference_wrapper<const Quat> oldRot,std::reference_wrapper<const Quat> rot) {
			auto dir = uquat::forward(rot);
			if(uvec::cmp(dir,reinterpret_cast<Vector3&>(m_bufferData.direction)) == true)
				return;
			reinterpret_cast<Vector3&>(m_bufferData.direction) = dir;
			if(m_bufferData.direction.x == 0.f && m_bufferData.direction.y == 0.f && m_bufferData.direction.z == 0.f)
				m_bufferData.direction.z = 1.f;
			if(m_renderBuffer != nullptr)
				c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,direction),m_bufferData.direction);
			umath::set_flag(m_stateFlags,StateFlags::RenderScheduled,true);
		}),CallbackType::Component,&component);
	}
	else if(typeid(component) == typeid(CRadiusComponent))
	{
		FlagCallbackForRemoval(static_cast<CRadiusComponent&>(component).GetRadiusProperty()->AddCallback([this](std::reference_wrapper<const float> oldRadius,std::reference_wrapper<const float> radius) {
			if(radius == m_bufferData.position.w)
				return;
			m_bufferData.position.w = radius;
			if(m_renderBuffer != nullptr)
				c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,position) +offsetof(Vector4,w),m_bufferData.position.w);
			umath::set_flag(m_stateFlags,StateFlags::RenderScheduled,true);
		}),CallbackType::Component,&component);
	}
	else if(typeid(component) == typeid(CColorComponent))
	{
		FlagCallbackForRemoval(static_cast<CColorComponent&>(component).GetColorProperty()->AddCallback([this](std::reference_wrapper<const Color> oldColor,std::reference_wrapper<const Color> color) {
			m_bufferData.color = color.get().ToVector4();
			if(m_renderBuffer != nullptr)
				c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,color),m_bufferData.color);

			if(color.get().a == 0 || (color.get().r == 0 && color.get().g == 0 && color.get().b == 0))
				umath::set_flag(m_bufferData.flags,LightBufferData::BufferFlags::TurnedOn,false);
			else
			{
				auto pToggleComponent = GetEntity().GetComponent<CToggleComponent>();
				if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn() == true)
					umath::set_flag(m_bufferData.flags,LightBufferData::BufferFlags::TurnedOn,true);
			}
		}),CallbackType::Component,&component);
	}
	else if(typeid(component) == typeid(CLightSpotComponent))
	{
		m_bufferData.flags &= ~(LightBufferData::BufferFlags::TypeSpot | LightBufferData::BufferFlags::TypePoint | LightBufferData::BufferFlags::TypeDirectional);
		m_bufferData.flags |= LightBufferData::BufferFlags::TypeSpot;
		if(m_renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,flags),m_bufferData.flags);
	}
	else if(typeid(component) == typeid(CLightPointComponent))
	{
		m_bufferData.flags &= ~(LightBufferData::BufferFlags::TypeSpot | LightBufferData::BufferFlags::TypePoint | LightBufferData::BufferFlags::TypeDirectional);
		m_bufferData.flags |= LightBufferData::BufferFlags::TypePoint;
		if(m_renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,flags),m_bufferData.flags);
	}
	else if(typeid(component) == typeid(CLightDirectionalComponent))
	{
		m_bufferData.flags &= ~(LightBufferData::BufferFlags::TypeSpot | LightBufferData::BufferFlags::TypePoint | LightBufferData::BufferFlags::TypeDirectional);
		m_bufferData.flags |= LightBufferData::BufferFlags::TypeDirectional;
		if(m_renderBuffer != nullptr)
			c_engine->ScheduleRecordUpdateBuffer(m_renderBuffer,offsetof(LightBufferData,flags),m_bufferData.flags);
	}
}
void CLightComponent::OnEntitySpawn()
{
	CBaseLightComponent::OnEntitySpawn();
	InitializeShadowMap();
	UpdateEntity(&static_cast<CBaseEntity&>(GetEntity()));
}

const pragma::LightBufferData &CLightComponent::GetBufferData() const {return const_cast<CLightComponent*>(this)->GetBufferData();}
pragma::LightBufferData &CLightComponent::GetBufferData() {return m_bufferData;}
const pragma::ShadowBufferData *CLightComponent::GetShadowBufferData() const {return const_cast<CLightComponent*>(this)->GetShadowBufferData();}
pragma::ShadowBufferData *CLightComponent::GetShadowBufferData() {return m_shadowBufferData.get();}

uint64_t CLightComponent::GetLastTimeShadowRendered() const {return m_lastShadowRendered;}
void CLightComponent::SetLastTimeShadowRendered(uint64_t t) {m_lastShadowRendered = t;}

ShadowMap *CLightComponent::GetShadowMap() {return m_shadow.get();}

void CLightComponent::ClearCache()
{
	m_stateFlags |= StateFlags::FullUpdateRequired;
	SetStaticResolved(false);
	for(auto &info : m_shadowInfoSets)
		info.meshInfo.clear();
}

Mat4 &CLightComponent::GetTransformationMatrix(unsigned int j)
{
	CEGetTransformationMatrix evData{j};
	InvokeEventCallbacks(EVENT_GET_TRANSFORMATION_MATRIX,evData);
	if(evData.transformation != nullptr)
		return *evData.transformation;
	static Mat4 m;
	m = umat::identity();
	return m;
}

const std::shared_ptr<prosper::Buffer> &CLightComponent::GetRenderBuffer() const {return m_renderBuffer;}
const std::shared_ptr<prosper::Buffer> &CLightComponent::GetShadowBuffer() const {return m_shadowBuffer;}
void CLightComponent::SetRenderBuffer(const std::shared_ptr<prosper::Buffer> &renderBuffer) {m_renderBuffer = renderBuffer;}
void CLightComponent::SetShadowBuffer(const std::shared_ptr<prosper::Buffer> &renderBuffer) {m_shadowBuffer = renderBuffer;}

///////////////////

void Console::commands::debug_light_sources(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightComponent>>();
	std::vector<pragma::CLightComponent*> lights;
	lights.reserve(entIt.GetCount());
	for(auto *ent : entIt)
		lights.push_back(ent->GetComponent<CLightComponent>().get());
	
	auto numLights = 0u;
	auto numTurnedOn = 0u;
	std::vector<size_t> discrepancies {};
	for(auto *l : lights)
	{
		++numLights;
		auto pToggleComponent = l->GetEntity().GetComponent<CToggleComponent>();
		if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn() == true)
			++numTurnedOn;
	}

	auto lightId = 0u;
	for(auto *l : lights)
	{
		Con::cout<<"Light #"<<lightId<<":"<<Con::endl;
		Con::cout<<"\tType: ";
		auto type = LightType::Invalid;
		auto *pLight = l->GetLight(type);
		switch(type)
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
			LightBufferData data;
			buf->Read(0ull,sizeof(data),&data);
			std::string type = "Unknown";
			if((data.flags &LightBufferData::BufferFlags::TypeSpot) != LightBufferData::BufferFlags::None)
				type = "Spot";
			else if((data.flags &LightBufferData::BufferFlags::TypePoint) != LightBufferData::BufferFlags::None)
				type = "Point";
			else if((data.flags &LightBufferData::BufferFlags::TypeDirectional) != LightBufferData::BufferFlags::None)
				type = "Directional";
			Con::cout<<"\t\tPosition: ("<<data.position.x<<","<<data.position.y<<","<<data.position.z<<")"<<Con::endl;
			Con::cout<<"\t\tShadow Index: "<<data.shadowIndex<<Con::endl;
			Con::cout<<"\t\tShadow Map Index: "<<data.shadowMapIndex<<Con::endl;
			Con::cout<<"\t\tType: "<<type<<Con::endl;
			Con::cout<<"\t\tColor: ("<<data.color.r<<","<<data.color.g<<","<<data.color.b<<","<<data.color.a<<")"<<Con::endl;
			Con::cout<<"\t\tDirection: ("<<data.direction.x<<","<<data.direction.y<<","<<data.direction.z<<")"<<Con::endl;
			Con::cout<<"\t\tCone Start Offset: "<<data.direction.w<<Con::endl;
			Con::cout<<"\t\tDistance: "<<data.position.w<<Con::endl;
			Con::cout<<"\t\tOuter cutoff angle: "<<data.cutoffOuter<<Con::endl;
			Con::cout<<"\t\tInner cutoff angle: "<<data.cutoffInner<<Con::endl;
			Con::cout<<"\t\tAttenuation: "<<data.attenuation<<Con::endl;
			Con::cout<<"\t\tFlags: "<<umath::to_integral(data.flags)<<Con::endl;
			Con::cout<<"\t\tTurned On: "<<(((data.flags &LightBufferData::BufferFlags::TurnedOn) == LightBufferData::BufferFlags::TurnedOn) ? "Yes" : "No")<<Con::endl;
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

/////////////////

CEShouldPassEntity::CEShouldPassEntity(const CBaseEntity &entity,uint32_t &renderFlags)
	: entity{entity},renderFlags{renderFlags}
{}
void CEShouldPassEntity::PushArguments(lua_State *l) {}

/////////////////

CEShouldPassMesh::CEShouldPassMesh(const Model &model,const CModelSubMesh &mesh)
	: model{model},mesh{mesh}
{}
void CEShouldPassMesh::PushArguments(lua_State *l) {}

/////////////////

CEShouldPassEntityMesh::CEShouldPassEntityMesh(const CBaseEntity &entity,const CModelMesh &mesh,uint32_t &renderFlags)
	: entity{entity},mesh{mesh},renderFlags{renderFlags}
{}
void CEShouldPassEntityMesh::PushArguments(lua_State *l) {}

/////////////////

CEShouldUpdateRenderPass::CEShouldUpdateRenderPass()
{}
void CEShouldUpdateRenderPass::PushArguments(lua_State *l) {}

/////////////////

CEGetTransformationMatrix::CEGetTransformationMatrix(uint32_t index)
	: index{index}
{}
void CEGetTransformationMatrix::PushArguments(lua_State *l) {}

/////////////////

CEHandleShadowMap::CEHandleShadowMap(std::unique_ptr<ShadowMap> &shadowMap)
	: shadowMap{shadowMap}
{}
void CEHandleShadowMap::PushArguments(lua_State *l) {}

/////////////////

CEOnShadowBufferInitialized::CEOnShadowBufferInitialized(prosper::Buffer &shadowBuffer)
	: shadowBuffer{shadowBuffer}
{}
void CEOnShadowBufferInitialized::PushArguments(lua_State *l)
{
	Lua::Push<std::shared_ptr<Lua::Vulkan::Buffer>>(l,shadowBuffer.shared_from_this());
}
#pragma optimize("",on)
