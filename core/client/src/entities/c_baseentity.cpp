/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "cmaterialmanager.h"
#include "pragma/model/c_modelmanager.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/model/c_model.h"
#include "pragma/model/brush/c_brushmesh.h"
#include <pragma/math/intersection.h>
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/parentinfo.h"
#include "pragma/rendering/rendersystem.h"
#include <materialmanager.h>
#include "pragma/model/c_modelmesh.h"
#include <pragma/logging.hpp>
#include <pragma/lua/luacallback.h>
#include <pragma/networking/nwm_util.h>
#include "luasystem.h"
#include <pragma/lua/luafunction_call.h>
#include "pragma/util/util_smoke_trail.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_child_component.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/c_name_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_generic_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/components/c_vehicle_component.hpp"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_time_scale_component.hpp"
#include "pragma/entities/components/c_ai_component.hpp"
#include <pragma/util/bulletinfo.h>
#include <pragma/physics/raytraces.h>
#include <pragma/physics/collisionmasks.h>
#include <pragma/audio/alsound_type.h>
#include <pragma/physics/environment.hpp>
#include <sharedutils/scope_guard.h>
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/util/util_handled.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/composite_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

LINK_ENTITY_TO_CLASS(entity, CBaseEntity);

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void CBaseEntity::OnComponentAdded(pragma::BaseEntityComponent &component)
{
	BaseEntity::OnComponentAdded(component);
	auto typeIndex = std::type_index(typeid(component));
	if(typeid(component) == typeid(pragma::CRenderComponent))
		m_renderComponent = &static_cast<pragma::CRenderComponent &>(component);
	else if(typeid(component) == typeid(pragma::CTransformComponent))
		m_transformComponent = &static_cast<pragma::CTransformComponent &>(component);
	else if(typeid(component) == typeid(pragma::CPhysicsComponent))
		m_physicsComponent = &static_cast<pragma::CPhysicsComponent &>(component);
	else if(typeid(component) == typeid(pragma::CWorldComponent))
		umath::set_flag(m_stateFlags, StateFlags::HasWorldComponent);
	else if(typeid(component) == typeid(pragma::CModelComponent))
		m_modelComponent = &static_cast<pragma::CModelComponent &>(component);
	else if(typeid(component) == typeid(pragma::CGenericComponent))
		m_genericComponent = &static_cast<pragma::CGenericComponent &>(component);
	else if(typeid(component) == typeid(pragma::CChildComponent))
		m_childComponent = &static_cast<pragma::CChildComponent &>(component);
	else if(typeid(component) == typeid(pragma::CompositeComponent)) {
		static_cast<pragma::CompositeComponent &>(component).AddEventCallback(pragma::CompositeComponent::EVENT_ON_ENTITY_ADDED, [this](std::reference_wrapper<pragma::ComponentEvent> e) -> util::EventReply {
			auto &evData = static_cast<pragma::CECompositeEntityChanged &>(e.get());
			static_cast<CBaseEntity &>(evData.ent).GetSceneFlagsProperty()->Link(*GetSceneFlagsProperty()); // TODO: This skips the EVENT_ON_SCENE_FLAGS_CHANGED event
			return util::EventReply::Unhandled;
		});
		static_cast<pragma::CompositeComponent &>(component).AddEventCallback(pragma::CompositeComponent::EVENT_ON_ENTITY_REMOVED, [this](std::reference_wrapper<pragma::ComponentEvent> e) -> util::EventReply {
			auto &evData = static_cast<pragma::CECompositeEntityChanged &>(e.get());
			static_cast<CBaseEntity &>(evData.ent).GetSceneFlagsProperty()->Unlink(); // TODO: This skips the EVENT_ON_SCENE_FLAGS_CHANGED event
			return util::EventReply::Unhandled;
		});
	}
}
void CBaseEntity::OnComponentRemoved(pragma::BaseEntityComponent &component)
{
	BaseEntity::OnComponentRemoved(component);
	if(typeid(component) == typeid(pragma::CWorldComponent))
		umath::set_flag(m_stateFlags, StateFlags::HasWorldComponent, false);
	else if(typeid(component) == typeid(pragma::CRenderComponent))
		m_renderComponent = nullptr;
	else if(typeid(component) == typeid(pragma::CTransformComponent))
		m_transformComponent = nullptr;
	else if(typeid(component) == typeid(pragma::CPhysicsComponent))
		m_physicsComponent = nullptr;
	else if(typeid(component) == typeid(pragma::CModelComponent))
		m_modelComponent = nullptr;
	else if(typeid(component) == typeid(pragma::CGenericComponent))
		m_genericComponent = nullptr;
	else if(typeid(component) == typeid(pragma::CChildComponent))
		m_childComponent = nullptr;
}
pragma::CRenderComponent *CBaseEntity::GetRenderComponent() const { return m_renderComponent; }
void CBaseEntity::InitializeLuaObject(lua_State *lua) { pragma::BaseLuaHandle::InitializeLuaObject<CBaseEntity>(lua); }

