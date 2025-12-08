// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#undef GetClassName

module pragma.client;

import :entities.base_entity;

import :client_state;
import :engine;
import :entities.components;
import :model;
import :game;
import :util;
import pragma.shared;

void pragma::ecs::CBaseEntity::OnComponentAdded(pragma::BaseEntityComponent &component)
{
	pragma::ecs::BaseEntity::OnComponentAdded(component);
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
	else if(typeid(component) == typeid(pragma::ecs::CompositeComponent)) {
		static_cast<pragma::ecs::CompositeComponent &>(component).AddEventCallback(pragma::ecs::compositeComponent::EVENT_ON_ENTITY_ADDED, [this](std::reference_wrapper<pragma::ComponentEvent> e) -> util::EventReply {
			auto &evData = static_cast<pragma::ecs::events::CECompositeEntityChanged &>(e.get());
			static_cast<pragma::ecs::CBaseEntity &>(evData.ent).GetSceneFlagsProperty()->Link(*GetSceneFlagsProperty()); // TODO: This skips the EVENT_ON_SCENE_FLAGS_CHANGED event
			return util::EventReply::Unhandled;
		});
		static_cast<pragma::ecs::CompositeComponent &>(component).AddEventCallback(pragma::ecs::compositeComponent::EVENT_ON_ENTITY_REMOVED, [this](std::reference_wrapper<pragma::ComponentEvent> e) -> util::EventReply {
			auto &evData = static_cast<pragma::ecs::events::CECompositeEntityChanged &>(e.get());
			static_cast<pragma::ecs::CBaseEntity &>(evData.ent).GetSceneFlagsProperty()->Unlink(); // TODO: This skips the EVENT_ON_SCENE_FLAGS_CHANGED event
			return util::EventReply::Unhandled;
		});
	}
}
void pragma::ecs::CBaseEntity::OnComponentRemoved(pragma::BaseEntityComponent &component)
{
	pragma::ecs::BaseEntity::OnComponentRemoved(component);
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
pragma::CRenderComponent *pragma::ecs::CBaseEntity::GetRenderComponent() const { return m_renderComponent; }
void pragma::ecs::CBaseEntity::InitializeLuaObject(lua::State *lua) { pragma::BaseLuaHandle::InitializeLuaObject<pragma::ecs::CBaseEntity>(lua); }

//////////////////////////////////

pragma::ComponentEventId cBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED = pragma::INVALID_COMPONENT_ID;
void pragma::ecs::CBaseEntity::RegisterEvents(pragma::EntityComponentManager &componentManager) { cBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED = componentManager.RegisterEvent("ON_SCENE_FLAGS_CHANGED", typeid(pragma::ecs::BaseEntity), pragma::ComponentEventInfo::Type::Broadcast); }

pragma::ecs::CBaseEntity::CBaseEntity() : pragma::ecs::BaseEntity(), m_sceneFlags {util::UInt32Property::Create(0)} {}

pragma::ecs::BaseEntity *pragma::ecs::CBaseEntity::GetServersideEntity() const
{
	if(IsClientsideOnly() == true)
		return nullptr;
	auto *svState = pragma::get_cengine()->GetServerNetworkState();
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
const util::PUInt32Property &pragma::ecs::CBaseEntity::GetSceneFlagsProperty() const { return m_sceneFlags; }
uint32_t pragma::ecs::CBaseEntity::GetSceneFlags() const { return *m_sceneFlags; }
void pragma::ecs::CBaseEntity::AddToScene(pragma::CSceneComponent &scene)
{
	*m_sceneFlags = **m_sceneFlags | get_scene_flag(scene);
	BroadcastEvent(cBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED);
}
void pragma::ecs::CBaseEntity::RemoveFromScene(pragma::CSceneComponent &scene)
{
	*m_sceneFlags = **m_sceneFlags & ~get_scene_flag(scene);
	BroadcastEvent(cBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED);
}
void pragma::ecs::CBaseEntity::RemoveFromAllScenes()
{
	*m_sceneFlags = 0;
	BroadcastEvent(cBaseEntity::EVENT_ON_SCENE_FLAGS_CHANGED);
}
bool pragma::ecs::CBaseEntity::IsInScene(const pragma::CSceneComponent &scene) const { return (**m_sceneFlags & get_scene_flag(scene)) != 0; }
std::vector<pragma::CSceneComponent *> pragma::ecs::CBaseEntity::GetScenes() const
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

void pragma::ecs::CBaseEntity::Construct(unsigned int idx, unsigned int clientIdx)
{
	m_clientIdx = clientIdx;
	pragma::ecs::BaseEntity::Construct(idx);
}

unsigned int pragma::ecs::CBaseEntity::GetClientIndex() { return m_clientIdx; }
uint32_t pragma::ecs::CBaseEntity::GetLocalIndex() const { return const_cast<pragma::ecs::CBaseEntity *>(this)->GetClientIndex(); }

void pragma::ecs::CBaseEntity::Initialize()
{
	pragma::ecs::BaseEntity::Initialize();
	auto className = client_entities::ClientEntityRegistry::Instance().GetClassName(typeid(*this));
	std::string strClassName = className ? std::string {*className} : std::string {};
	m_className = pragma::ents::register_class_name(strClassName);
}

void pragma::ecs::CBaseEntity::DoSpawn()
{
	pragma::ecs::BaseEntity::DoSpawn();
	pragma::get_cgame()->SpawnEntity(this);
}

Bool pragma::ecs::CBaseEntity::ReceiveNetEvent(UInt32 eventId, NetPacket &p)
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

void pragma::ecs::CBaseEntity::ReceiveData(NetPacket &packet)
{
	m_spawnFlags = packet->Read<uint32_t>();
	SetUuid(packet->Read<util::Uuid>());

	auto &componentManager = static_cast<pragma::CEntityComponentManager &>(pragma::get_cgame()->GetEntityComponentManager());
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

void pragma::ecs::CBaseEntity::ReceiveSnapshotData(NetPacket &) {}

void pragma::ecs::CBaseEntity::OnRemove()
{
	auto mdlComponent = GetModelComponent();
	if(mdlComponent)
		mdlComponent->SetModel(std::shared_ptr<pragma::asset::Model>(nullptr)); // Make sure to clear all clientside model mesh references
	pragma::ecs::BaseEntity::OnRemove();
}

void pragma::ecs::CBaseEntity::Remove()
{
	if(umath::is_flag_set(GetStateFlags(), pragma::ecs::BaseEntity::StateFlags::Removed))
		return;
	pragma::ecs::BaseEntity::Remove();
	SceneRenderDesc::AssertRenderQueueThreadInactive();
	pragma::Game *game = pragma::get_client_state()->GetGameState();
	game->RemoveEntity(this);
}

pragma::NetworkState *pragma::ecs::CBaseEntity::GetNetworkState() const { return pragma::get_client_state(); }

bool pragma::ecs::CBaseEntity::IsClientsideOnly() const { return (GetIndex() == 0) ? true : false; }

bool pragma::ecs::CBaseEntity::IsNetworkLocal() const { return IsClientsideOnly(); }

void pragma::ecs::CBaseEntity::SendNetEventTCP(UInt32 eventId) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	NetPacket p;
	SendNetEventTCP(eventId, p);
}
void pragma::ecs::CBaseEntity::SendNetEventTCP(UInt32 eventId, NetPacket &data) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	eventId = pragma::get_cgame()->LocalNetEventIdToShared(eventId);
	if(eventId == std::numeric_limits<pragma::NetEventId>::max()) {
		Con::cwar << "Attempted to send net event " << eventId << " which has no known serverside id associated!" << Con::endl;
		return;
	}
	pragma::networking::write_entity(data, this);
	data->Write<UInt32>(eventId);
	pragma::get_client_state()->SendPacket(pragma::networking::net_messages::server::ENT_EVENT, data, pragma::networking::Protocol::SlowReliable);
}
void pragma::ecs::CBaseEntity::SendNetEventUDP(UInt32 eventId) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	NetPacket p;
	SendNetEventUDP(eventId, p);
}
void pragma::ecs::CBaseEntity::SendNetEventUDP(UInt32 eventId, NetPacket &data) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	;
	eventId = pragma::get_cgame()->LocalNetEventIdToShared(eventId);
	if(eventId == std::numeric_limits<pragma::NetEventId>::max()) {
		Con::cwar << "Attempted to send net event " << eventId << " which has no known serverside id associated!" << Con::endl;
		return;
	}
	pragma::networking::write_entity(data, this);
	data->Write<UInt32>(eventId);
	pragma::get_client_state()->SendPacket(pragma::networking::net_messages::server::ENT_EVENT, data, pragma::networking::Protocol::FastUnreliable);
}
pragma::ComponentHandle<pragma::BaseAnimatedComponent> pragma::ecs::CBaseEntity::GetAnimatedComponent() const
{
	auto pComponent = GetComponent<pragma::CAnimatedComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseAnimatedComponent>() : pragma::ComponentHandle<pragma::BaseAnimatedComponent> {};
}
pragma::ComponentHandle<pragma::BaseWeaponComponent> pragma::ecs::CBaseEntity::GetWeaponComponent() const
{
	auto pComponent = GetComponent<pragma::CWeaponComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseWeaponComponent>() : pragma::ComponentHandle<pragma::BaseWeaponComponent> {};
}
pragma::ComponentHandle<pragma::BaseVehicleComponent> pragma::ecs::CBaseEntity::GetVehicleComponent() const
{
	auto pComponent = GetComponent<pragma::CVehicleComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseVehicleComponent>() : pragma::ComponentHandle<pragma::BaseVehicleComponent> {};
}
pragma::ComponentHandle<pragma::BaseAIComponent> pragma::ecs::CBaseEntity::GetAIComponent() const
{
	auto pComponent = GetComponent<pragma::CAIComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseAIComponent>() : pragma::ComponentHandle<pragma::BaseAIComponent> {};
}
pragma::ComponentHandle<pragma::BaseCharacterComponent> pragma::ecs::CBaseEntity::GetCharacterComponent() const
{
	auto pComponent = GetComponent<pragma::CCharacterComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseCharacterComponent>() : pragma::ComponentHandle<pragma::BaseCharacterComponent> {};
}
pragma::ComponentHandle<pragma::BasePlayerComponent> pragma::ecs::CBaseEntity::GetPlayerComponent() const
{
	auto pComponent = GetComponent<pragma::CPlayerComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BasePlayerComponent>() : pragma::ComponentHandle<pragma::BasePlayerComponent> {};
}
pragma::ComponentHandle<pragma::BaseTimeScaleComponent> pragma::ecs::CBaseEntity::GetTimeScaleComponent() const
{
	auto pComponent = GetComponent<pragma::CTimeScaleComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseTimeScaleComponent>() : pragma::ComponentHandle<pragma::BaseTimeScaleComponent> {};
}
pragma::ComponentHandle<pragma::BaseNameComponent> pragma::ecs::CBaseEntity::GetNameComponent() const
{
	auto pComponent = GetComponent<pragma::CNameComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseNameComponent>() : pragma::ComponentHandle<pragma::BaseNameComponent> {};
}
bool pragma::ecs::CBaseEntity::IsCharacter() const { return HasComponent<pragma::CCharacterComponent>(); }
bool pragma::ecs::CBaseEntity::IsPlayer() const { return HasComponent<pragma::CPlayerComponent>(); }
bool pragma::ecs::CBaseEntity::IsWeapon() const { return HasComponent<pragma::CWeaponComponent>(); }
bool pragma::ecs::CBaseEntity::IsVehicle() const { return HasComponent<pragma::CVehicleComponent>(); }
bool pragma::ecs::CBaseEntity::IsNPC() const { return HasComponent<pragma::CAIComponent>(); }

const bounding_volume::AABB &pragma::ecs::CBaseEntity::GetLocalRenderBounds() const
{
	auto *renderC = GetRenderComponent();
	if(renderC == nullptr) {
		static bounding_volume::AABB bounds {};
		return bounds;
	}
	return renderC->GetLocalRenderBounds();
}
const bounding_volume::AABB &pragma::ecs::CBaseEntity::GetAbsoluteRenderBounds(bool updateBounds) const
{
	auto *renderC = GetRenderComponent();
	if(renderC == nullptr) {
		static bounding_volume::AABB bounds {};
		return bounds;
	}
	return updateBounds ? renderC->GetUpdatedAbsoluteRenderBounds() : renderC->GetAbsoluteRenderBounds();
}

void pragma::ecs::CBaseEntity::AddChild(CBaseEntity &ent)
{
	RemoveEntityOnRemoval(&ent);
	ent.m_sceneFlags->Link(*m_sceneFlags);
}
