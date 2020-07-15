/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/physics/physicstypes.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_networked_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/model/model.h"
#include "pragma/entities/baseentity_events.hpp"

DEFINE_BASE_HANDLE(DLLNETWORK,BaseEntity,Entity);

bool BaseEntity::IsStatic() const
{
	if(GetAnimatedComponent().valid())
		return false;
	auto physComponent = GetPhysicsComponent();
	auto type = physComponent.valid() ? physComponent->GetPhysicsType() : PHYSICSTYPE::NONE;
	return (type == PHYSICSTYPE::NONE || type == PHYSICSTYPE::STATIC) ? true : false;
}
bool BaseEntity::IsDynamic() const {return !IsStatic();}

extern DLLENGINE Engine *engine;
Con::c_cout& BaseEntity::print(Con::c_cout &os)
{
	auto *componentManager = GetComponentManager();
	auto pNameComponent = componentManager ? static_cast<pragma::BaseNameComponent*>(FindComponent("name").get()) : nullptr;
	os<<"Entity["<<m_index<<"]["<<GetLocalIndex()<<"]["<<GetClass()<<"]["<<(pNameComponent != nullptr ? pNameComponent->GetName() : "")<<"][";
	auto mdlComponent = componentManager ? GetModelComponent() : util::WeakHandle<pragma::BaseModelComponent>{};
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		os<<"NULL";
	else
		os<<hMdl->GetName();
	os<<"]";
	return os;
}

std::ostream& BaseEntity::print(std::ostream &os)
{
	auto pNameComponent = static_cast<pragma::BaseNameComponent*>(FindComponent("name").get());
	os<<"Entity["<<m_index<<"]["<<GetLocalIndex()<<"]["<<GetClass()<<"]["<<(pNameComponent != nullptr ? pNameComponent->GetName() : "")<<"][";
	auto mdlComponent = GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		os<<"NULL";
	else
		os<<hMdl->GetName();
	os<<"]";
	return os;
}

pragma::ComponentEventId BaseEntity::EVENT_HANDLE_KEY_VALUE = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId BaseEntity::EVENT_ON_SPAWN = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId BaseEntity::EVENT_ON_POST_SPAWN = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId BaseEntity::EVENT_ON_REMOVE = pragma::INVALID_COMPONENT_ID;
BaseEntity::BaseEntity()
	: pragma::BaseEntityComponentSystem(),LuaObj<EntityHandle>()
{}
pragma::NetEventId BaseEntity::FindNetEvent(const std::string &name) const
{
	return GetNetworkState()->GetGameState()->FindNetEvent(name);
}

BaseEntity::StateFlags BaseEntity::GetStateFlags() const {return m_stateFlags;}
void BaseEntity::ResetStateChangeFlags() {m_stateFlags &= ~(StateFlags::CollisionBoundsChanged | StateFlags::PositionChanged | StateFlags::RenderBoundsChanged | StateFlags::RotationChanged);}
bool BaseEntity::HasStateFlag(StateFlags flag) const {return ((m_stateFlags &flag) == flag) ? true : false;}
void BaseEntity::SetStateFlag(StateFlags flag) {m_stateFlags |= flag;}
pragma::ComponentEventId BaseEntity::RegisterComponentEvent(const std::string &name) const
{
	return GetNetworkState()->GetGameState()->GetEntityComponentManager().RegisterEvent(name);
}
pragma::ComponentEventId BaseEntity::GetEventId(const std::string &name) const
{
	return GetNetworkState()->GetGameState()->GetEntityComponentManager().GetEventId(name);
}
void BaseEntity::OnRemove()
{
	for(auto it=m_entsRemove.begin();it!=m_entsRemove.end();++it)
	{
		auto &hEnt = *it;
		if(hEnt.IsValid())
			hEnt->Remove();
	}
	BroadcastEvent(EVENT_ON_REMOVE);
	ClearComponents();
}

void BaseEntity::Construct(unsigned int idx)
{
	m_index = idx;
	Initialize();
}

