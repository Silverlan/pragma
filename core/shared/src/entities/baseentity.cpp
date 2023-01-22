/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
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
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/model/model.h"
#include "pragma/entities/baseentity_events.hpp"

void BaseEntity::SetEnabled(bool enabled)
{
	auto *toggleC = dynamic_cast<pragma::BaseToggleComponent*>(FindComponent("toggle").get());
	if(toggleC == nullptr && enabled == true)
		return;
	if(toggleC == nullptr)
		toggleC = dynamic_cast<pragma::BaseToggleComponent*>(AddComponent("toggle").get());
	if(toggleC == nullptr)
		return;
	toggleC->SetTurnedOn(enabled);
}
bool BaseEntity::IsEnabled() const
{
	auto isEnabled = true;
	auto *toggleC = dynamic_cast<pragma::BaseToggleComponent*>(FindComponent("toggle").get());
	if(toggleC != nullptr)
		isEnabled = toggleC->IsTurnedOn();
	return isEnabled;
}
bool BaseEntity::IsDisabled() const
{
	auto isEnabled = true;
	auto *toggleC = dynamic_cast<pragma::BaseToggleComponent*>(FindComponent("toggle").get());
	if(toggleC != nullptr)
		isEnabled = toggleC->IsTurnedOn();
	return !isEnabled;
}

std::optional<Color> BaseEntity::GetColor() const
{
	auto *colorC = dynamic_cast<pragma::BaseColorComponent*>(FindComponent("color").get());
	if(colorC == nullptr)
		return {};
	return colorC->GetColor();
}
void BaseEntity::SetColor(const Color &color)
{
	auto *colorC = dynamic_cast<pragma::BaseColorComponent*>(AddComponent("color").get());
	if(colorC == nullptr)
		return;
	colorC->SetColor(color);
}
bool BaseEntity::IsStatic() const
{
	if(GetAnimatedComponent().valid())
		return false;
	auto *physComponent = GetPhysicsComponent();
	auto type = physComponent ? physComponent->GetPhysicsType() : PHYSICSTYPE::NONE;
	return (type == PHYSICSTYPE::NONE || type == PHYSICSTYPE::STATIC) ? true : false;
}
bool BaseEntity::IsDynamic() const {return !IsStatic();}

extern DLLNETWORK Engine *engine;
Con::c_cout& BaseEntity::print(Con::c_cout &os)
{
	auto *componentManager = GetComponentManager();
	auto pNameComponent = componentManager ? static_cast<pragma::BaseNameComponent*>(FindComponent("name").get()) : nullptr;
	os<<"Entity[G:"<<m_index<<"][L:"<<GetLocalIndex()<<"][C:"<<GetClass()<<"][N:"<<(pNameComponent != nullptr ? pNameComponent->GetName() : "")<<"][";
	auto mdlComponent = componentManager ? GetModelComponent() : nullptr;
	auto hMdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
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
	os<<"Entity[G:"<<m_index<<"][L:"<<GetLocalIndex()<<"][C:"<<GetClass()<<"][N:"<<(pNameComponent != nullptr ? pNameComponent->GetName() : "")<<"][";
	auto mdlComponent = GetModelComponent();
	auto hMdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		os<<"NULL";
	else
		os<<hMdl->GetName();
	os<<"]";
	return os;
}

std::string BaseEntity::ToString() const
{
	std::stringstream ss;
	const_cast<BaseEntity*>(this)->print(ss);
	return ss.str();
}

pragma::ComponentEventId BaseEntity::EVENT_HANDLE_KEY_VALUE = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId BaseEntity::EVENT_ON_SPAWN = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId BaseEntity::EVENT_ON_POST_SPAWN = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId BaseEntity::EVENT_ON_REMOVE = pragma::INVALID_COMPONENT_ID;
BaseEntity::BaseEntity()
	: pragma::BaseEntityComponentSystem{},pragma::BaseLuaHandle{},m_uuid{util::generate_uuid_v4()}
{}
pragma::NetEventId BaseEntity::FindNetEvent(const std::string &name) const
{
	return GetNetworkState()->GetGameState()->FindNetEvent(name);
}

