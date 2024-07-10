/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/audio/s_alsound.h"
#include "pragma/audio/s_alsoundscript.h"
#include <pragma/lua/luacallback.h>
#include <pragma/networking/nwm_util.h>
#include "luasystem.h"
#include "pragma/entities/parentinfo.h"
#include <pragma/lua/luafunction_call.h>
#include "pragma/entities/player.h"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/lua/lua_handles.hpp"
#include "pragma/model/s_modelmanager.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/components/s_child_component.hpp"
#include "pragma/entities/components/s_model_component.hpp"
#include "pragma/entities/components/s_animated_component.hpp"
#include "pragma/entities/components/s_weapon_component.hpp"
#include "pragma/entities/components/s_vehicle_component.hpp"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_physics_component.hpp"
#include "pragma/entities/components/s_time_scale_component.hpp"
#include "pragma/entities/components/s_name_component.hpp"
#include "pragma/entities/components/s_transform_component.hpp"
#include "pragma/entities/components/s_generic_component.hpp"
#include <servermanager/sv_nwm_recipientfilter.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>
#include <sharedutils/scope_guard.h>
#include <pragma/lua/libraries/ltimer.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/physics/raytraces.h>
#include <pragma/physics/collisionmasks.h>
#include <pragma/util/bulletinfo.h>
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/components/base_entity_component.hpp>
#include <pragma/entities/components/base_actor_component.hpp>
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern EntityClassMap<SBaseEntity> *g_ServerEntityFactories;
extern ServerEntityNetworkMap *g_SvEntityNetworkMap;

extern DLLNETWORK Engine *engine;
extern ServerState *server;
extern SGame *s_game;

LINK_ENTITY_TO_CLASS(entity, SBaseEntity);

SBaseEntity::SBaseEntity() : BaseEntity(), m_bShared(false), m_bSynchronized(true) {}

void SBaseEntity::DoSpawn()
{
	BaseEntity::DoSpawn();
	Game *game = server->GetGameState();
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
		umath::set_flag(m_stateFlags, StateFlags::HasWorldComponent);
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
		umath::set_flag(m_stateFlags, StateFlags::HasWorldComponent, false);
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

BaseEntity *SBaseEntity::GetClientsideEntity() const
{
	if(IsShared() == false)
		return nullptr;
	auto *clState = engine->GetClientState();
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

	std::string className;
	g_ServerEntityFactories->GetClassName(typeid(*this), &className);
	m_className = pragma::ents::register_class_name(className);

	unsigned int ID = g_SvEntityNetworkMap->GetFactoryID(typeid(*this));
	if(ID == 0)
		return;
	m_bShared = true;
}
void SBaseEntity::InitializeLuaObject(lua_State *lua) { pragma::BaseLuaHandle::InitializeLuaObject<SBaseEntity>(lua); }
bool SBaseEntity::IsShared() const { return m_bShared; }
void SBaseEntity::SetShared(bool b) { m_bShared = b; }
Bool SBaseEntity::IsNetworked() { return (IsShared() && IsSpawned()) ? true : false; }

bool SBaseEntity::IsServersideOnly() const { return IsShared() == false; }
bool SBaseEntity::IsNetworkLocal() const { return IsServersideOnly(); }

void SBaseEntity::SendData(NetPacket &packet, pragma::networking::ClientRecipientFilter &rp)
{
	packet->Write<uint32_t>(GetSpawnFlags());
	packet->Write(GetUuid());

	auto &componentManager = s_game->GetEntityComponentManager();
	auto &components = GetComponents();
	auto offset = packet->GetOffset();
	auto numComponents = umath::min(components.size(), static_cast<size_t>(std::numeric_limits<uint8_t>::max()));
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

pragma::NetEventId SBaseEntity::RegisterNetEvent(const std::string &name) const { return static_cast<SGame *>(GetNetworkState()->GetGameState())->RegisterNetEvent(name); }

void SBaseEntity::Remove()
{
	if(umath::is_flag_set(GetStateFlags(), BaseEntity::StateFlags::Removed))
		return;
	BaseEntity::Remove();
	Game *game = server->GetGameState();
	game->RemoveEntity(this);
}

NetworkState *SBaseEntity::GetNetworkState() const { return server; }

void SBaseEntity::SendNetEvent(pragma::NetEventId eventId, NetPacket &packet, pragma::networking::Protocol protocol, const pragma::networking::ClientRecipientFilter &rf)
{
	if(!IsShared() || !IsSpawned())
		return;
	nwm::write_entity(packet, this);
	packet->Write<UInt32>(eventId);
	server->SendPacket("ent_event", packet, protocol, rf);
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
	Con::cwar << Con::PREFIX_SERVER << "Unhandled net event '" << eventId << "' for entity " << GetClass() << Con::endl;
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
	nwm::write_entity(packet, this);
	packet->Write<pragma::ComponentId>(componentId);
	static_cast<ServerState *>(GetNetworkState())->SendPacket("add_shared_component", packet, pragma::networking::Protocol::SlowReliable);
	return c;
}
