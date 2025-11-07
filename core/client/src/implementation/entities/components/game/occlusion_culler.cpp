// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

#include "pragma/console/helper.hpp"

module pragma.client;

import :entities.components.game_occlusion_culler;
import :client_state;
import :engine;
import :game;

using namespace pragma;

void COcclusionCullerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void COcclusionCullerComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	m_occlusionOctree = std::make_shared<OcclusionOctree<CBaseEntity *>>(256.f, 1'073'741'824.f, 4096.f, [](const CBaseEntity *ent, Vector3 &min, Vector3 &max) {
		auto &renderBounds = ent->GetAbsoluteRenderBounds();
		min = renderBounds.min;
		max = renderBounds.max;
	});
	m_occlusionOctree->Initialize();
	m_occlusionOctree->SetSingleReferenceMode(true);
	m_occlusionOctree->SetToStringCallback([](CBaseEntity *ent) -> std::string { return std::string {*ent->GetClass()} + " " + std::to_string(ent->GetIndex()); });

	pragma::ecs::EntityIterator entIt {*pragma::get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
	for(auto *ent : entIt)
		AddEntity(static_cast<CBaseEntity &>(*ent));
}

void COcclusionCullerComponent::AddEntity(CBaseEntity &ent)
{
	// Add entity to octree
	auto cbRenderMode = FunctionCallback<util::EventReply, std::reference_wrapper<ComponentEvent>>::Create(nullptr);
	auto it = m_callbacks.find(&ent);
	if(it == m_callbacks.end())
		it = m_callbacks.insert(std::make_pair(&ent, std::vector<CallbackHandle> {})).first;
	it->second.push_back(cbRenderMode); // Render mode callback has to be removed in the EVENT_ON_REMOVE event, otherwise the callback will cause the entity to be re-added to the tree AFTER it just has been removed
	auto fInsertOctreeObject = [this](CBaseEntity *ent) {
		auto it = m_callbacks.find(ent);
		if(it == m_callbacks.end())
			return;
		SceneRenderDesc::AssertRenderQueueThreadInactive();
		m_occlusionOctree->InsertObject(ent);
		auto pTrComponent = ent->GetTransformComponent();
		if(pTrComponent != nullptr) {
			auto &trC = static_cast<CTransformComponent &>(*pTrComponent);
			it->second.push_back(trC.AddEventCallback(cTransformComponent::EVENT_ON_POSE_CHANGED, [this, ent](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
				if(umath::is_flag_set(static_cast<pragma::CEOnPoseChanged &>(evData.get()).changeFlags, pragma::TransformChangeFlags::PositionChanged) == false)
					return util::EventReply::Unhandled;
				// SceneRenderDesc::AssertRenderQueueThreadInactive();
				// Note: Entity positions should generally not be updated during rendering,
				// however in some cases they have to be updated every frame (e.g. for attachables).
				// In those cases we still mustn't update the entity in the octree though, so we
				// check for it here. TODO: This is a messy solution, find a better way!
				if(SceneRenderDesc::GetActiveRenderQueueThreadCount() == 0)
					m_occlusionOctree->UpdateObject(ent);
				return util::EventReply::Unhandled;
			}));
		}
		auto pGenericComponent = ent->GetComponent<pragma::CGenericComponent>();
		if(pGenericComponent.valid()) {
			it->second.push_back(pGenericComponent->BindEventUnhandled(pragma::cModelComponent::EVENT_ON_MODEL_CHANGED, [this, pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				SceneRenderDesc::AssertRenderQueueThreadInactive();
				auto *ent = static_cast<CBaseEntity *>(&pGenericComponent->GetEntity());
				m_occlusionOctree->UpdateObject(ent);
			}));
			it->second.push_back(pGenericComponent->BindEventUnhandled(pragma::cRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED, [this, pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				SceneRenderDesc::AssertRenderQueueThreadInactive();
				auto *ent = static_cast<CBaseEntity *>(&pGenericComponent->GetEntity());
				m_occlusionOctree->UpdateObject(ent);
			}));
			it->second.push_back(pGenericComponent->BindEventUnhandled(pragma::ecs::baseEntity::EVENT_ON_REMOVE, [this, pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				auto *ent = static_cast<CBaseEntity *>(&pGenericComponent->GetEntity());
				auto it = m_callbacks.find(ent);

				// We need to copy the this pointer because removing the callback invalidates the captured variable
				auto *thisCpy = this;
				if(it != m_callbacks.end()) {
					for(auto &hCb : it->second) {
						if(hCb.IsValid() == false)
							continue;
						hCb.Remove();
					}
					thisCpy->m_callbacks.erase(it);
				}
				SceneRenderDesc::AssertRenderQueueThreadInactive();
				thisCpy->m_occlusionOctree->RemoveObject(ent);
			}));
		}
	};
	auto pRenderComponent = ent.GetRenderComponent();
	if(!pRenderComponent)
		return;
	auto hThis = GetHandle();
	static_cast<Callback<util::EventReply, std::reference_wrapper<pragma::ComponentEvent>> *>(cbRenderMode.get())->SetFunction([this, &ent, fInsertOctreeObject, hThis, cbRenderMode](std::reference_wrapper<pragma::ComponentEvent> evData) mutable -> util::EventReply {
		if(hThis.expired()) {
			if(cbRenderMode.IsValid())
				cbRenderMode.Remove();
			return util::EventReply::Unhandled;
		}
		auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
		auto renderMode = pRenderComponent.valid() ? pRenderComponent->GetSceneRenderPass() : pragma::rendering::SceneRenderPass::None;
		auto &occlusionTree = GetOcclusionOctree();
		if(renderMode == pragma::rendering::SceneRenderPass::World || renderMode == pragma::rendering::SceneRenderPass::Sky) {
			if(occlusionTree.ContainsObject(&ent) == false)
				fInsertOctreeObject(&ent);
		}
		else
			occlusionTree.RemoveObject(&ent);
		return util::EventReply::Unhandled;
	});
	pRenderComponent->AddEventCallback(cRenderComponent::EVENT_ON_RENDER_MODE_CHANGED, cbRenderMode);
	auto renderMode = pRenderComponent->GetSceneRenderPass();
	if(renderMode != pragma::rendering::SceneRenderPass::World && renderMode != pragma::rendering::SceneRenderPass::Sky)
		return;
	fInsertOctreeObject(&ent);
}