bool BaseEntity::IsMapEntity() const
{
	auto *mapComponent = static_cast<pragma::MapComponent*>(FindComponent("map").get());
	return mapComponent != nullptr && mapComponent->GetMapIndex() != 0;
}
uint64_t BaseEntity::GetUniqueIndex() const {return m_uniqueIndex;}
void BaseEntity::SetUniqueIndex(uint64_t idx) {m_uniqueIndex = idx;}

void BaseEntity::RemoveEntityOnRemoval(BaseEntity *ent,Bool bRemove) {RemoveEntityOnRemoval(ent->GetHandle(),bRemove);}
void BaseEntity::RemoveEntityOnRemoval(const EntityHandle &hEnt,Bool bRemove)
{
	if(!hEnt.IsValid())
		return;
	auto *ent = hEnt.get();
	auto it = std::find_if(m_entsRemove.begin(),m_entsRemove.end(),[ent](EntityHandle &hOther) {
		return (hOther.IsValid() && hOther.get() == ent) ? true : false;
	});
	if(bRemove == true)
	{
		if(it == m_entsRemove.end())
			m_entsRemove.push_back(hEnt);
		return;
	}
	else if(it == m_entsRemove.end())
		return;
	m_entsRemove.erase(it);
}
void BaseEntity::SetKeyValue(std::string key,std::string val)
{
	ustring::to_lower(key);
	pragma::CEKeyValueData inputData{key,val};
	if(BroadcastEvent(EVENT_HANDLE_KEY_VALUE,inputData) == util::EventReply::Handled)
		return;
	if(key == "spawnflags")
		m_spawnFlags = util::to_int(val);
}
void BaseEntity::SetSpawnFlags(uint32_t spawnFlags) {m_spawnFlags = spawnFlags;}
unsigned int BaseEntity::GetSpawnFlags() const {return m_spawnFlags;}

void BaseEntity::MarkForSnapshot(bool b)
{
	if(b)
		m_stateFlags |= StateFlags::SnapshotUpdateRequired;
	else
		m_stateFlags &= ~StateFlags::SnapshotUpdateRequired;
}
bool BaseEntity::IsMarkedForSnapshot() const {return (m_stateFlags &StateFlags::SnapshotUpdateRequired) != StateFlags::None;}

void BaseEntity::EraseFunction(int) {}

lua_State *BaseEntity::GetLuaState() const
{
	auto *nw = GetNetworkState();
	auto *game = nw->GetGameState();
	return game->GetLuaState();
}

pragma::NetEventId BaseEntity::SetupNetEvent(const std::string &name) const {return GetNetworkState()->GetGameState()->SetupNetEvent(name);}

void BaseEntity::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_HANDLE_KEY_VALUE = componentManager.RegisterEvent("HANDLE_KEY_VALUE");
	EVENT_ON_SPAWN = componentManager.RegisterEvent("ON_SPAWN");
	EVENT_ON_POST_SPAWN = componentManager.RegisterEvent("ON_POST_SPAWN");
	EVENT_ON_REMOVE = componentManager.RegisterEvent("ON_REMOVE");
}

void BaseEntity::Initialize()
{
	InitializeHandle();
	BaseEntityComponentSystem::Initialize(*this,GetNetworkState()->GetGameState()->GetEntityComponentManager());
	AddComponent("entity");
}

std::string BaseEntity::GetClass() const {return m_class;}

