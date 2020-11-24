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

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

ComponentEventId CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED = INVALID_COMPONENT_ID;
luabind::object CModelComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CModelComponentHandleWrapper>(l);}
void CModelComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	BaseModelComponent::RegisterEvents(componentManager);
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

uint32_t CModelComponent::GetLOD() const {return m_lod;}

void CModelComponent::UpdateRenderMeshes()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::RenderMeshUpdateRequired) == false)
		return;
	umath::set_flag(m_stateFlags,StateFlags::RenderMeshUpdateRequired,false);
	m_lodRenderMeshes.clear();
	m_lodMeshes.clear();

	auto &mdl = GetModel();
	auto numLods = umath::max(mdl ? mdl->GetLODCount() : 1u,static_cast<uint32_t>(1));
	m_lodRenderMeshes.resize(numLods);
	m_lodMeshes.resize(numLods);
	if(mdl != nullptr)
	{
		for(auto i=decltype(numLods){0u};i<numLods;++i)
		{
			mdl->GetBodyGroupMeshes(GetBodyGroups(),i,m_lodMeshes[i]);
			for(auto &mesh : m_lodMeshes[i])
			{
				for(auto &subMesh : mesh->GetSubMeshes())
					m_lodRenderMeshes[i].push_back(subMesh);
			}
		}
	}
}

void CModelComponent::UpdateLOD(UInt32 lod)
{
	UpdateRenderMeshes();
	//std::unordered_map<unsigned int,RenderInstance*>::iterator it = m_renderInstances.find(m_lod);
	//if(it != m_renderInstances.end())
	//	it->second->SetEnabled(false);
	m_lod = lod;//CUChar(lod);

	//UpdateRenderMeshes();
	//it = m_renderInstances.find(m_lod);
	//if(it != m_renderInstances.end())
	//	it->second->SetEnabled(true);
	UpdateRenderMeshes();
	BroadcastEvent(EVENT_ON_RENDER_MESHES_UPDATED);
}

void CModelComponent::SetLOD(uint32_t lod) {m_lod = lod;}

void CModelComponent::SetAutoLodEnabled(bool enabled) {umath::set_flag(m_stateFlags,StateFlags::AutoLodDisabled,!enabled);}
bool CModelComponent::IsAutoLodEnabled() const {return !umath::is_flag_set(m_stateFlags,StateFlags::AutoLodDisabled);}

void CModelComponent::UpdateLOD(const CSceneComponent &scene,const CCameraComponent &cam,const Mat4 &vp)
{
	UpdateRenderMeshes();
	if(IsAutoLodEnabled() == false)
		return;
	auto &mdl = GetModel();
	if(mdl == nullptr || mdl->GetLODCount() == 0)
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

std::vector<std::shared_ptr<ModelMesh>> &CModelComponent::GetLODMeshes() {return GetLODMeshes(m_lod);}
const std::vector<std::shared_ptr<ModelMesh>> &CModelComponent::GetLODMeshes() const {return const_cast<CModelComponent*>(this)->GetLODMeshes();}
std::vector<std::shared_ptr<ModelSubMesh>> &CModelComponent::GetRenderMeshes() {return GetRenderMeshes(m_lod);}
const std::vector<std::shared_ptr<ModelSubMesh>> &CModelComponent::GetRenderMeshes() const {return const_cast<CModelComponent*>(this)->GetRenderMeshes();}

std::vector<std::shared_ptr<ModelMesh>> &CModelComponent::GetLODMeshes(uint32_t lod)
{
	UpdateRenderMeshes();
	lod = umath::min(lod,static_cast<uint32_t>(m_lodMeshes.size() -1));
	assert(lod < m_lodMeshes.size());
	return m_lodMeshes[lod];
}
const std::vector<std::shared_ptr<ModelMesh>> &CModelComponent::GetLODMeshes(uint32_t lod) const {return const_cast<CModelComponent*>(this)->GetLODMeshes(lod);}
std::vector<std::shared_ptr<ModelSubMesh>> &CModelComponent::GetRenderMeshes(uint32_t lod)
{
	UpdateRenderMeshes();
	lod = umath::min(lod,static_cast<uint32_t>(m_lodRenderMeshes.size() -1));
	assert(lod < m_lodRenderMeshes.size());
	return m_lodRenderMeshes[lod];
}
const std::vector<std::shared_ptr<ModelSubMesh>> &CModelComponent::GetRenderMeshes(uint32_t lod) const {return const_cast<CModelComponent*>(this)->GetRenderMeshes(lod);}

bool CModelComponent::SetBodyGroup(UInt32 groupId,UInt32 id)
{
	auto r = BaseModelComponent::SetBodyGroup(groupId,id);
	if(r == false)
		return r;
	UpdateLOD(m_lod); // Update our active meshes
	umath::set_flag(m_stateFlags,StateFlags::RenderMeshUpdateRequired);
	return true;
}

void CModelComponent::OnModelChanged(const std::shared_ptr<Model> &model)
{
	m_lod = 0;

	m_lodRenderMeshes.clear();
	m_lodRenderMeshes.push_back({});

	m_lodMeshes.clear();
	m_lodMeshes.push_back({});

	umath::set_flag(m_stateFlags,StateFlags::RenderMeshUpdateRequired);

	if(model == nullptr)
	{
		BaseModelComponent::OnModelChanged(model);
		return;
	}
	UpdateLOD(0);
	BaseModelComponent::OnModelChanged(model);
}