const OcclusionOctree<CBaseEntity *> &COcclusionCullerComponent::GetOcclusionOctree() const { return const_cast<COcclusionCullerComponent *>(this)->GetOcclusionOctree(); }
OcclusionOctree<CBaseEntity *> &COcclusionCullerComponent::GetOcclusionOctree() { return *m_occlusionOctree; }

void COcclusionCullerComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	for(auto &pair : m_callbacks) {
		for(auto &hCb : pair.second) {
			if(hCb.IsValid() == false)
				continue;
			hCb.Remove();
		}
	}
}

void COcclusionCullerComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }

////////

void COcclusionCuller::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<COcclusionCullerComponent>();
}

DLLCLIENT void CMD_debug_render_octree_static_print(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(pragma::get_cgame() == nullptr)
		return;
	auto *entWorld = pragma::get_cgame()->GetWorld();
	if(entWorld == nullptr) {
		Con::cwar << "No world entity found!" << Con::endl;
		return;
	}
	auto meshTree = static_cast<pragma::CWorldComponent *>(entWorld)->GetMeshTree();
	if(meshTree == nullptr) {
		Con::cwar << "World-entity has no octree!" << Con::endl;
		return;
	}
	meshTree->DebugPrint();
}

DLLCLIENT void CMD_debug_render_octree_dynamic_print(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(pragma::get_cgame() == nullptr)
		return;
	auto *scene = pragma::get_cgame()->GetScene<pragma::CSceneComponent>();
	auto *culler = scene ? scene->FindOcclusionCuller<pragma::COcclusionCullerComponent>() : nullptr;
	if(culler == nullptr)
		return;
	auto &octree = culler->GetOcclusionOctree();
	octree.DebugPrint();
}