BaseEntity::StateFlags BaseEntity::GetStateFlags() const {return m_stateFlags;}
void BaseEntity::ResetStateChangeFlags() {m_stateFlags &= ~(StateFlags::CollisionBoundsChanged | StateFlags::PositionChanged | StateFlags::RenderBoundsChanged | StateFlags::RotationChanged);}
bool BaseEntity::HasStateFlag(StateFlags flag) const {return ((m_stateFlags &flag) == flag) ? true : false;}
void BaseEntity::SetStateFlag(StateFlags flag) {m_stateFlags |= flag;}
pragma::BaseEntityComponent *BaseEntity::FindComponentMemberIndex(const util::Path &path,pragma::ComponentMemberIndex &outMemberIdx)
{
	auto hComponent = FindComponent(std::string{path.GetFront()});
	if(!hComponent.valid())
		return nullptr;
	auto memPath = path;
	memPath.PopFront();
	auto memIdx = hComponent->GetMemberIndex(memPath.GetString());
	if(!memIdx.has_value())
		return nullptr;
	outMemberIdx = *memIdx;
	return hComponent.get();
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
	pragma::BaseLuaHandle::InvalidateHandle();

	auto &uuidMap = GetNetworkState()->GetGameState()->GetEntityUuidMap();
	auto it = uuidMap.find(util::get_uuid_hash(m_uuid));
	if(it != uuidMap.end())
		uuidMap.erase(it);
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

EntityHandle BaseEntity::GetHandle() const {return pragma::BaseLuaHandle::GetHandle<BaseEntity>();}

void BaseEntity::RemoveEntityOnRemoval(BaseEntity *ent,Bool bRemove) {RemoveEntityOnRemoval(ent->GetHandle(),bRemove);}
void BaseEntity::RemoveEntityOnRemoval(const EntityHandle &hEnt,Bool bRemove)
{
	if(!hEnt.valid())
		return;
	auto *ent = hEnt.get();
	auto it = std::find_if(m_entsRemove.begin(),m_entsRemove.end(),[ent](EntityHandle &hOther) {
		return (hOther.valid() && hOther.get() == ent) ? true : false;
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
	else if(key == "uuid")
		m_uuid = util::uuid_string_to_bytes(val);
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
	std::type_index typeIndex = typeid(BaseEntity);
	EVENT_HANDLE_KEY_VALUE = componentManager.RegisterEvent("HANDLE_KEY_VALUE",typeIndex);
	EVENT_ON_SPAWN = componentManager.RegisterEvent("ON_SPAWN",typeIndex);
	EVENT_ON_POST_SPAWN = componentManager.RegisterEvent("ON_POST_SPAWN",typeIndex);
	EVENT_ON_REMOVE = componentManager.RegisterEvent("ON_REMOVE",typeIndex);
}

void BaseEntity::SetUuid(const util::Uuid &uuid)
{
	auto &uuidMap = GetNetworkState()->GetGameState()->GetEntityUuidMap();
	auto it = uuidMap.find(util::get_uuid_hash(m_uuid));
	if(it != uuidMap.end())
		uuidMap.erase(it);
	m_uuid = uuid;
	uuidMap[util::get_uuid_hash(m_uuid)] = this;
}

void BaseEntity::Initialize()
{
	auto &uuidMap = GetNetworkState()->GetGameState()->GetEntityUuidMap();
	uuidMap[util::get_uuid_hash(m_uuid)] = this;

	InitializeLuaObject(GetLuaState());

	BaseEntityComponentSystem::Initialize(*this,GetNetworkState()->GetGameState()->GetEntityComponentManager());
	AddComponent("entity");
}

std::string BaseEntity::GetClass() const {return m_class;}

void BaseEntity::SetPose(const umath::Transform &outTransform)
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return;
	SetPosition(outTransform.GetOrigin());
	SetRotation(outTransform.GetRotation());
}
void BaseEntity::SetPose(const umath::ScaledTransform &outTransform)
{
	SetPosition(outTransform.GetOrigin());
	SetRotation(outTransform.GetRotation());
	SetScale(outTransform.GetScale());
}
const umath::ScaledTransform &BaseEntity::GetPose() const
{
	if(!m_transformComponent)
	{
		static umath::ScaledTransform defaultPose {};
		return defaultPose;
	}
	return m_transformComponent->GetPose();
}
const Vector3 &BaseEntity::GetPosition() const
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return uvec::ORIGIN;
	return trComponent->GetPosition();
}
void BaseEntity::SetPosition(const Vector3 &pos)
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return;
	trComponent->SetPosition(pos);
}
Vector3 BaseEntity::GetCenter() const
{
	auto physComponent = GetPhysicsComponent();
	if(!physComponent)
		return GetPosition();
	return physComponent->GetCenter();
}
const Quat &BaseEntity::GetRotation() const
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return uquat::UNIT;
	return trComponent->GetRotation();
}
void BaseEntity::SetRotation(const Quat &rot)
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return;
	trComponent->SetRotation(rot);
}
const Vector3 &BaseEntity::GetScale() const
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
	{
		static Vector3 defaultScale {1.f,1.f,1.f};
		return defaultScale;
	}
	return trComponent->GetScale();
}
void BaseEntity::SetScale(const Vector3 &scale)
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return;
	trComponent->SetScale(scale);
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

