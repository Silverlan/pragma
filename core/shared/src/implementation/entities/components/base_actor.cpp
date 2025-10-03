// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "algorithm"

#include "sharedutils/util.h"

#include "mathutil/uvec.h"

#include <udm.hpp>
#include "pragma/lua/luaapi.h"

module pragma.shared;

import :entities.components.base_actor;

using namespace pragma;

BaseActorComponent::HitboxData::HitboxData(uint32_t _boneId, const Vector3 &_offset) : boneId(_boneId), offset(_offset) {}

//////////////////

ComponentEventId BaseActorComponent::EVENT_ON_KILLED = INVALID_COMPONENT_ID;
ComponentEventId BaseActorComponent::EVENT_ON_RESPAWN = INVALID_COMPONENT_ID;
ComponentEventId BaseActorComponent::EVENT_ON_DEATH = INVALID_COMPONENT_ID;
void BaseActorComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_KILLED = registerEvent("ON_KILLED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_RESPAWN = registerEvent("ON_RESPAWN", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_DEATH = registerEvent("ON_DEATH", ComponentEventInfo::Type::Broadcast);
}
BaseActorComponent::BaseActorComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_bAlive(true), m_bFrozen(util::BoolProperty::Create(false)) {}

void BaseActorComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		InitializeMoveController();
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnPhysicsInitialized(); });
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnPhysicsDestroyed(); });
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_UPDATED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { PhysicsUpdate(0.0 /* unused */); });

	auto &ent = GetEntity();
	ent.AddComponent("gravity");
	ent.AddComponent("name");
	ent.AddComponent("flammable");
	ent.AddComponent("health");
	auto whObservableComponent = ent.AddComponent("observable");
	if(whObservableComponent.valid()) {
		auto *pObservableComponent = static_cast<BaseObservableComponent *>(whObservableComponent.get());
		pObservableComponent->SetCameraEnabled(pragma::BaseObservableComponent::CameraType::FirstPerson, false);
		pObservableComponent->SetCameraEnabled(pragma::BaseObservableComponent::CameraType::ThirdPerson, true);
	}
	ent.AddComponent("submergible");
	ent.AddComponent("physics");
	ent.AddComponent("transform");
	ent.AddComponent("sound_emitter");
	ent.AddComponent("velocity");
	ent.AddComponent("damageable");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);

	m_netEvSetFrozen = SetupNetEvent("set_frozen");
}

void BaseActorComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["alive"] = m_bAlive;
	udm["frozen"] = **m_bFrozen;
	udm["moveControllerName"] = m_moveControllerName;
	if(m_moveControllerNameY.has_value())
		udm["moveControllerNameY"] = *m_moveControllerNameY;
	udm["moveControllerIndex"] = m_moveController;
	udm["moveControllerIndexY"] = m_moveControllerY;
	auto udmHitboxData = udm.AddArray("hitboxData", m_hitboxData.size());
	for(auto i = decltype(m_hitboxData.size()) {0u}; i < m_hitboxData.size(); ++i) {
		auto &hitboxData = m_hitboxData[i];
		auto udmHitbox = udmHitboxData[i];
		udmHitbox["boneId"] = hitboxData.boneId;
		udmHitbox["offset"] = hitboxData.offset;
	}
}
void BaseActorComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	udm["alive"](m_bAlive);
	udm["frozen"](**m_bFrozen);
	udm["moveControllerName"](m_moveControllerName);
	auto udmMoveControllerNameY = udm["moveControllerNameY"];
	if(udmMoveControllerNameY) {
		std::string name;
		udmMoveControllerNameY(name);
		m_moveControllerNameY = name;
	}
	udm["moveControllerIndex"](m_moveController);
	udm["moveControllerIndexY"](m_moveControllerY);
	auto udmHitboxData = udm["hitboxData"];
	auto numHitboxData = udmHitboxData.GetSize();
	m_hitboxData.resize(numHitboxData);
	for(auto i = decltype(numHitboxData) {0u}; i < numHitboxData; ++i) {
		auto &hitboxData = m_hitboxData[i];
		auto udmHitbox = udmHitboxData[i];
		udmHitbox["boneId"](hitboxData.boneId);
		udmHitbox["offset"](hitboxData.offset);
	}
}