//////////////////////////////////

extern EntityClassMap<CBaseEntity> *g_ClientEntityFactories;
pragma::ComponentEventId CBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED = pragma::INVALID_COMPONENT_ID;
void CBaseEntity::RegisterEvents(pragma::EntityComponentManager &componentManager) { EVENT_ON_SCENE_FLAGS_CHANGED = componentManager.RegisterEvent("ON_SCENE_FLAGS_CHANGED", typeid(BaseEntity), pragma::ComponentEventInfo::Type::Broadcast); }

CBaseEntity::CBaseEntity() : BaseEntity(), m_sceneFlags {util::UInt32Property::Create(0)} {}

BaseEntity *CBaseEntity::GetServersideEntity() const
{
	if(IsClientsideOnly() == true)
		return nullptr;
	auto *svState = c_engine->GetServerNetworkState();
	if(svState == nullptr)
		return nullptr;
	auto *game = svState->GetGameState();
	if(game == nullptr)
		return nullptr;
	return game->GetEntity(GetIndex());
}

static uint64_t get_scene_flag(const pragma::CSceneComponent &scene)
{
	auto index = scene.GetSceneIndex();
	return 1 << index;
}
const util::PUInt32Property &CBaseEntity::GetSceneFlagsProperty() const { return m_sceneFlags; }
uint32_t CBaseEntity::GetSceneFlags() const { return *m_sceneFlags; }
void CBaseEntity::AddToScene(pragma::CSceneComponent &scene)
{
	*m_sceneFlags = **m_sceneFlags | get_scene_flag(scene);
	BroadcastEvent(EVENT_ON_SCENE_FLAGS_CHANGED);
}
void CBaseEntity::RemoveFromScene(pragma::CSceneComponent &scene)
{
	*m_sceneFlags = **m_sceneFlags & ~get_scene_flag(scene);
	BroadcastEvent(EVENT_ON_SCENE_FLAGS_CHANGED);
}
void CBaseEntity::RemoveFromAllScenes()
{
	*m_sceneFlags = 0;
	BroadcastEvent(EVENT_ON_SCENE_FLAGS_CHANGED);
}
bool CBaseEntity::IsInScene(const pragma::CSceneComponent &scene) const { return (**m_sceneFlags & get_scene_flag(scene)) != 0; }
std::vector<pragma::CSceneComponent *> CBaseEntity::GetScenes() const
{
	std::vector<pragma::CSceneComponent *> scenes {};
	auto numScenes = sizeof(pragma::CSceneComponent::SceneFlags) * 8;
	scenes.reserve(numScenes);
	for(auto i = decltype(numScenes) {0u}; i < numScenes; ++i) {
		auto sceneFlag = static_cast<pragma::CSceneComponent::SceneFlags>(1 << i);
		if((**m_sceneFlags & sceneFlag) == 0)
			continue;
		auto *scene = pragma::CSceneComponent::GetByIndex(i);
		if(scene == nullptr)
			continue;
		scenes.push_back(scene);
	}
	return scenes;
}

void CBaseEntity::Construct(unsigned int idx, unsigned int clientIdx)
{
	m_clientIdx = clientIdx;
	BaseEntity::Construct(idx);
}

unsigned int CBaseEntity::GetClientIndex() { return m_clientIdx; }
uint32_t CBaseEntity::GetLocalIndex() const { return const_cast<CBaseEntity *>(this)->GetClientIndex(); }

void CBaseEntity::Initialize()
{
	BaseEntity::Initialize();
	std::string className;
	g_ClientEntityFactories->GetClassName(typeid(*this), &className);
	m_className = pragma::ents::register_class_name(className);
}

void CBaseEntity::DoSpawn()
{
	BaseEntity::DoSpawn();
	c_game->SpawnEntity(this);
}

