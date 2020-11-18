/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include <pragma/console/sh_cmd.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;


LINK_ENTITY_TO_CLASS(game_occlusion_culler,COcclusionCuller);
#pragma optimize("",off)
luabind::object COcclusionCullerComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CShadowManagerComponentHandleWrapper>(l);}

void COcclusionCullerComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	m_occlusionOctree = std::make_shared<OcclusionOctree<CBaseEntity*>>(256.f,1'073'741'824.f,4096.f,[](const CBaseEntity *ent,Vector3 &min,Vector3 &max) {
		auto pRenderComponent = ent->GetRenderComponent();
		auto pTrComponent = ent->GetTransformComponent();
		if(pRenderComponent.valid())
			pRenderComponent->GetRenderBounds(&min,&max);
		else
		{
			min = {};
			max = {};
		}
		if(pTrComponent.expired())
			return;
		auto &pos = pTrComponent->GetPosition();
		min += pos;
		max += pos;
	});
	m_occlusionOctree->Initialize();
	m_occlusionOctree->SetSingleReferenceMode(true);
	m_occlusionOctree->SetToStringCallback([](CBaseEntity *ent) -> std::string {
		return ent->GetClass() +" " +std::to_string(ent->GetIndex());
	});

	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
	for(auto *ent : entIt)
		AddEntity(static_cast<CBaseEntity&>(*ent));
}

void COcclusionCullerComponent::AddEntity(CBaseEntity &ent)
{
	// Add entity to octree
	auto cbRenderMode = FunctionCallback<void,std::reference_wrapper<const RenderMode>,std::reference_wrapper<const RenderMode>>::Create(nullptr);
	m_callbacks.push_back(cbRenderMode); // Render mode callback has to be removed in the EVENT_ON_REMOVE event, otherwise the callback will cause the entity to be re-added to the tree AFTER it just has been removed
	auto fInsertOctreeObject = [this](CBaseEntity *ent) {
		m_occlusionOctree->InsertObject(ent);
		auto pTrComponent = ent->GetTransformComponent();
		if(pTrComponent.valid())
		{
			auto &trC = static_cast<CTransformComponent&>(*pTrComponent);
			m_callbacks.push_back(trC.AddEventCallback(CTransformComponent::EVENT_ON_POSE_CHANGED,[this,&ent](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
				if(umath::is_flag_set(static_cast<pragma::CEOnPoseChanged&>(evData.get()).changeFlags,pragma::TransformChangeFlags::PositionChanged) == false)
					return util::EventReply::Unhandled;
				m_occlusionOctree->UpdateObject(ent);
				return util::EventReply::Unhandled;
			}));
		}
		auto pGenericComponent = ent->GetComponent<pragma::CGenericComponent>();
		if(pGenericComponent.valid())
		{
			m_callbacks.push_back(pGenericComponent->BindEventUnhandled(pragma::CModelComponent::EVENT_ON_MODEL_CHANGED,[this,pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				auto *ent = static_cast<CBaseEntity*>(&pGenericComponent->GetEntity());
				m_occlusionOctree->UpdateObject(ent);
			}));
			m_callbacks.push_back(pGenericComponent->BindEventUnhandled(pragma::CRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED,[this,pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				auto *ent = static_cast<CBaseEntity*>(&pGenericComponent->GetEntity());
				m_occlusionOctree->UpdateObject(ent);
			}));
			m_callbacks.push_back(pGenericComponent->BindEventUnhandled(BaseEntity::EVENT_ON_REMOVE,[this,pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				auto *ent = static_cast<CBaseEntity*>(&pGenericComponent->GetEntity());
				m_occlusionOctree->RemoveObject(ent);
				m_occlusionOctree->IterateObjects([](const OcclusionOctree<CBaseEntity*>::Node &node) -> bool {

					return true;
					},[&](const CBaseEntity *entOther) {
						if(entOther == ent)
							throw std::runtime_error("!!"); // TODO: What is this?
					});
			}));
		}
	};
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.expired())
		return;
	auto hThis = GetHandle();
	static_cast<Callback<void,std::reference_wrapper<const RenderMode>,std::reference_wrapper<const RenderMode>>*>(cbRenderMode.get())->SetFunction([this,&ent,fInsertOctreeObject,hThis,cbRenderMode](std::reference_wrapper<const RenderMode> old,std::reference_wrapper<const RenderMode> newMode) mutable {
		if(hThis.expired())
		{
			if(cbRenderMode.IsValid())
				cbRenderMode.Remove();
			return;
		}
		auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
		auto renderMode = pRenderComponent.valid() ? pRenderComponent->GetRenderMode() : RenderMode::None;
		auto &occlusionTree = GetOcclusionOctree();
		if(renderMode == RenderMode::World || renderMode == RenderMode::Skybox || renderMode == RenderMode::Water)
		{
			if(occlusionTree.ContainsObject(&ent) == false)
				fInsertOctreeObject(&ent);
		}
		else
			occlusionTree.RemoveObject(&ent);
	});
	pRenderComponent->GetRenderModeProperty()->AddCallback(cbRenderMode);
	auto renderMode = pRenderComponent->GetRenderMode();
	if(renderMode != RenderMode::World && renderMode != RenderMode::Skybox && renderMode != RenderMode::Water)
		return;
	fInsertOctreeObject(&ent);
}

