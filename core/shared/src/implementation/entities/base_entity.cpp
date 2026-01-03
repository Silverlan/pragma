// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :entities.base_entity;
import :util.global_string_table;

pragma::Game &pragma::ecs::BaseEntity::GetGame() const { return *GetNetworkState()->GetGameState(); }
pragma::ecs::BaseEntity *pragma::ecs::BaseEntity::CreateChild(const std::string &className)
{
	auto &game = GetGame();
	auto *child = game.CreateEntity(className);
	if(!child)
		return nullptr;
	child->SetParent(this);
	return child;
}
void pragma::ecs::BaseEntity::SetEnabled(bool enabled)
{
	auto *toggleC = dynamic_cast<BaseToggleComponent *>(FindComponent("toggle").get());
	if(toggleC == nullptr && enabled == true)
		return;
	if(toggleC == nullptr)
		toggleC = dynamic_cast<BaseToggleComponent *>(AddComponent("toggle").get());
	if(toggleC == nullptr)
		return;
	toggleC->SetTurnedOn(enabled);
}
bool pragma::ecs::BaseEntity::IsEnabled() const
{
	auto isEnabled = true;
	auto *toggleC = dynamic_cast<BaseToggleComponent *>(FindComponent("toggle").get());
	if(toggleC != nullptr)
		isEnabled = toggleC->IsTurnedOn();
	return isEnabled;
}
bool pragma::ecs::BaseEntity::IsDisabled() const
{
	auto isEnabled = true;
	auto *toggleC = dynamic_cast<BaseToggleComponent *>(FindComponent("toggle").get());
	if(toggleC != nullptr)
		isEnabled = toggleC->IsTurnedOn();
	return !isEnabled;
}

std::optional<Color> pragma::ecs::BaseEntity::GetColor() const
{
	auto *colorC = dynamic_cast<BaseColorComponent *>(FindComponent("color").get());
	if(colorC == nullptr)
		return {};
	return colorC->GetColor();
}
void pragma::ecs::BaseEntity::SetColor(const Color &color)
{
	auto *colorC = dynamic_cast<BaseColorComponent *>(AddComponent("color").get());
	if(colorC == nullptr)
		return;
	colorC->SetColor(color);
}
bool pragma::ecs::BaseEntity::IsStatic() const
{
	if(GetAnimatedComponent().valid())
		return false;
	auto *physComponent = GetPhysicsComponent();
	auto type = physComponent ? physComponent->GetPhysicsType() : physics::PhysicsType::None;
	return (type == physics::PhysicsType::None || type == physics::PhysicsType::Static) ? true : false;
}
bool pragma::ecs::BaseEntity::IsDynamic() const { return !IsStatic(); }

Con::c_cout &pragma::ecs::BaseEntity::print(Con::c_cout &os) const
{
	auto *componentManager = GetComponentManager();
	auto pNameComponent = componentManager ? static_cast<BaseNameComponent *>(FindComponent("name").get()) : nullptr;
	os << "Entity[G:" << m_index << "][L:" << GetLocalIndex() << "][C:" << GetClass() << "][N:" << (pNameComponent != nullptr ? pNameComponent->GetName() : "") << "][";
	auto mdlComponent = componentManager ? GetModelComponent() : nullptr;
	auto hMdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		os << "NULL";
	else
		os << hMdl->GetName();
	os << "]";
	return os;
}

std::ostream &pragma::ecs::BaseEntity::print(std::ostream &os) const
{
	auto pNameComponent = static_cast<BaseNameComponent *>(FindComponent("name").get());
	os << "Entity[G:" << m_index << "][L:" << GetLocalIndex() << "][C:" << GetClass() << "][N:" << (pNameComponent != nullptr ? pNameComponent->GetName() : "") << "][";
	auto mdlComponent = GetModelComponent();
	auto hMdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		os << "NULL";
	else
		os << hMdl->GetName();
	os << "]";
	return os;
}

std::string pragma::ecs::BaseEntity::ToString() const
{
	std::stringstream ss;
	const_cast<BaseEntity *>(this)->print(ss);
	return ss.str();
}

pragma::ecs::BaseEntity::BaseEntity() : BaseEntityComponentSystem {}, BaseLuaHandle {}, m_uuid {util::generate_uuid_v4()} {}
pragma::NetEventId pragma::ecs::BaseEntity::FindNetEvent(const std::string &name) const { return GetNetworkState()->GetGameState()->FindNetEvent(name); }

