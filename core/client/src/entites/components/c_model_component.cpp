/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/c_modelmanager.h"

using namespace pragma;
#pragma optimize("",off)
extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

ComponentEventId CModelComponent::EVENT_ON_UPDATE_LOD = INVALID_COMPONENT_ID;
ComponentEventId CModelComponent::EVENT_ON_UPDATE_LOD_BY_POS = INVALID_COMPONENT_ID;
ComponentEventId CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED = INVALID_COMPONENT_ID;
luabind::object CModelComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CModelComponentHandleWrapper>(l);}
void CModelComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	BaseModelComponent::RegisterEvents(componentManager);
	EVENT_ON_UPDATE_LOD = componentManager.RegisterEvent("EVENT_ON_UPDATE_LOD",std::type_index(typeid(CModelComponent)));
	EVENT_ON_UPDATE_LOD_BY_POS = componentManager.RegisterEvent("EVENT_ON_UPDATE_LOD_BY_POS",std::type_index(typeid(CModelComponent)));
	EVENT_ON_RENDER_MESHES_UPDATED = componentManager.RegisterEvent("EVENT_ON_RENDER_MESHES_UPDATED");
}

void CModelComponent::Initialize()
{
	BaseModelComponent::Initialize();
	auto &ent = GetEntity();
	auto pRenderComponent = ent.GetComponent<CRenderComponent>();
	if(pRenderComponent.valid())
		pRenderComponent->SetRenderBufferDirty();
}

void CModelComponent::SetMaterialOverride(uint32_t idx,const std::string &matOverride)
{
	if(idx >= m_materialOverrides.size())
		m_materialOverrides.resize(idx +1);
	m_materialOverrides.at(idx) = client->LoadMaterial(matOverride);
}
void CModelComponent::SetMaterialOverride(uint32_t idx,CMaterial &mat)
{
	if(idx >= m_materialOverrides.size())
		m_materialOverrides.resize(idx +1);
	m_materialOverrides.at(idx) = mat.GetHandle();
}
void CModelComponent::ClearMaterialOverride(uint32_t idx)
{
	if(idx >= m_materialOverrides.size())
		return;
	m_materialOverrides.at(idx) = {};
}
CMaterial *CModelComponent::GetMaterialOverride(uint32_t idx) const
{
	return (idx < m_materialOverrides.size()) ? static_cast<CMaterial*>(m_materialOverrides.at(idx).get()) : nullptr;
}
const std::vector<MaterialHandle> &CModelComponent::GetMaterialOverrides() const {return m_materialOverrides;}

CMaterial *CModelComponent::GetRenderMaterial(uint32_t idx) const
{
	auto *matOverride = GetMaterialOverride(idx);
	if(matOverride)
		return matOverride;
	auto &mdl = GetModel();
	if(mdl == nullptr)
		return nullptr;
	return static_cast<CMaterial*>(mdl->GetMaterial(idx));
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
	m_renderMeshes.clear();
	m_lodMeshes.clear();

	auto &mdl = GetModel();
	if(mdl != nullptr)
	{
		mdl->GetBodyGroupMeshes(GetBodyGroups(),lod,m_lodMeshes);
		for(auto &mesh : m_lodMeshes)
		{
			for(auto &subMesh : mesh->GetSubMeshes())
				m_renderMeshes.push_back(subMesh);
		}
	}
	//UpdateRenderMeshes();
	//it = m_renderInstances.find(m_lod);
	//if(it != m_renderInstances.end())
	//	it->second->SetEnabled(true);
	BroadcastEvent(EVENT_ON_RENDER_MESHES_UPDATED);
}

void CModelComponent::SetLOD(uint8_t lod) {m_lod = lod;}

void CModelComponent::UpdateLOD(const CSceneComponent &scene,const CCameraComponent &cam,const Mat4 &vp)
{
	auto &mdl = GetModel();
	if(mdl == nullptr || mdl->GetLODCount() == 0)
		return;

	CEOnUpdateLODByPos evData{scene,cam,vp};
	if(InvokeEventCallbacks(EVENT_ON_UPDATE_LOD_BY_POS,evData) == util::EventReply::Handled)
		return;

	// TODO: This needs optimizing
	auto &pos = GetEntity().GetPosition();
	auto w = scene.GetWidth();
	auto h = scene.GetHeight();
	auto posOffset = pos +cam.GetEntity().GetUp() *1.f;
	auto uvMin = umat::to_screen_uv(pos,vp);
	auto uvMax = umat::to_screen_uv(posOffset,vp);
	auto extents = umath::max(uvMin.y,uvMax.y) -umath::min(uvMin.y,uvMax.y);
	extents *= h;
	extents *= 2.f; // TODO: Why?

	auto size = 100.f /extents;
	auto &lods = mdl->GetLODs();
	uint32_t lod = 0;
	for(auto i=decltype(lods.size()){0u};i<lods.size();++i)
	{
		auto &lodInfo = lods.at(i);
		if(size >= lodInfo.distance)
		{
			lod = i +1;
			continue;
		}
		break;
	}
	lod += c_game->GetLODBias();
	if(m_lod == lod)
		return;
	UpdateLOD(lod);
}
std::vector<std::shared_ptr<ModelSubMesh>> &CModelComponent::GetRenderMeshes() {return m_renderMeshes;}
const std::vector<std::shared_ptr<ModelSubMesh>> &CModelComponent::GetRenderMeshes() const {return const_cast<CModelComponent*>(this)->GetRenderMeshes();}

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

void CModelComponent::OnModelChanged(const std::shared_ptr<Model> &model)
{
	m_lod = 0;
	m_renderMeshes.clear();
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

CEOnUpdateLODByPos::CEOnUpdateLODByPos(const CSceneComponent &scene,const CCameraComponent &cam,const Mat4 &vp)
	: scene{scene},camera{cam},viewProjection{vp}
{}
void CEOnUpdateLODByPos::PushArguments(lua_State *l)
{
	scene.GetLuaObject().push(l);
	camera.GetLuaObject().push(l);
	Lua::Push<Mat4>(l,viewProjection);
}
#pragma optimize("",on)