const OcclusionOctree<CBaseEntity*> &COcclusionCullerComponent::GetOcclusionOctree() const {return const_cast<COcclusionCullerComponent*>(this)->GetOcclusionOctree();}
OcclusionOctree<CBaseEntity*> &COcclusionCullerComponent::GetOcclusionOctree() {return *m_occlusionOctree;}

void COcclusionCullerComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	for(auto &hCb : m_callbacks)
	{
		if(hCb.IsValid() == false)
			continue;
		hCb.Remove();
	}
}

void COcclusionCullerComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
}

////////

void COcclusionCuller::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<COcclusionCullerComponent>();
}


DLLCLIENT void CMD_debug_render_octree_static_print(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&)
{
	if(c_game == nullptr)
		return;
	auto *entWorld = c_game->GetWorld();
	if(entWorld == nullptr)
	{
		Con::cwar<<"WARNING: No world entity found!"<<Con::endl;
		return;
	}
	auto meshTree = static_cast<pragma::CWorldComponent*>(entWorld)->GetMeshTree();
	if(meshTree == nullptr)
	{
		Con::cwar<<"WARNING: World-entity has no octree!"<<Con::endl;
		return;
	}
	meshTree->DebugPrint();
}

DLLCLIENT void CMD_debug_render_octree_dynamic_print(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	if(c_game == nullptr)
		return;
	auto *scene = c_game->GetScene();
	auto *culler = scene ? scene->FindOcclusionCuller() : nullptr;
	if(culler == nullptr)
		return;
	auto &octree = culler->GetOcclusionOctree();
	octree.DebugPrint();
}

DLLCLIENT void CMD_debug_render_octree_dynamic_find(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	if(c_game == NULL || pl == NULL)
		return;
	auto &entPl = pl->GetEntity();
	if(entPl.IsCharacter() == false)
		return;
	auto charComponent = entPl.GetCharacterComponent();
	auto ents = command::find_target_entity(state,*charComponent,argv);
	if(ents.empty())
		return;
	auto *scene = c_game->GetScene();
	auto *culler = scene ? scene->FindOcclusionCuller() : nullptr;
	if(culler == nullptr)
		return;
	Con::cout<<"Searching for entity '";
	auto *entFind = ents.front();
	entFind->print(Con::cout);
	Con::cout<<"'..."<<Con::endl;

	auto &octree = culler->GetOcclusionOctree();
	std::function<const OcclusionOctree<CBaseEntity*>::Node*(const OcclusionOctree<CBaseEntity*>::Node &node)> iterateTree = nullptr;
	iterateTree = [&iterateTree,entFind](const OcclusionOctree<CBaseEntity*>::Node &node) -> const OcclusionOctree<CBaseEntity*>::Node* {
		auto &nodeBounds = node.GetWorldBounds();
		auto &objs = node.GetObjects();
		auto it = std::find(objs.begin(),objs.end(),entFind);
		if(it != objs.end())
			return &node;
		auto *children = node.GetChildren();
		if(children == nullptr)
			return nullptr;
		for(auto &c : *children)
		{
			auto *node = iterateTree(static_cast<OcclusionOctree<CBaseEntity*>::Node&>(*c));
			if(node)
				return node;
		}
		return nullptr;
	};
	auto *node = iterateTree(octree.GetRootNode());
	if(node == nullptr)
	{
		Con::cout<<"Not found!"<<Con::endl;
		return;
	}
	Con::cout<<"Found in:"<<Con::endl;
	node->DebugPrint();
}

static void CVAR_CALLBACK_debug_render_octree_static_draw(NetworkState*,ConVar*,bool,bool val)
{
	if(c_game == nullptr)
		return;
	auto *entWorld = c_game->GetWorld();
	if(entWorld == nullptr)
	{
		Con::cwar<<"WARNING: No world entity found!"<<Con::endl;
		return;
	}
	auto meshTree = static_cast<pragma::CWorldComponent*>(entWorld)->GetMeshTree();
	if(meshTree == nullptr)
	{
		Con::cwar<<"WARNING: World-entity has no octree!"<<Con::endl;
		return;
	}
	meshTree->SetDebugModeEnabled(val);
	/*if(c_game == nullptr)
	return;
	auto mode = OcclusionCulling::GetMode();
	if(mode != OcclusionCulling::Mode::CHC)
	{
	std::cout<<"This command requires 'cl_render_occlusion_culling' to be set to '2'"<<std::endl;
	return;
	}
	auto *octTree = OcclusionCulling::GetRenderOctTree();
	if(octTree == nullptr)
	return;
	octTree->ShowOctTree(val);
	auto *chc = OcclusionCulling::GetCHC();
	if(chc == nullptr)
	return;
	chc->SetDrawDebugTexture(val);*/
}
REGISTER_CONVAR_CALLBACK_CL(debug_render_octree_static_draw,CVAR_CALLBACK_debug_render_octree_static_draw);

static void CVAR_CALLBACK_debug_render_octree_dynamic_draw(NetworkState*,ConVar*,bool,bool val)
{
	if(c_game == nullptr)
		return;
	auto *scene = c_game->GetScene();
	auto *culler = scene ? scene->FindOcclusionCuller() : nullptr;
	if(culler == nullptr)
		return;
	auto &octree = culler->GetOcclusionOctree();
	octree.SetDebugModeEnabled(val);
}
REGISTER_CONVAR_CALLBACK_CL(debug_render_octree_dynamic_draw,CVAR_CALLBACK_debug_render_octree_dynamic_draw);
#pragma optimize("",on)
