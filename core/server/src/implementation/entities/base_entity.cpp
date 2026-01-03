// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.base;

import :audio;
import :entities;
import :entities.components;
import :entities.registration;
import :game;
import :model_manager;
import :server_state;

#undef GetClassName

SBaseEntity::SBaseEntity() : BaseEntity(), m_bShared(false), m_bSynchronized(true) {}

void SBaseEntity::DoSpawn()
{
	BaseEntity::DoSpawn();
	pragma::Game *game = pragma::ServerState::Get()->GetGameState();
	game->SpawnEntity(this);
}

void SBaseEntity::OnComponentAdded(pragma::BaseEntityComponent &component)
{
	BaseEntity::OnComponentAdded(component);
	if(typeid(component) == typeid(pragma::STransformComponent))
		m_transformComponent = &static_cast<pragma::STransformComponent &>(component);
	else if(typeid(component) == typeid(pragma::SPhysicsComponent))
		m_physicsComponent = &static_cast<pragma::SPhysicsComponent &>(component);
	else if(typeid(component) == typeid(pragma::SWorldComponent))
		pragma::math::set_flag(m_stateFlags, StateFlags::HasWorldComponent);
	else if(typeid(component) == typeid(pragma::SModelComponent))
		m_modelComponent = &static_cast<pragma::SModelComponent &>(component);
	else if(typeid(component) == typeid(pragma::SGenericComponent))
		m_genericComponent = &static_cast<pragma::SGenericComponent &>(component);
	else if(typeid(component) == typeid(pragma::SChildComponent))
		m_childComponent = &static_cast<pragma::SChildComponent &>(component);
}
void SBaseEntity::OnComponentRemoved(pragma::BaseEntityComponent &component)
{
	BaseEntity::OnComponentRemoved(component);
	if(typeid(component) == typeid(pragma::SWorldComponent))
		pragma::math::set_flag(m_stateFlags, StateFlags::HasWorldComponent, false);
	else if(typeid(component) == typeid(pragma::STransformComponent))
		m_transformComponent = nullptr;
	else if(typeid(component) == typeid(pragma::SPhysicsComponent))
		m_physicsComponent = nullptr;
	else if(typeid(component) == typeid(pragma::SModelComponent))
		m_modelComponent = nullptr;
	else if(typeid(component) == typeid(pragma::SGenericComponent))
		m_genericComponent = nullptr;
	else if(typeid(component) == typeid(pragma::SChildComponent))
		m_childComponent = nullptr;
}

pragma::ecs::BaseEntity *SBaseEntity::GetClientsideEntity() const
{
	if(IsShared() == false)
		return nullptr;
	auto *clState = pragma::Engine::Get()->GetClientState();
	if(clState == nullptr)
		return nullptr;
	auto *game = clState->GetGameState();
	if(game == nullptr)
		return nullptr;
	return game->GetEntity(GetIndex());
}

Bool SBaseEntity::IsSynchronized() const { return (IsShared() && m_bSynchronized) ? true : false; }
void SBaseEntity::SetSynchronized(Bool b) { m_bSynchronized = b; }

void SBaseEntity::Initialize()
{
	BaseEntity::Initialize();

	auto className = server_entities::ServerEntityRegistry::Instance().GetClassName(typeid(*this));
	std::string strClassName = className ? std::string {*className} : std::string {};
	m_className = pragma::ents::register_class_name(strClassName);

	auto ID = server_entities::ServerEntityRegistry::Instance().GetNetworkFactoryID(typeid(*this));
	if(ID == std::nullopt)
		return;
	m_bShared = true;
}

void SBaseEntity::InitializeLuaObject(lua::State *lua) { BaseEntity::InitializeLuaObject<SBaseEntity>(lua); }
bool SBaseEntity::IsShared() const { return m_bShared; }
void SBaseEntity::SetShared(bool b) { m_bShared = b; }
Bool SBaseEntity::IsNetworked() { return (IsShared() && IsSpawned()) ? true : false; }