Bool CBaseEntity::ReceiveNetEvent(UInt32 eventId, NetPacket &p)
{
	for(auto &pComponent : GetComponents()) {
		auto *pNetComponent = dynamic_cast<pragma::CBaseNetComponent *>(pComponent.get());
		if(pNetComponent == nullptr)
			continue;
		if(pNetComponent->ReceiveNetEvent(eventId, p))
			return true;
	}
	Con::cwar << Con::PREFIX_CLIENT << "Unhandled net event '" << eventId << "' for entity " << GetClass() << Con::endl;
	return false;
}

void CBaseEntity::ReceiveData(NetPacket &packet)
{
	m_spawnFlags = packet->Read<uint32_t>();
	SetUuid(packet->Read<util::Uuid>());

	auto &componentManager = static_cast<pragma::CEntityComponentManager &>(c_game->GetEntityComponentManager());
	auto &componentTypes = componentManager.GetRegisteredComponentTypes();
	auto &svComponentToClComponentTable = componentManager.GetServerComponentIdToClientComponentIdTable();
	auto numComponents = packet->Read<uint8_t>();
	for(auto i = decltype(numComponents) {0u}; i < numComponents; ++i) {
		auto svId = packet->Read<pragma::ComponentId>();
		auto componentSize = packet->Read<uint8_t>();
		auto offset = packet->GetOffset();
		if(svId < svComponentToClComponentTable.size() && svComponentToClComponentTable.at(svId) != pragma::CEntityComponentManager::INVALID_COMPONENT) {
			auto clId = svComponentToClComponentTable.at(svId);
			if(clId >= componentTypes.size())
				throw std::runtime_error("Invalid client component type index " + std::to_string(clId) + "!");
			auto *componentInfo = componentManager.GetComponentInfo(clId);
			if(componentInfo == nullptr)
				throw std::invalid_argument("Invalid component id " + std::to_string(clId) + "!");
			else {
				std::string componentName {componentInfo->name};
				auto component = FindComponent(componentName); // TODO: FindComponent should use std::string_view
				if(component.expired()) {
					// The client doesn't know about component, so we'll add it here.
					// This should usually never happen, except for cases where components
					// have been added the entity, of which the entity's implementation doesn't know about.
					// (This can be the case for map entities for instance.)
					component = AddComponent(clId);
				}
				if(component.expired() == false) {
					auto *netComponent = dynamic_cast<pragma::CBaseNetComponent *>(component.get());
					if(netComponent != nullptr)
						netComponent->ReceiveData(packet);
				}
				else {
					std::stringstream ss;
					ss << componentInfo->name;
					spdlog::warn("Net data for entity {} contained component data for component '{}' which hasn't been attached to the entity clientside! Skipping...", ToString(), ss.str());
				}
			}
		}
		else
			spdlog::warn("Net data for entity {} contained component data for component type with non-existing clientside representation! Skipping...", ToString());
		packet->SetOffset(offset + componentSize);
	}
}

void CBaseEntity::ReceiveSnapshotData(NetPacket &) {}

void CBaseEntity::OnRemove()
{
	auto mdlComponent = GetModelComponent();
	if(mdlComponent)
		mdlComponent->SetModel(std::shared_ptr<Model>(nullptr)); // Make sure to clear all clientside model mesh references
	BaseEntity::OnRemove();
}

void CBaseEntity::Remove()
{
	if(umath::is_flag_set(GetStateFlags(), BaseEntity::StateFlags::Removed))
		return;
	BaseEntity::Remove();
	SceneRenderDesc::AssertRenderQueueThreadInactive();
	Game *game = client->GetGameState();
	game->RemoveEntity(this);
}

NetworkState *CBaseEntity::GetNetworkState() const { return client; }

bool CBaseEntity::IsClientsideOnly() const { return (GetIndex() == 0) ? true : false; }

bool CBaseEntity::IsNetworkLocal() const { return IsClientsideOnly(); }