DLLCLIENT void CMD_debug_render_octree_dynamic_find(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(pragma::get_client_state() == nullptr || pl == nullptr)
		return;
	auto &entPl = pl->GetEntity();
	if(entPl.IsCharacter() == false)
		return;
	auto charComponent = entPl.GetCharacterComponent();
	auto ents = command::find_target_entity(state, *charComponent, argv);
	if(ents.empty())
		return;
	auto *scene = pragma::get_cgame()->GetScene<pragma::CSceneComponent>();
	auto *culler = scene ? scene->FindOcclusionCuller<pragma::COcclusionCullerComponent>() : nullptr;
	if(culler == nullptr)
		return;
	auto *entFind = ents.front();
	Con::cout << "Searching for entity '" << *entFind << "'..." << Con::endl;

	auto &octree = culler->GetOcclusionOctree();
	std::function<const OcclusionOctree<CBaseEntity *>::Node *(const OcclusionOctree<CBaseEntity *>::Node &node)> iterateTree = nullptr;
	iterateTree = [&iterateTree, entFind](const OcclusionOctree<CBaseEntity *>::Node &node) -> const OcclusionOctree<CBaseEntity *>::Node * {
		auto &nodeBounds = node.GetWorldBounds();
		auto &objs = node.GetObjects();
		auto it = std::find(objs.begin(), objs.end(), entFind);
		if(it != objs.end())
			return &node;
		auto *children = node.GetChildren();
		if(children == nullptr)
			return nullptr;
		for(auto &c : *children) {
			auto *node = iterateTree(static_cast<OcclusionOctree<CBaseEntity *>::Node &>(*c));
			if(node)
				return node;
		}
		return nullptr;
	};
	auto *node = iterateTree(octree.GetRootNode());
	if(node == nullptr) {
		Con::cout << "Not found!" << Con::endl;
		return;
	}
	Con::cout << "Found in:" << Con::endl;
	node->DebugPrint();
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_render_octree_static_print", &CMD_debug_render_octree_static_print, pragma::console::ConVarFlags::None, "Prints the octree for static world geometry to the console, or a file if a file name is specified.");
	auto UVN = pragma::console::client::register_command("debug_render_octree_dynamic_print", &CMD_debug_render_octree_dynamic_print, pragma::console::ConVarFlags::None, "Prints the octree for dynamic objects to the console, or a file if a file name is specified.");
	auto UVN = pragma::console::client::register_command("debug_render_octree_dynamic_find", &CMD_debug_render_octree_dynamic_find, pragma::console::ConVarFlags::None, "Finds the specified entity in the octree for dynamic objects.");
}

static void CVAR_CALLBACK_debug_render_octree_static_draw(NetworkState *, const ConVar &, bool, bool val)
{
	if(pragma::get_cgame() == nullptr)
		return;
	auto *entWorld = pragma::get_cgame()->GetWorld();
	if(entWorld == nullptr) {
		Con::cwar << "No world entity found!" << Con::endl;
		return;
	}
	auto meshTree = static_cast<pragma::CWorldComponent *>(entWorld)->GetMeshTree();
	if(meshTree == nullptr) {
		Con::cwar << "World-entity has no octree!" << Con::endl;
		return;
	}
	meshTree->SetDebugModeEnabled(val);
	/*if(pragma::get_cgame() == nullptr)
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
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("debug_render_octree_static_draw", &CVAR_CALLBACK_debug_render_octree_static_draw);
}

static void CVAR_CALLBACK_debug_render_octree_dynamic_draw(NetworkState *, const ConVar &, bool, bool val)
{
	if(pragma::get_cgame() == nullptr)
		return;
	auto *scene = pragma::get_cgame()->GetScene<pragma::CSceneComponent>();
	auto *culler = scene ? scene->FindOcclusionCuller<pragma::COcclusionCullerComponent>() : nullptr;
	if(culler == nullptr)
		return;
	auto &octree = culler->GetOcclusionOctree();
	octree.SetDebugModeEnabled(val);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>("debug_render_octree_dynamic_draw", &CVAR_CALLBACK_debug_render_octree_dynamic_draw);
}