void BaseActorComponent::SetMoveController(const std::string &moveController)
{
	m_moveControllerName = moveController;
	m_moveControllerNameY = {};
	InitializeMoveController();
}
void BaseActorComponent::SetMoveController(const std::string &moveControllerX, const std::string &moveControllerY)
{
	m_moveControllerName = moveControllerX;
	m_moveControllerNameY = moveControllerY;
	InitializeMoveController();
}
int32_t BaseActorComponent::GetMoveController() const { return m_moveController; }
int32_t BaseActorComponent::GetMoveControllerY() const { return m_moveControllerY; }
void BaseActorComponent::InitializeMoveController()
{
	m_moveController = -1;
	m_moveControllerY = -1;
	auto &ent = GetEntity();
	auto &mdl = ent.GetModel();
	if(mdl == nullptr)
		return;
	m_moveController = mdl->LookupBlendController(m_moveControllerName);
	if(m_moveControllerNameY.has_value())
		m_moveControllerY = mdl->LookupBlendController(*m_moveControllerNameY);
}

void BaseActorComponent::UpdateMoveController()
{
	if(m_moveController == -1)
		return;
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	if(animComponent.expired())
		return;
	auto pTrComponent = ent.GetTransformComponent();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	auto vel = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {};
	auto l = uvec::length_sqr(vel);
	if(m_moveControllerY == -1) {
		if(l > 0.f) {
			auto dirMove = uvec::get_normal(vel);
			auto dir = pTrComponent ? pTrComponent->GetForward() : uvec::FORWARD;
			float yawMove = uvec::get_yaw(dirMove);
			float yawDir = uvec::get_yaw(dir);
			float yawOffset = umath::get_angle_difference(yawDir, yawMove);
			yawOffset = umath::normalize_angle(yawOffset, 0);
			//float moveYaw = GetBlendController(blendController);
			//yawOffset = Math::ApproachAngle(moveYaw,yawOffset,1.f);
			animComponent->SetBlendController(m_moveController, CInt32(yawOffset));
		}
		else
			animComponent->SetBlendController(m_moveController, 0.f);
	}
	else {
		if(l > 0.f) {
			auto dirMove = uvec::get_normal(vel);
			auto dir = pTrComponent ? pTrComponent->GetForward() : uvec::FORWARD;
			auto dirRight = pTrComponent ? pTrComponent->GetRight() : uvec::RIGHT;
			auto rotInv = pTrComponent ? pTrComponent->GetRotation() : uquat::identity();
			uquat::inverse(rotInv);
			uvec::rotate(&dirMove, rotInv);
			uvec::rotate(&dir, rotInv);
			uvec::rotate(&dirRight, rotInv);
			// animComponent->SetBlendController(m_moveController,1.0 -(uvec::dot(dirRight,dirMove) +1.0) /2.0); // TODO
			animComponent->SetBlendController(m_moveControllerY, 1.0 - (uvec::dot(dir, dirMove) + 1.0) / 2.0);
		}
		else {
			animComponent->SetBlendController(m_moveController, 0.f);
			animComponent->SetBlendController(m_moveControllerY, 0.f);
		}
	}
}

const util::PBoolProperty &BaseActorComponent::GetFrozenProperty() const { return m_bFrozen; }
bool BaseActorComponent::IsFrozen() const { return *m_bFrozen; }
bool BaseActorComponent::IsAlive() const { return m_bAlive; }
bool BaseActorComponent::IsDead() const { return !IsAlive(); }
void BaseActorComponent::SetFrozen(bool b) { *m_bFrozen = b; }

void BaseActorComponent::Ragdolize()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(!pPhysComponent)
		return;
	auto *phys = pPhysComponent->GetPhysicsObject();
	if(phys != nullptr && pPhysComponent->GetPhysicsType() == PHYSICSTYPE::DYNAMIC)
		return;
	pPhysComponent->SetMoveType(MOVETYPE::PHYSICS);
	phys = pPhysComponent->InitializePhysics(PHYSICSTYPE::DYNAMIC);
	if(phys == nullptr)
		return;
}

void BaseActorComponent::OnDeath(DamageInfo *dmgInfo)
{
	auto evOnDeath = CEOnCharacterKilled {dmgInfo};
	if(BroadcastEvent(EVENT_ON_DEATH, evOnDeath) == util::EventReply::Handled)
		return;

	Ragdolize();
}

void BaseActorComponent::Kill(DamageInfo *dmgInfo)
{
	if(!IsAlive())
		return;
	m_bAlive = false;
	OnDeath(dmgInfo);

	auto evOnKilled = CEOnCharacterKilled {dmgInfo};
	BroadcastEvent(EVENT_ON_KILLED, evOnKilled);
}

void BaseActorComponent::Respawn()
{
	m_bAlive = true;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->SetMoveType(MOVETYPE::WALK);
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent)
		pTrComponent->SetRotation(uquat::identity());
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(pVelComponent.valid()) {
		pVelComponent->SetVelocity({});
		pVelComponent->SetAngularVelocity({});
	}
	BroadcastEvent(EVENT_ON_RESPAWN);
}