bool SBaseEntity::IsServersideOnly() const { return IsShared() == false; }
bool SBaseEntity::IsNetworkLocal() const { return IsServersideOnly(); }

void SBaseEntity::SendData(NetPacket &packet, pragma::networking::ClientRecipientFilter &rp)
{
	packet->Write<uint32_t>(GetSpawnFlags());
	packet->Write(GetUuid());

	auto &componentManager = pragma::SGame::Get()->GetEntityComponentManager();
	auto &components = GetComponents();
	auto offset = packet->GetOffset();
	auto numComponents = pragma::math::min(components.size(), static_cast<size_t>(std::numeric_limits<uint8_t>::max()));
	packet->Write<uint8_t>(numComponents);
	for(auto &pComponent : components) {
		if(pComponent.expired() || pComponent->ShouldTransmitNetData() == false) {
			--numComponents;
			continue;
		}
		auto *pNetComponent = dynamic_cast<pragma::SBaseNetComponent *>(pComponent.get());
		if(pNetComponent == nullptr) {
			throw std::logic_error("Component must be derived from SBaseNetComponent if net data is enabled!");
			continue;
		}
		auto id = pComponent->GetComponentId();
		packet->Write<pragma::ComponentId>(id);
		auto szComponent = 0u;
		auto offset = packet->GetOffset();
		packet->Write<uint8_t>(static_cast<uint8_t>(0u));
		pNetComponent->SendData(packet, rp);
		szComponent = packet->GetOffset() - (offset + sizeof(uint8_t));
		if(szComponent > std::numeric_limits<uint8_t>::max())
			throw std::runtime_error("Component size mustn't exceed " + std::to_string(std::numeric_limits<uint8_t>::max()) + " bytes!");
		packet->Write<uint8_t>(szComponent, &offset);
	}
	packet->Write<uint8_t>(numComponents, &offset);
}

void SBaseEntity::SendSnapshotData(NetPacket &, pragma::BasePlayerComponent &) {}

pragma::NetEventId SBaseEntity::RegisterNetEvent(const std::string &name) const { return static_cast<pragma::SGame *>(GetNetworkState()->GetGameState())->RegisterNetEvent(name); }

void SBaseEntity::Remove()
{
	if(pragma::math::is_flag_set(GetStateFlags(), StateFlags::Removed))
		return;
	BaseEntity::Remove();
	pragma::Game *game = pragma::ServerState::Get()->GetGameState();
	game->RemoveEntity(this);
}

pragma::NetworkState *SBaseEntity::GetNetworkState() const { return pragma::ServerState::Get(); }

void SBaseEntity::SendNetEvent(pragma::NetEventId eventId, NetPacket &packet, pragma::networking::Protocol protocol, const pragma::networking::ClientRecipientFilter &rf)
{
	if(!IsShared() || !IsSpawned())
		return;
	pragma::networking::write_entity(packet, this);
	packet->Write<UInt32>(eventId);
	pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::ENT_EVENT, packet, protocol, rf);
}
void SBaseEntity::SendNetEvent(pragma::NetEventId eventId, NetPacket &packet, pragma::networking::Protocol protocol)
{
	if(!IsShared() || !IsSpawned())
		return;
	SendNetEvent(eventId, packet, protocol, pragma::networking::ClientRecipientFilter {});
}
void SBaseEntity::SendNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(!IsShared() || !IsSpawned())
		return;
	SendNetEvent(eventId, packet, pragma::networking::Protocol::FastUnreliable);
}
void SBaseEntity::SendNetEvent(pragma::NetEventId eventId, pragma::networking::Protocol protocol)
{
	if(!IsShared() || !IsSpawned())
		return;
	NetPacket packet {};
	SendNetEvent(eventId, packet, protocol);
}
Bool SBaseEntity::ReceiveNetEvent(pragma::BasePlayerComponent &pl, pragma::NetEventId eventId, NetPacket &packet)
{
	for(auto &pComponent : GetComponents()) {
		auto *pNetComponent = dynamic_cast<pragma::SBaseNetComponent *>(pComponent.get());
		if(pNetComponent == nullptr)
			continue;
		if(pNetComponent->ReceiveNetEvent(pl, eventId, packet))
			return true;
	}
	Con::CWAR << Con::PREFIX_SERVER << "Unhandled net event '" << eventId << "' for entity " << GetClass() << Con::endl;
	return false;
}

