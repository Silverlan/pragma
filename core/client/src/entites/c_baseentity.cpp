#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/rendering/shaders/c_shader.h"
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
#include "pragma/rendering/uniformbinding.h"
#include "pragma/rendering/rendersystem.h"
#include <materialmanager.h>
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/luacallback.h>
#include <pragma/networking/nwm_util.h>
#include "luasystem.h"
#include "pragma/rendering/scene/scene.h"
#include <pragma/lua/luafunction_call.h>
#include "pragma/util/util_smoke_trail.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/c_name_component.hpp"
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
#include <pragma/entities/entity_component_system_t.hpp>

LINK_ENTITY_TO_CLASS(baseentity,CBaseEntity);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void CBaseEntity::OnComponentAdded(pragma::BaseEntityComponent &component)
{
	BaseEntity::OnComponentAdded(component);
	auto *ptrRenderComponent = dynamic_cast<pragma::CRenderComponent*>(&component);
	if(ptrRenderComponent != nullptr)
		m_renderComponent = std::static_pointer_cast<pragma::CRenderComponent>(ptrRenderComponent->shared_from_this());
}
util::WeakHandle<pragma::CRenderComponent> CBaseEntity::GetRenderComponent() const {return m_renderComponent;}

//////////////////////////////////

extern EntityClassMap<CBaseEntity> *g_ClientEntityFactories;
CBaseEntity::CBaseEntity()
	: BaseEntity()
{}

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
// Obsolete
/*
void CBaseEntity::UpdateAnimatedState()
{
	auto bAnimated = false;
	if(GetModelComponent().GetAnimation() != -1 || GetModelComponent().GetAnimationSlotInfos().empty() == false)
		bAnimated = true;
	else
	{
		auto pPhysComponent = GetPhysicsComponent();
		if(pPhysComponent != nullptr && pPhysComponent->GetPhysConstraints().empty() == false)
			bAnimated = true;
	}
	GetModelComponent().SetAnimated(bAnimated);
}
*/

void CBaseEntity::Construct(unsigned int idx,unsigned int clientIdx)
{
	m_clientIdx = clientIdx;
	BaseEntity::Construct(idx);
}

unsigned int CBaseEntity::GetClientIndex() {return m_clientIdx;}

void CBaseEntity::Initialize()
{
	BaseEntity::Initialize();
	CGame *game = client->GetGameState();
	lua_State *lua = game->GetLuaState();
	InitializeLuaObject(lua);
	g_ClientEntityFactories->GetClassName(typeid(*this),&m_class);

	 // prosper TODO
	//m_modelComponent->AddCallback(pragma::EComponentModelCallbackEvent::OnModelChanged,FunctionCallback<void,std::reference_wrapper<std::shared_ptr<Model>>>::Create([this](std::reference_wrapper<std::shared_ptr<Model>> mdl) {
		//m_softBodyBuffers = nullptr; // prosper TODO
	//}));
}

void CBaseEntity::DoSpawn()
{
	BaseEntity::DoSpawn();
	c_game->SpawnEntity(this);
}

Bool CBaseEntity::ReceiveNetEvent(UInt32 eventId,NetPacket &p)
{
	for(auto &pComponent : GetComponents())
	{
		auto *pNetComponent = dynamic_cast<pragma::CBaseNetComponent*>(pComponent.get());
		if(pNetComponent == nullptr)
			continue;
		if(pNetComponent->ReceiveNetEvent(eventId,p))
			return true;
	}
	Con::ccl<<"WARNING: Unhandled net event '"<<eventId<<"' for entity "<<GetClass()<<Con::endl;
	return false;
}

void CBaseEntity::ReceiveData(NetPacket &packet)
{
	m_uniqueIndex = packet->Read<uint64_t>();
	m_spawnFlags = packet->Read<uint32_t>();

	auto &componentManager = static_cast<pragma::CEntityComponentManager&>(c_game->GetEntityComponentManager());
	auto &componentTypes = componentManager.GetRegisteredComponentTypes();
	auto &svComponentToClComponentTable = componentManager.GetServerComponentIdToClientComponentIdTable();
	auto numComponents = packet->Read<uint8_t>();
	for(auto i=decltype(numComponents){0u};i<numComponents;++i)
	{
		auto svId = packet->Read<pragma::ComponentId>();
		auto componentSize = packet->Read<uint8_t>();
		auto offset = packet->GetOffset();
		if(svId < svComponentToClComponentTable.size() && svComponentToClComponentTable.at(svId) != pragma::CEntityComponentManager::INVALID_COMPONENT)
		{
			auto clId = svComponentToClComponentTable.at(svId);
			if(clId >= componentTypes.size())
				throw std::runtime_error("Invalid client component type index " +std::to_string(clId) +"!");
			auto *componentInfo = componentManager.GetComponentInfo(clId);
			if(componentInfo == nullptr)
				throw std::invalid_argument("Invalid component id " +std::to_string(clId) +"!");
			else
			{
				auto component = FindComponent(componentInfo->name);
				if(component.expired())
				{
					// The client doesn't know about component, so we'll add it here.
					// This should usually never happen, except for cases where components
					// have been added the entity, of which the entity's implementation doesn't know about.
					// (This can be the case for map entities for instance.)
					component = AddComponent(clId);
				}
				if(component.expired() == false)
				{
					auto *netComponent = dynamic_cast<pragma::CBaseNetComponent*>(component.get());
					if(netComponent != nullptr)
						netComponent->ReceiveData(packet);
				}
				else
				{
					Con::cwar<<"WARNING: Net data for entity ";
					print(Con::cout);
					Con::cwar<<" contained component data for component '"<<componentInfo->name<<"' which hasn't been attached to the entity clientside! Skipping..."<<Con::endl;
				}
			}
		}
		else
		{
			Con::cwar<<"WARNING: Net data for entity ";
			print(Con::cout);
			Con::cwar<<" contained component data for component type with non-existing clientside representation! Skipping..."<<Con::endl;
		}
		packet->SetOffset(offset +componentSize);
	}
}