void BaseActorComponent::OnPhysicsInitialized()
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return;
	m_hitboxData.clear();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return;
	auto hitboxBones = hMdl->GetHitboxBones();
	auto pTrComponent = ent.GetTransformComponent();
	auto scale = pTrComponent ? pTrComponent->GetScale() : Vector3 {1.f, 1.f, 1.f};

	auto &hitboxes = hMdl->GetHitboxes();
	std::vector<pragma::physics::ICollisionObject *> physHitboxes;
	physHitboxes.reserve(hitboxes.size());
	m_hitboxData.reserve(hitboxes.size());
	for(auto &it : hitboxes) {
		auto &hb = it.second;
		auto min = hb.min * scale;
		auto max = hb.max * scale;
		auto extents = (max - min) * 0.5f;
		auto shape = physEnv->CreateBoxShape(extents, physEnv->GetGenericMaterial());
		if(shape == nullptr)
			continue;
		auto col = physEnv->CreateGhostObject(*shape);
		if(col == nullptr)
			continue;
		col->SetCollisionsEnabled(false); // Disable collisions
		physHitboxes.push_back(col.Get());

		// Physics object origin is in the center of the hitbox bounds, but the origin of the actual hitbox may not be; We need to calculate the offset between them and rotate it later when updating (See BaseActorComponent::UpdateHitboxPhysics)
		auto offset = max - extents; // Offset between bone origin and physics object origin
		m_hitboxData.push_back({it.first, offset});
	}
	m_physHitboxes = PhysObj::Create<PhysObj>(*this, physHitboxes);
	auto collisionMask = ent.IsPlayer() ? CollisionMask::PlayerHitbox : CollisionMask::NPCHitbox;
	m_physHitboxes->SetCollisionFilter(collisionMask, collisionMask); // Required for raytraces
	m_physHitboxes->Spawn();
}
bool BaseActorComponent::FindHitgroup(const pragma::physics::ICollisionObject &phys, HitGroup &hitgroup) const
{
	if(m_physHitboxes == nullptr)
		return false;
	auto &colObjs = m_physHitboxes->GetCollisionObjects();
	auto it = std::find_if(colObjs.begin(), colObjs.end(), [&phys](const util::TSharedHandle<pragma::physics::ICollisionObject> &hPhys) { return (hPhys.Get() == &phys) ? true : false; });
	if(it == colObjs.end())
		return false;
	auto idx = it - colObjs.begin();
	if(idx >= m_hitboxData.size())
		return false;
	auto &hitboxData = m_hitboxData[idx];
	auto &ent = GetEntity();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return false;
	auto &hitboxes = hMdl->GetHitboxes();
	auto itHitbox = hitboxes.find(hitboxData.boneId);
	if(itHitbox == hitboxes.end())
		return false;
	hitgroup = itHitbox->second.group;
	return true;
}
void BaseActorComponent::PhysicsUpdate(double) { UpdateHitboxPhysics(); }
PhysObjHandle BaseActorComponent::GetHitboxPhysicsObject() const { return (m_physHitboxes == nullptr) ? PhysObjHandle {} : m_physHitboxes->GetHandle(); }
void BaseActorComponent::UpdateHitboxPhysics()
{
	if(m_physHitboxes == nullptr)
		return;
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	if(!mdlComponent)
		return;
	auto &colObjs = m_physHitboxes->GetCollisionObjects();
	auto numColObjs = colObjs.size();
	for(auto i = decltype(m_hitboxData.size()) {0}; i < m_hitboxData.size(); ++i) {
		if(i >= numColObjs)
			break;
		auto &hitboxInfo = m_hitboxData[i];
		auto *col = colObjs[i]->GetGhostObject();
		if(col != nullptr) {
			Vector3 min, max, origin;
			Quat rot;
			if(mdlComponent->GetHitboxBounds(hitboxInfo.boneId, min, max, origin, rot) == true) {
				auto offset = hitboxInfo.offset;
				offset = offset * uquat::get_inverse(rot);
				origin += offset;

				col->SetPos(origin);
				col->SetRotation(rot);
			}
		}
	}
}
void BaseActorComponent::OnPhysicsDestroyed()
{
	m_physHitboxes = nullptr;
	m_hitboxData.clear();
}

///////////////

CEOnCharacterKilled::CEOnCharacterKilled(DamageInfo *damageInfo) : damageInfo(damageInfo) {}
void CEOnCharacterKilled::PushArguments(lua_State *l)
{
	if(damageInfo != nullptr)
		Lua::Push<DamageInfo *>(l, damageInfo);
	else
		Lua::PushNil(l);
}