pragma::ComponentHandle<pragma::BaseAnimatedComponent> SBaseEntity::GetAnimatedComponent() const
{
	auto pComponent = GetComponent<pragma::SAnimatedComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseAnimatedComponent>() : pragma::ComponentHandle<pragma::BaseAnimatedComponent> {};
}
pragma::ComponentHandle<pragma::BaseWeaponComponent> SBaseEntity::GetWeaponComponent() const
{
	auto pComponent = GetComponent<pragma::SWeaponComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseWeaponComponent>() : pragma::ComponentHandle<pragma::BaseWeaponComponent> {};
}
pragma::ComponentHandle<pragma::BaseVehicleComponent> SBaseEntity::GetVehicleComponent() const
{
	auto pComponent = GetComponent<pragma::SVehicleComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseVehicleComponent>() : pragma::ComponentHandle<pragma::BaseVehicleComponent> {};
}
pragma::ComponentHandle<pragma::BaseAIComponent> SBaseEntity::GetAIComponent() const
{
	auto pComponent = GetComponent<pragma::SAIComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseAIComponent>() : pragma::ComponentHandle<pragma::BaseAIComponent> {};
}
pragma::ComponentHandle<pragma::BaseCharacterComponent> SBaseEntity::GetCharacterComponent() const
{
	auto pComponent = GetComponent<pragma::SCharacterComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseCharacterComponent>() : pragma::ComponentHandle<pragma::BaseCharacterComponent> {};
}
pragma::ComponentHandle<pragma::BasePlayerComponent> SBaseEntity::GetPlayerComponent() const
{
	auto pComponent = GetComponent<pragma::SPlayerComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BasePlayerComponent>() : pragma::ComponentHandle<pragma::BasePlayerComponent> {};
}
pragma::ComponentHandle<pragma::BaseTimeScaleComponent> SBaseEntity::GetTimeScaleComponent() const
{
	auto pComponent = GetComponent<pragma::STimeScaleComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseTimeScaleComponent>() : pragma::ComponentHandle<pragma::BaseTimeScaleComponent> {};
}
pragma::ComponentHandle<pragma::BaseNameComponent> SBaseEntity::GetNameComponent() const
{
	auto pComponent = GetComponent<pragma::SNameComponent>();
	return pComponent.valid() ? pComponent->GetHandle<pragma::BaseNameComponent>() : pragma::ComponentHandle<pragma::BaseNameComponent> {};
}
bool SBaseEntity::IsCharacter() const { return HasComponent<pragma::SCharacterComponent>(); }
bool SBaseEntity::IsPlayer() const { return HasComponent<pragma::SPlayerComponent>(); }
bool SBaseEntity::IsWeapon() const { return HasComponent<pragma::SWeaponComponent>(); }
bool SBaseEntity::IsVehicle() const { return HasComponent<pragma::SVehicleComponent>(); }
bool SBaseEntity::IsNPC() const { return HasComponent<pragma::SAIComponent>(); }

pragma::ComponentHandle<pragma::BaseEntityComponent> SBaseEntity::AddNetworkedComponent(const std::string &name)
{
	auto c = FindComponent(name);
	if(c.valid())
		return c;
	c = AddComponent(name);
	if(c.expired() || IsShared() == false || c->ShouldTransmitNetData() == false)
		return c;
	auto componentId = c->GetComponentId();
	NetPacket packet {};
	pragma::networking::write_entity(packet, this);
	packet->Write<pragma::ComponentId>(componentId);
	static_cast<pragma::ServerState *>(GetNetworkState())->SendPacket(pragma::networking::net_messages::client::ADD_SHARED_COMPONENT, packet, pragma::networking::Protocol::SlowReliable);
	return c;
}