pragma::ecs::BaseEntity::StateFlags pragma::ecs::BaseEntity::GetStateFlags() const { return m_stateFlags; }
void pragma::ecs::BaseEntity::ResetStateChangeFlags() { m_stateFlags &= ~(StateFlags::CollisionBoundsChanged | StateFlags::PositionChanged | StateFlags::RenderBoundsChanged | StateFlags::RotationChanged); }
bool pragma::ecs::BaseEntity::HasStateFlag(StateFlags flag) const { return ((m_stateFlags & flag) == flag) ? true : false; }
void pragma::ecs::BaseEntity::SetStateFlag(StateFlags flag) { m_stateFlags |= flag; }
pragma::BaseEntityComponent *pragma::ecs::BaseEntity::FindComponentMemberIndex(const util::Path &path, ComponentMemberIndex &outMemberIdx)
{
	auto hComponent = FindComponent(std::string {path.GetFront()});
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
void pragma::ecs::BaseEntity::OnRemove()
{
	BaseEntityComponentSystem::OnRemove();
	BroadcastEvent(baseEntity::EVENT_ON_REMOVE);
	ClearComponents();
	InvalidateHandle();

	auto &uuidMap = GetNetworkState()->GetGameState()->GetEntityUuidMap();
	auto it = uuidMap.find(util::get_uuid_hash(m_uuid));
	if(it != uuidMap.end())
		uuidMap.erase(it);
}

void pragma::ecs::BaseEntity::Construct(unsigned int idx)
{
	m_index = idx;
	Initialize();
}

bool pragma::ecs::BaseEntity::IsMapEntity() const
{
	auto *mapComponent = static_cast<MapComponent *>(FindComponent("map").get());
	return mapComponent != nullptr && mapComponent->GetMapIndex() != 0;
}

EntityHandle pragma::ecs::BaseEntity::GetHandle() const { return BaseLuaHandle::GetHandle<BaseEntity>(); }

void pragma::ecs::BaseEntity::RemoveEntityOnRemoval(BaseEntity *ent, Bool bRemove) { RemoveEntityOnRemoval(ent->GetHandle(), bRemove); }
void pragma::ecs::BaseEntity::RemoveEntityOnRemoval(const EntityHandle &hEnt, Bool bRemove)
{
	if(!hEnt.valid())
		return;
	auto lifelineLinkC = AddComponent<LifelineLinkComponent>();
	if(lifelineLinkC.expired())
		return;
	lifelineLinkC->RemoveEntityOnRemoval(hEnt, bRemove);
}
void pragma::ecs::BaseEntity::SetKeyValue(std::string key, std::string val)
{
	string::to_lower(key);
	CEKeyValueData inputData {key, val};
	if(BroadcastEvent(baseEntity::EVENT_HANDLE_KEY_VALUE, inputData) == util::EventReply::Handled)
		return;
	if(key == "spawnflags")
		m_spawnFlags = util::to_int(val);
	else if(key == "uuid")
		SetUuid(util::uuid_string_to_bytes(val));
}
void pragma::ecs::BaseEntity::SetSpawnFlags(uint32_t spawnFlags) { m_spawnFlags = spawnFlags; }
unsigned int pragma::ecs::BaseEntity::GetSpawnFlags() const { return m_spawnFlags; }

void pragma::ecs::BaseEntity::MarkForSnapshot(bool b)
{
	if(b)
		m_stateFlags |= StateFlags::SnapshotUpdateRequired;
	else
		m_stateFlags &= ~StateFlags::SnapshotUpdateRequired;
}
bool pragma::ecs::BaseEntity::IsMarkedForSnapshot() const { return (m_stateFlags & StateFlags::SnapshotUpdateRequired) != StateFlags::None; }

lua::State *pragma::ecs::BaseEntity::GetLuaState() const
{
	auto *nw = GetNetworkState();
	auto *game = nw->GetGameState();
	return game->GetLuaState();
}

pragma::NetEventId pragma::ecs::BaseEntity::SetupNetEvent(const std::string &name) const { return GetNetworkState()->GetGameState()->SetupNetEvent(name); }

void pragma::ecs::BaseEntity::RegisterEvents(EntityComponentManager &componentManager)
{
	std::type_index typeIndex = typeid(BaseEntity);
	baseEntity::EVENT_HANDLE_KEY_VALUE = componentManager.RegisterEvent("HANDLE_KEY_VALUE", typeIndex);
	baseEntity::EVENT_ON_SPAWN = componentManager.RegisterEvent("ON_SPAWN", typeIndex);
	baseEntity::EVENT_ON_POST_SPAWN = componentManager.RegisterEvent("ON_POST_SPAWN", typeIndex);
	baseEntity::EVENT_ON_REMOVE = componentManager.RegisterEvent("ON_REMOVE", typeIndex);
}

void pragma::ecs::BaseEntity::SetUuid(const util::Uuid &uuid)
{
	auto &uuidMap = GetNetworkState()->GetGameState()->GetEntityUuidMap();
	auto it = uuidMap.find(util::get_uuid_hash(m_uuid));
	if(it != uuidMap.end())
		uuidMap.erase(it);
	m_uuid = uuid;
	uuidMap[util::get_uuid_hash(m_uuid)] = this;
}

void pragma::ecs::BaseEntity::Initialize()
{
	auto &uuidMap = GetNetworkState()->GetGameState()->GetEntityUuidMap();
	uuidMap[util::get_uuid_hash(m_uuid)] = this;

	InitializeLuaObject(GetLuaState());

	BaseEntityComponentSystem::Initialize(*this, GetNetworkState()->GetGameState()->GetEntityComponentManager());
	AddComponent("entity");
}

pragma::GString pragma::ecs::BaseEntity::GetClass() const { return m_className; }

void pragma::ecs::BaseEntity::SetPose(const math::Transform &outTransform, CoordinateSpace space)
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return;
	SetPosition(outTransform.GetOrigin());
	SetRotation(outTransform.GetRotation());
}
void pragma::ecs::BaseEntity::SetPose(const math::ScaledTransform &outTransform, CoordinateSpace space)
{
	SetPosition(outTransform.GetOrigin());
	SetRotation(outTransform.GetRotation());
	SetScale(outTransform.GetScale());
}
pragma::math::ScaledTransform pragma::ecs::BaseEntity::GetPose(CoordinateSpace space) const
{
	switch(space) {
	case CoordinateSpace::Local:
		{
			if(!m_childComponent)
				return GetPose();
			auto *parent = m_childComponent->GetParentEntity();
			if(!parent)
				return GetPose();
			return parent->GetPose().GetInverse() * GetPose();
		}
	case CoordinateSpace::World:
	case CoordinateSpace::Object:
	default:
		return GetPose();
	}
	return {};
}
const pragma::math::ScaledTransform &pragma::ecs::BaseEntity::GetPose() const
{
	if(!m_transformComponent) {
		static math::ScaledTransform defaultPose {};
		return defaultPose;
	}
	return m_transformComponent->GetPose();
}
Vector3 pragma::ecs::BaseEntity::GetPosition(CoordinateSpace space) const
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return uvec::PRM_ORIGIN;
	return trComponent->GetPosition(space);
}
const Vector3 &pragma::ecs::BaseEntity::GetPosition() const
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return uvec::PRM_ORIGIN;
	return trComponent->GetPosition();
}
void pragma::ecs::BaseEntity::SetPosition(const Vector3 &pos, CoordinateSpace space)
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return;
	trComponent->SetPosition(pos, space);
}
Vector3 pragma::ecs::BaseEntity::GetCenter() const
{
	auto physComponent = GetPhysicsComponent();
	if(!physComponent)
		return GetPosition();
	return physComponent->GetCenter();
}
Quat pragma::ecs::BaseEntity::GetRotation(CoordinateSpace space) const
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return uquat::PRM_UNIT;
	return trComponent->GetRotation(space);
}
const Quat &pragma::ecs::BaseEntity::GetRotation() const
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return uquat::PRM_UNIT;
	return trComponent->GetRotation();
}
void pragma::ecs::BaseEntity::SetRotation(const Quat &rot, CoordinateSpace space)
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return;
	trComponent->SetRotation(rot, space);
}
Vector3 pragma::ecs::BaseEntity::GetScale(CoordinateSpace space) const
{
	auto trComponent = GetTransformComponent();
	if(!trComponent) {
		static Vector3 defaultScale {1.f, 1.f, 1.f};
		return defaultScale;
	}
	return trComponent->GetScale(space);
}
const Vector3 &pragma::ecs::BaseEntity::GetScale() const
{
	auto trComponent = GetTransformComponent();
	if(!trComponent) {
		static Vector3 defaultScale {1.f, 1.f, 1.f};
		return defaultScale;
	}
	return trComponent->GetScale();
}
void pragma::ecs::BaseEntity::SetScale(const Vector3 &scale, CoordinateSpace space)
{
	auto trComponent = GetTransformComponent();
	if(!trComponent)
		return;
	trComponent->SetScale(scale, space);
}