void CBaseEntity::SendNetEventTCP(UInt32 eventId) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	NetPacket p;
	SendNetEventTCP(eventId, p);
}
void CBaseEntity::SendNetEventTCP(UInt32 eventId, NetPacket &data) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	eventId = c_game->LocalNetEventIdToShared(eventId);
	if(eventId == std::numeric_limits<pragma::NetEventId>::max()) {
		Con::cwar << "Attempted to send net event " << eventId << " which has no known serverside id associated!" << Con::endl;
		return;
	}
	nwm::write_entity(data, this);
	data->Write<UInt32>(eventId);
	client->SendPacket("ent_event", data, pragma::networking::Protocol::SlowReliable);
}
void CBaseEntity::SendNetEventUDP(UInt32 eventId) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	NetPacket p;
	SendNetEventUDP(eventId, p);
}
void CBaseEntity::SendNetEventUDP(UInt32 eventId, NetPacket &data) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	;
	eventId = c_game->LocalNetEventIdToShared(eventId);
	if(eventId == std::numeric_limits<pragma::NetEventId>::max()) {
		Con::cwar << "Attempted to send net event " << eventId << " which has no known serverside id associated!" << Con::endl;
		return;
	}
	nwm::write_entity(data, this);
	data->Write<UInt32>(eventId);
	client->SendPacket("ent_event", data, pragma::networking::Protocol::FastUnreliable);
}
pragma::ComponentHandle<pragma::BaseAnimatedComponent> CBaseEntity::GetAnimatedComponent() const
{
	auto pComponent = GetComponent<pragma::CAnimatedComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseAnimatedComponent>() : pragma::ComponentHandle<pragma::BaseAnimatedComponent> {};
}
pragma::ComponentHandle<pragma::BaseWeaponComponent> CBaseEntity::GetWeaponComponent() const
{
	auto pComponent = GetComponent<pragma::CWeaponComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseWeaponComponent>() : pragma::ComponentHandle<pragma::BaseWeaponComponent> {};
}
pragma::ComponentHandle<pragma::BaseVehicleComponent> CBaseEntity::GetVehicleComponent() const
{
	auto pComponent = GetComponent<pragma::CVehicleComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseVehicleComponent>() : pragma::ComponentHandle<pragma::BaseVehicleComponent> {};
}
pragma::ComponentHandle<pragma::BaseAIComponent> CBaseEntity::GetAIComponent() const
{
	auto pComponent = GetComponent<pragma::CAIComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseAIComponent>() : pragma::ComponentHandle<pragma::BaseAIComponent> {};
}
pragma::ComponentHandle<pragma::BaseCharacterComponent> CBaseEntity::GetCharacterComponent() const
{
	auto pComponent = GetComponent<pragma::CCharacterComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseCharacterComponent>() : pragma::ComponentHandle<pragma::BaseCharacterComponent> {};
}
pragma::ComponentHandle<pragma::BasePlayerComponent> CBaseEntity::GetPlayerComponent() const
{
	auto pComponent = GetComponent<pragma::CPlayerComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BasePlayerComponent>() : pragma::ComponentHandle<pragma::BasePlayerComponent> {};
}
pragma::ComponentHandle<pragma::BaseTimeScaleComponent> CBaseEntity::GetTimeScaleComponent() const
{
	auto pComponent = GetComponent<pragma::CTimeScaleComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseTimeScaleComponent>() : pragma::ComponentHandle<pragma::BaseTimeScaleComponent> {};
}
pragma::ComponentHandle<pragma::BaseNameComponent> CBaseEntity::GetNameComponent() const
{
	auto pComponent = GetComponent<pragma::CNameComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseNameComponent>() : pragma::ComponentHandle<pragma::BaseNameComponent> {};
}
bool CBaseEntity::IsCharacter() const { return HasComponent<pragma::CCharacterComponent>(); }
bool CBaseEntity::IsPlayer() const { return HasComponent<pragma::CPlayerComponent>(); }
bool CBaseEntity::IsWeapon() const { return HasComponent<pragma::CWeaponComponent>(); }
bool CBaseEntity::IsVehicle() const { return HasComponent<pragma::CVehicleComponent>(); }
bool CBaseEntity::IsNPC() const { return HasComponent<pragma::CAIComponent>(); }

const bounding_volume::AABB &CBaseEntity::GetLocalRenderBounds() const
{
	auto *renderC = GetRenderComponent();
	if(renderC == nullptr) {
		static bounding_volume::AABB bounds {};
		return bounds;
	}
	return renderC->GetLocalRenderBounds();
}
const bounding_volume::AABB &CBaseEntity::GetAbsoluteRenderBounds(bool updateBounds) const
{
	auto *renderC = GetRenderComponent();
	if(renderC == nullptr) {
		static bounding_volume::AABB bounds {};
		return bounds;
	}
	return updateBounds ? renderC->GetUpdatedAbsoluteRenderBounds() : renderC->GetAbsoluteRenderBounds();
}

void CBaseEntity::AddChild(CBaseEntity &ent)
{
	RemoveEntityOnRemoval(&ent);
	ent.m_sceneFlags->Link(*m_sceneFlags);
}
