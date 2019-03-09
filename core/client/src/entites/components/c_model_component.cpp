#include "stdafx_client.h"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/c_modelmanager.h"
#include <pragma/entities/components/base_model_component_setmodel.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

ComponentEventId CModelComponent::EVENT_ON_UPDATE_LOD = INVALID_COMPONENT_ID;
ComponentEventId CModelComponent::EVENT_ON_UPDATE_LOD_BY_POS = INVALID_COMPONENT_ID;
luabind::object CModelComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CModelComponentHandleWrapper>(l);}
void CModelComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	BaseModelComponent::RegisterEvents(componentManager);
	EVENT_ON_UPDATE_LOD = componentManager.RegisterEvent("EVENT_ON_UPDATE_LOD",std::type_index(typeid(CModelComponent)));
	EVENT_ON_UPDATE_LOD_BY_POS = componentManager.RegisterEvent("EVENT_ON_UPDATE_LOD_BY_POS",std::type_index(typeid(CModelComponent)));
}

void CModelComponent::Initialize()
{
	BaseModelComponent::Initialize();
	auto &ent = GetEntity();
	auto pRenderComponent = ent.GetComponent<CRenderComponent>();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderBufferDirty();
}

Bool CModelComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetBodyGroup)
	{
		auto groupId = packet->Read<UInt32>();
		auto id = packet->Read<UInt32>();
		SetBodyGroup(groupId,id);
		return true;
	}
	return false;
}

void CModelComponent::ReceiveData(NetPacket &packet)
{
	std::string mdl = packet->ReadString();
	if(!mdl.empty())
		SetModel(mdl.c_str());
	SetSkin(packet->Read<unsigned int>());

	auto numBodyGroups = packet->Read<uint32_t>();
	for(auto i=decltype(numBodyGroups){0};i<numBodyGroups;++i)
	{
		auto bg = packet->Read<uint32_t>();
		SetBodyGroup(i,bg);
	}
}

bool CModelComponent::IsWeighted() const
{
	auto animComponent = GetEntity().GetAnimatedComponent();
	return animComponent.valid() && animComponent->GetBoneCount() > 0u;
}

unsigned char CModelComponent::GetLOD() {return m_lod;}

void CModelComponent::UpdateLOD(UInt32 lod)
{
	//lod = 0u;
	CEOnUpdateLOD evData{lod};
	if(InvokeEventCallbacks(EVENT_ON_UPDATE_LOD,evData) == util::EventReply::Handled)
		return;
	//std::unordered_map<unsigned int,RenderInstance*>::iterator it = m_renderInstances.find(m_lod);
	//if(it != m_renderInstances.end())
	//	it->second->SetEnabled(false);
	m_lod = lod;//CUChar(lod);
	m_lodMeshes.clear();

	auto &mdl = GetModel();
	if(mdl != nullptr)
		mdl->GetBodyGroupMeshes(GetBodyGroups(),lod,m_lodMeshes);
	//UpdateRenderMeshes();
	//it = m_renderInstances.find(m_lod);
	//if(it != m_renderInstances.end())
	//	it->second->SetEnabled(true);
}

void CModelComponent::SetLOD(uint8_t lod) {m_lod = lod;}

void CModelComponent::UpdateLOD(const Vector3 &posCam)
{
	CEOnUpdateLODByPos evData{posCam};
	if(InvokeEventCallbacks(EVENT_ON_UPDATE_LOD_BY_POS,evData) == util::EventReply::Handled)
		return;
	auto mdl = GetModel();
	if(mdl == nullptr)
		return;
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto dist = pTrComponent.valid() ? uvec::distance(pTrComponent->GetPosition(),posCam) : 0.f;
	auto lod = c_game->GetLOD(dist,mdl->GetLODCount());
	if(m_lod == lod)
		return;
	UpdateLOD(lod);
}
std::vector<std::shared_ptr<ModelMesh>> &CModelComponent::GetLODMeshes() {return m_lodMeshes;}
const std::vector<std::shared_ptr<ModelMesh>> &CModelComponent::GetLODMeshes() const {return const_cast<CModelComponent*>(this)->GetLODMeshes();}

bool CModelComponent::SetBodyGroup(UInt32 groupId,UInt32 id)
{
	auto r = BaseModelComponent::SetBodyGroup(groupId,id);
	if(r == false)
		return r;
	UpdateLOD(m_lod); // Update our active meshes
	return true;
}

void CModelComponent::SetModel(const std::string &mdl)
{
	BaseModelComponent::SetModel(mdl);
	BaseModelComponent::SetModel<CModel,CModelMesh,CModelManager>(mdl);
}

void CModelComponent::OnModelChanged(const std::shared_ptr<Model> &model)
{
	m_lod = 0;
	m_lodMeshes.clear();
	if(model == nullptr)
	{
		BaseModelComponent::OnModelChanged(model);
		return;
	}
	UpdateLOD(0);
	BaseModelComponent::OnModelChanged(model);
}

///////////////

CEOnUpdateLOD::CEOnUpdateLOD(uint32_t lod)
	: lod{lod}
{}
void CEOnUpdateLOD::PushArguments(lua_State *l)
{
	Lua::PushInt(l,lod);
}

///////////////

CEOnUpdateLODByPos::CEOnUpdateLODByPos(const Vector3 &posCam)
	: posCam{posCam}
{}
void CEOnUpdateLODByPos::PushArguments(lua_State *l)
{
	Lua::Push<Vector3>(l,posCam);
}