void pragma::ecs::BaseEntity::DoSpawn()
{
	// Flag has to be set before events are triggered, in case
	// one of the events relies (directly or indirectly) on :IsSpawned
	m_stateFlags |= StateFlags::Spawned;
	BroadcastEvent(baseEntity::EVENT_ON_SPAWN);
}

void pragma::ecs::BaseEntity::Spawn()
{
	if(IsSpawned())
		return;
	math::set_flag(m_stateFlags, StateFlags::IsSpawning);
	DoSpawn();
	math::set_flag(m_stateFlags, StateFlags::IsSpawning, false);
	OnPostSpawn();
}

void pragma::ecs::BaseEntity::OnSpawn() {}

void pragma::ecs::BaseEntity::OnPostSpawn() { BroadcastEvent(baseEntity::EVENT_ON_POST_SPAWN); }

bool pragma::ecs::BaseEntity::IsSpawned() const { return (m_stateFlags & StateFlags::Spawned) != StateFlags::None && !IsRemoved(); }

bool pragma::ecs::BaseEntity::IsInert() const
{
	if(GetAnimatedComponent().valid())
		return false;
	if(IsStatic() == true)
		return true;
	auto pPhysComponent = GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	return (phys != nullptr && phys->IsSleeping()) ? true : false;
}