void BaseEntity::GetPose(umath::Transform &outTransform) const
{
	outTransform = {GetPosition(),GetRotation()};
}
void BaseEntity::SetPose(const umath::Transform &outTransform)
{
	SetPosition(outTransform.GetOrigin());
	SetRotation(outTransform.GetRotation());
}
void BaseEntity::GetPose(umath::ScaledTransform &outTransform) const
{
	outTransform = {GetPosition(),GetRotation(),GetScale()};
}
void BaseEntity::SetPose(const umath::ScaledTransform &outTransform)
{
	SetPosition(outTransform.GetOrigin());
	SetRotation(outTransform.GetRotation());
	SetScale(outTransform.GetScale());
}
const Vector3 &BaseEntity::GetPosition() const
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
		return uvec::ORIGIN;
	return trComponent->GetPosition();
}
void BaseEntity::SetPosition(const Vector3 &pos)
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
		return;
	trComponent->SetPosition(pos);
}
Vector3 BaseEntity::GetCenter() const
{
	auto physComponent = GetPhysicsComponent();
	if(physComponent.expired())
		return GetPosition();
	return physComponent->GetCenter();
}
const Quat &BaseEntity::GetRotation() const
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
		return uquat::UNIT;
	return trComponent->GetOrientation();
}
void BaseEntity::SetRotation(const Quat &rot)
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
		return;
	trComponent->SetOrientation(rot);
}
const Vector3 &BaseEntity::GetScale() const
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
	{
		static Vector3 defaultScale {1.f,1.f,1.f};
		return defaultScale;
	}
	return trComponent->GetScale();
}
void BaseEntity::SetScale(const Vector3 &scale)
{
	auto trComponent = GetTransformComponent();
	if(trComponent.expired())
		return;
	trComponent->SetScale(scale);
}

void BaseEntity::OnComponentAdded(pragma::BaseEntityComponent &component)
{
	pragma::BaseEntityComponentSystem::OnComponentAdded(component);
	auto *ptrTransformComponent = dynamic_cast<pragma::BaseTransformComponent*>(&component);
	if(ptrTransformComponent != nullptr)
		m_transformComponent = std::static_pointer_cast<pragma::BaseTransformComponent>(ptrTransformComponent->shared_from_this());
}
void BaseEntity::OnComponentRemoved(pragma::BaseEntityComponent &component)
{
	pragma::BaseEntityComponentSystem::OnComponentRemoved(component);
}

void BaseEntity::DoSpawn()
{
	// Flag has to be set before events are triggered, in case
	// one of the events relies (directly or indirectly) on :IsSpawned
	m_stateFlags |= StateFlags::Spawned;
	BroadcastEvent(EVENT_ON_SPAWN);
}

void BaseEntity::Spawn()
{
	if(IsSpawned())
		return;
	DoSpawn();
	OnPostSpawn();
}

void BaseEntity::OnSpawn() {}

void BaseEntity::OnPostSpawn() {BroadcastEvent(EVENT_ON_POST_SPAWN);}

bool BaseEntity::IsSpawned() const {return (m_stateFlags &StateFlags::Spawned) != StateFlags::None;}

bool BaseEntity::IsInert() const
{
	if(GetAnimatedComponent().valid())
		return false;
	if(IsStatic() == true)
		return true;
	auto pPhysComponent = GetPhysicsComponent();
	auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	return (phys != nullptr && phys->IsSleeping()) ? true : false;
}

unsigned int BaseEntity::GetIndex() const {return m_index;}
uint32_t BaseEntity::GetLocalIndex() const {return GetIndex();}

bool BaseEntity::IsWorld() const {return false;}
bool BaseEntity::IsScripted() const {return false;}

void BaseEntity::PrecacheModels() {}

util::WeakHandle<pragma::BaseTransformComponent> BaseEntity::GetTransformComponent() const {return m_transformComponent.expired() ? nullptr : m_transformComponent.lock();}

void BaseEntity::Remove() {}
void BaseEntity::RemoveSafely() {GetNetworkState()->GetGameState()->ScheduleEntityForRemoval(*this);}

void BaseEntity::InitializeHandle() {BaseEntity::InitializeHandle<EntityHandle>();}

////////////////////////////////////

DLLNETWORK bool operator==(const EntityHandle &a,const EntityHandle &b) {return a.get() == b.get();}

DLLNETWORK Con::c_cout& operator<<(Con::c_cout &os,const EntityHandle &ent)
{
	if(!ent.IsValid())
		os<<"NULL";
	else
		os<<*ent.get();
	return os;
}

static std::ostream& operator<<(std::ostream &os,BaseEntity &ent) {return ent.print(os);}

DLLNETWORK std::ostream& operator<<(std::ostream &os,const EntityHandle ent)
{
	if(!ent.IsValid())
		os<<"NULL";
	else
		os<<*ent.get();
	return os;
}