bool BaseEntity::IsSpawned() const {return (m_stateFlags &StateFlags::Spawned) != StateFlags::None && !IsRemoved();}

bool BaseEntity::IsInert() const
{
	if(GetAnimatedComponent().valid())
		return false;
	if(IsStatic() == true)
		return true;
	auto pPhysComponent = GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	return (phys != nullptr && phys->IsSleeping()) ? true : false;
}

unsigned int BaseEntity::GetIndex() const {return m_index;}
uint32_t BaseEntity::GetLocalIndex() const {return GetIndex();}

bool BaseEntity::IsWorld() const {return umath::is_flag_set(m_stateFlags,StateFlags::HasWorldComponent);}
bool BaseEntity::IsScripted() const {return false;}

void BaseEntity::PrecacheModels() {}

pragma::BaseTransformComponent *BaseEntity::GetTransformComponent() const {return m_transformComponent;}
pragma::BasePhysicsComponent *BaseEntity::GetPhysicsComponent() const {return m_physicsComponent;}
pragma::BaseModelComponent *BaseEntity::GetModelComponent() const {return m_modelComponent;}
pragma::BaseGenericComponent *BaseEntity::GetGenericComponent() const {return m_genericComponent;}

bool BaseEntity::IsRemoved() const {return umath::is_flag_set(m_stateFlags,StateFlags::Removed);}
void BaseEntity::Remove() {}
void BaseEntity::RemoveSafely() {GetNetworkState()->GetGameState()->ScheduleEntityForRemoval(*this);}

////////////////////////////////////

DLLNETWORK bool operator==(const EntityHandle &a,const EntityHandle &b) {return a.get() == b.get();}

DLLNETWORK Con::c_cout& operator<<(Con::c_cout &os,const EntityHandle &ent)
{
	if(!ent.valid())
		os<<"NULL";
	else
		os<<const_cast<BaseEntity&>(*ent.get());
	return os;
}

static std::ostream& operator<<(std::ostream &os,BaseEntity &ent) {return ent.print(os);}

DLLNETWORK std::ostream& operator<<(std::ostream &os,const EntityHandle ent)
{
	if(!ent.valid())
		os<<"NULL";
	else
		os<<const_cast<BaseEntity&>(*ent.get());
	return os;
}