unsigned int pragma::ecs::BaseEntity::GetIndex() const { return m_index; }
uint32_t pragma::ecs::BaseEntity::GetLocalIndex() const { return GetIndex(); }

bool pragma::ecs::BaseEntity::IsWorld() const { return math::is_flag_set(m_stateFlags, StateFlags::HasWorldComponent); }
bool pragma::ecs::BaseEntity::IsScripted() const { return false; }

void pragma::ecs::BaseEntity::PrecacheModels() {}

pragma::BaseTransformComponent *pragma::ecs::BaseEntity::GetTransformComponent() const { return m_transformComponent; }
pragma::BasePhysicsComponent *pragma::ecs::BaseEntity::GetPhysicsComponent() const { return m_physicsComponent; }
pragma::BaseModelComponent *pragma::ecs::BaseEntity::GetModelComponent() const { return m_modelComponent; }
pragma::BaseGenericComponent *pragma::ecs::BaseEntity::GetGenericComponent() const { return m_genericComponent; }
pragma::BaseChildComponent *pragma::ecs::BaseEntity::GetChildComponent() const { return m_childComponent; }

bool pragma::ecs::BaseEntity::IsRemoved() const { return math::is_flag_set(m_stateFlags, StateFlags::Removed); }
void pragma::ecs::BaseEntity::Remove() {}
void pragma::ecs::BaseEntity::RemoveSafely() { GetNetworkState()->GetGameState()->ScheduleEntityForRemoval(*this); }

CallbackHandle pragma::ecs::BaseEntity::CallOnRemove(const CallbackHandle &hCallback)
{
	auto *pComponent = static_cast<BaseGenericComponent *>(FindComponent("entity").get());
	if(pComponent != nullptr)
		pComponent->BindEventUnhandled(baseEntity::EVENT_ON_REMOVE, hCallback);
	return hCallback;
}

////////////////////////////////////

Con::c_cout &operator<<(Con::c_cout &os, const EntityHandle &ent)
{
	if(!ent.valid())
		os << "NULL";
	else
		os << *ent.get();
	return os;
}

std::ostream &operator<<(std::ostream &os, const EntityHandle ent)
{
	if(!ent.valid())
		os << "NULL";
	else
		os << *ent.get();
	return os;
}

const char *pragma::ents::register_class_name(const std::string &className) { return register_global_string(className); }
