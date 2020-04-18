#include "stdafx_client.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

#pragma optimize("",off)
LINK_ENTITY_TO_CLASS(game_occlusion_culler,COcclusionCuller);

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
			m_callbacks.push_back(pTrComponent->GetPosProperty()->AddCallback([this,ent](std::reference_wrapper<const Vector3> oldPos,std::reference_wrapper<const Vector3> pos) {
				m_occlusionOctree->UpdateObject(ent);
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
				for(auto &cb : m_callbacks)
				{
					if(cb.IsValid() == false)
						continue;
					cb.Remove();
				}
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
#pragma optimize("",on)
