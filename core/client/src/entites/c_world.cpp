/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/c_world.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/physics/collisiontypes.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_bsp_component.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <util_bsp.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void CWorldComponent::Initialize()
{
	BaseWorldComponent::Initialize();

	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		c_game->UpdateEnvironmentLightSource();

		m_lodBaseMeshIds.clear();
		auto &ent = GetEntity();
		auto mdlComponent = ent.GetModelComponent();
		auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
		if(mdl == nullptr)
		{
			ReloadMeshCache();
			return;
		}
		auto &lods = mdl->GetLODs();
		for(auto &lod : lods)
		{
			for(auto &pair : lod.meshReplacements)
				m_lodBaseMeshIds[pair.first] = true;
		}
		ReloadMeshCache();
	});
	BindEventUnhandled(CPhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		Vector3 min {};
		Vector3 max {};
		auto &ent = GetEntity();
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent.valid())
			pPhysComponent->GetCollisionBounds(&min,&max);
		auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
		if(pRenderComponent.valid())
			pRenderComponent->SetRenderBounds(min,max);
	});
	BindEvent(CModelComponent::EVENT_ON_UPDATE_LOD,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		// World determines LOD meshes individually, so do nothing here
		return util::EventReply::Handled;
	});
	BindEvent(CModelComponent::EVENT_ON_UPDATE_LOD_BY_POS,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		OnUpdateLOD(static_cast<CEOnUpdateLODByPos&>(evData.get()).posCam);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(CColorComponent::EVENT_ON_COLOR_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &onColorChangedData = static_cast<pragma::CEOnColorChanged&>(evData.get());
		EntityIterator entIt {*c_game};
		entIt.AttachFilter<TEntityIteratorFilterComponent<CLightDirectionalComponent>>();
		for(auto *ent : entIt)
		{
			auto pToggleComponent = ent->GetComponent<CToggleComponent>();
			if(pToggleComponent.valid() && pToggleComponent->IsTurnedOn())
				return; // Ambient color already defined by environmental light entity
		}
		c_game->GetWorldEnvironment().SetAmbientColor(onColorChangedData.color.ToVector4());
	});
	auto &ent = GetEntity();
	ent.AddComponent<CBSPComponent>();
	ent.AddComponent<CLightMapComponent>();
}
void CWorldComponent::ReloadCHCController()
{
	/*auto &scene = c_game->GetScene();
	auto &cam = scene->GetCamera();
	m_chcController = std::make_shared<CHC>(*cam);
	m_chcController->Reset(m_meshTree);*/ // prosper TODO
}
void CWorldComponent::SetBSPTree(const std::shared_ptr<util::BSPTree> &bspTree) {m_bspTree = bspTree;}
const std::shared_ptr<util::BSPTree> &CWorldComponent::GetBSPTree() const {return m_bspTree;}
void CWorldComponent::ReloadMeshCache()
{
	m_meshTree = nullptr;
	m_chcController = nullptr;
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	m_meshTree = std::make_shared<OcclusionOctree<std::shared_ptr<ModelMesh>>>(256.f,1'073'741'824.f,4096.f,[](const std::weak_ptr<ModelMesh> ptrSubMesh,Vector3 &min,Vector3 &max) {
		if(ptrSubMesh.expired() == true)
		{
			min = {};
			max = {};
			return;
		}
		auto subMesh = ptrSubMesh.lock();
		subMesh->GetBounds(min,max);
	});
	m_meshTree->Initialize();
	m_meshTree->SetSingleReferenceMode(true);
	m_meshTree->SetToStringCallback([](std::weak_ptr<ModelMesh> ptrMesh) -> std::string {
		if(ptrMesh.expired() == true)
			return "Expired";
		auto subMesh = ptrMesh.lock();
		std::stringstream ss;
		ss<<subMesh.get()<<" ("<<subMesh->GetTriangleCount()<<" Tris, "<<subMesh->GetVertexCount()<<" Vertices)";
		return ss.str();
	});
	OnUpdateLOD(Vector3{});
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.valid())
	{
		for(auto &mesh : pRenderComponent->GetLODMeshes())
			m_meshTree->InsertObject(mesh);
	}
	ReloadCHCController();
}
void CWorldComponent::OnEntitySpawn()
{
	BaseWorldComponent::OnEntitySpawn();
}
void CWorldComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseWorldComponent::OnEntityComponentAdded(component);
}
std::shared_ptr<OcclusionOctree<std::shared_ptr<ModelMesh>>> CWorldComponent::GetMeshTree() const {return m_meshTree;};
std::shared_ptr<CHC> CWorldComponent::GetCHCController() const {return m_chcController;}

void CWorldComponent::OnUpdateLOD(const Vector3 &posCam)
{
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto *mdlComponent = static_cast<CModelComponent*>(ent.GetModelComponent().get());
	mdlComponent->SetLOD(0);
	auto &mdl = mdlComponent->GetModel();
	if(mdl == nullptr)
		return;
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.expired())
		return;
	auto pTrComponent = ent.GetTransformComponent();
	auto &lodMeshes = pRenderComponent->GetLODMeshes();
	lodMeshes.clear();
	auto &pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	auto &baseMeshes = mdl->GetBaseMeshes();
	for(auto id : baseMeshes)
	{
		auto it = m_lodBaseMeshIds.find(id);
		if(it == m_lodBaseMeshIds.end())
		{
			auto group = mdl->GetMeshGroup(id);
			if(group != nullptr)
			{
				for(auto &mesh : group->GetMeshes())
					lodMeshes.push_back(mesh);
			}
			continue;
		}
		auto group = mdl->GetMeshGroup(id);
		if(group == nullptr)
			continue;
		auto &meshes = group->GetMeshes();
		if(meshes.empty())
			continue;
		auto &mesh = meshes.front();
		Vector3 min,max;
		mesh->GetBounds(min,max);
		min += pos;
		max += pos;
		Vector3 aabbPos;
		Geometry::ClosestPointOnAABBToPoint(min,max,posCam,&aabbPos);

		auto dist = uvec::distance(posCam,aabbPos);
		auto lod = CUInt32(dist /LOD_SWAP_DISTANCE);
		auto *lodInfo = mdl->GetLODInfo(lod);
		if(lodInfo != nullptr)
		{
			auto it = lodInfo->meshReplacements.find(id);
			if(it != lodInfo->meshReplacements.end())
			{
				auto lodGroup = mdl->GetMeshGroup(it->second);
				if(lodGroup != nullptr)
					group = lodGroup;
			}
		}
		for(auto &mesh : group->GetMeshes())
			lodMeshes.push_back(mesh);
	}
}
luabind::object CWorldComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CWorldComponentHandleWrapper>(l);}

//////////////

LINK_ENTITY_TO_CLASS(world,CWorld);

void CWorld::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWorldComponent>();
}
bool CWorld::IsWorld() const {return true;}

Con::c_cout& CWorld::print(Con::c_cout &os)
{
	os<<"CWorld["<<m_index<<"]"<<"["<<GetClass()<<"]"<<"[";
	auto mdlComponent = GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		os<<"NULL";
	else
		os<<mdl->GetName();
	os<<"]";
	return os;
}

std::ostream& CWorld::print(std::ostream &os)
{
	os<<"CWorld["<<m_index<<"]"<<"["<<GetClass()<<"]"<<"[";
	auto mdlComponent = GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		os<<"NULL";
	else
		os<<mdl->GetName();
	os<<"]";
	return os;
}