void CBaseEntity::ReceiveSnapshotData(NetPacket&) {}

void CBaseEntity::EraseFunction(int function)
{
	Game *game = client->GetGameState();
	lua_removereference(game->GetLuaState(),function);
}

void CBaseEntity::OnRemove()
{
	auto mdlComponent = GetModelComponent();
	if(mdlComponent.valid())
		mdlComponent->SetModel(std::shared_ptr<Model>(nullptr)); // Make sure to clear all clientside model mesh references
	BaseEntity::OnRemove();
}

void CBaseEntity::Remove()
{
	BaseEntity::Remove();
	Game *game = client->GetGameState();
	game->RemoveEntity(this);
}

NetworkState *CBaseEntity::GetNetworkState() const {return client;}

bool CBaseEntity::IsClientsideOnly() const {return (GetIndex() == 0) ? true : false;}

bool CBaseEntity::IsNetworkLocal() const {return IsClientsideOnly();}

void CBaseEntity::SendNetEventTCP(UInt32 eventId) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	NetPacket p;
	SendNetEventTCP(eventId,p);
}
void CBaseEntity::SendNetEventTCP(UInt32 eventId,NetPacket &data) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	nwm::write_entity(data,this);
	data->Write<UInt32>(eventId);
	client->SendPacket("ent_event",data,pragma::networking::Protocol::SlowReliable);
}
void CBaseEntity::SendNetEventUDP(UInt32 eventId) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	NetPacket p;
	SendNetEventUDP(eventId,p);
}
void CBaseEntity::SendNetEventUDP(UInt32 eventId,NetPacket &data) const
{
	if(IsClientsideOnly() || !IsSpawned())
		return;
	nwm::write_entity(data,this);
	data->Write<UInt32>(eventId);
	client->SendPacket("ent_event",data,pragma::networking::Protocol::FastUnreliable);
}
util::WeakHandle<pragma::BaseModelComponent> CBaseEntity::GetModelComponent() const
{
	auto pComponent = GetComponent<pragma::CModelComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BaseModelComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BaseModelComponent>{};
}
util::WeakHandle<pragma::BaseAnimatedComponent> CBaseEntity::GetAnimatedComponent() const
{
	auto pComponent = GetComponent<pragma::CAnimatedComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BaseAnimatedComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BaseAnimatedComponent>{};
}
util::WeakHandle<pragma::BaseWeaponComponent> CBaseEntity::GetWeaponComponent() const
{
	auto pComponent = GetComponent<pragma::CWeaponComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BaseWeaponComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BaseWeaponComponent>{};
}
util::WeakHandle<pragma::BaseVehicleComponent> CBaseEntity::GetVehicleComponent() const
{
	auto pComponent = GetComponent<pragma::CVehicleComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BaseVehicleComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BaseVehicleComponent>{};
}
util::WeakHandle<pragma::BaseAIComponent> CBaseEntity::GetAIComponent() const
{
	auto pComponent = GetComponent<pragma::CAIComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BaseAIComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BaseAIComponent>{};
}
util::WeakHandle<pragma::BaseCharacterComponent> CBaseEntity::GetCharacterComponent() const
{
	auto pComponent = GetComponent<pragma::CCharacterComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BaseCharacterComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BaseCharacterComponent>{};
}
util::WeakHandle<pragma::BasePlayerComponent> CBaseEntity::GetPlayerComponent() const
{
	auto pComponent = GetComponent<pragma::CPlayerComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BasePlayerComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BasePlayerComponent>{};
}
util::WeakHandle<pragma::BasePhysicsComponent> CBaseEntity::GetPhysicsComponent() const
{
	auto pComponent = GetComponent<pragma::CPhysicsComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BasePhysicsComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BasePhysicsComponent>{};
}
util::WeakHandle<pragma::BaseTimeScaleComponent> CBaseEntity::GetTimeScaleComponent() const
{
	auto pComponent = GetComponent<pragma::CTimeScaleComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BaseTimeScaleComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BaseTimeScaleComponent>{};
}
util::WeakHandle<pragma::BaseNameComponent> CBaseEntity::GetNameComponent() const
{
	auto pComponent = GetComponent<pragma::CNameComponent>();
	return pComponent.valid() ? std::static_pointer_cast<pragma::BaseNameComponent>(pComponent->shared_from_this()) : util::WeakHandle<pragma::BaseNameComponent>{};
}
bool CBaseEntity::IsCharacter() const {return HasComponent<pragma::CCharacterComponent>();}
bool CBaseEntity::IsPlayer() const {return HasComponent<pragma::CPlayerComponent>();}
bool CBaseEntity::IsWeapon() const {return HasComponent<pragma::CWeaponComponent>();}
bool CBaseEntity::IsVehicle() const {return HasComponent<pragma::CVehicleComponent>();}
bool CBaseEntity::IsNPC() const {return HasComponent<pragma::CAIComponent>();}
