/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include <pragma/engine.h>
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/baseentity.h"
#include <pragma/game/game.h>
#include "pragma/physics/collisionmasks.h"
#include "pragma/physics/physobj.h"
#include <pragma/physics/movetypes.h>
#include "pragma/physics/raytraces.h"
#include "pragma/game/game_limits.h"
#include "pragma/audio/alsound_type.h"
#include "pragma/math/util_engine_math.hpp"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_sound_emitter_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/movement_component.hpp"
#include "pragma/entities/components/orientation_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/model/model.h"
#include "pragma/entities/baseentity_trace.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/phys_material.hpp"
#include "pragma/physics/controller.hpp"

using namespace pragma;

extern DLLNETWORK Engine *engine;

//////////////////

void BaseCharacterComponent::InitializeController()
{
	//auto &ent = GetEntity();
	//PhysObj *phys = ent.GetPhysicsObject();
	//if(phys == NULL || !phys->IsController())
	//	return;
}

ComponentEventId BaseCharacterComponent::EVENT_ON_FOOT_STEP = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_ON_DEPLOY_WEAPON = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_PLAY_FOOTSTEP_SOUND = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_IS_MOVING = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_HANDLE_VIEW_ROTATION = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_ON_JUMP = INVALID_COMPONENT_ID;
void BaseCharacterComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseActorComponent::RegisterEvents(componentManager, registerEvent);
	EVENT_ON_FOOT_STEP = registerEvent("ON_FOOT_STEP", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_CHARACTER_ORIENTATION_CHANGED = registerEvent("ON_CHARACTER_ORIENTATION_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_DEPLOY_WEAPON = registerEvent("ON_DEPLOY_WEAPON", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_SET_ACTIVE_WEAPON = registerEvent("ON_SET_ACTIVE_WEAPON", ComponentEventInfo::Type::Broadcast);
	EVENT_PLAY_FOOTSTEP_SOUND = registerEvent("PLAY_FOOTSTEP_SOUND", ComponentEventInfo::Type::Broadcast);
	EVENT_IS_MOVING = registerEvent("IS_MOVING", ComponentEventInfo::Type::Explicit);
	EVENT_HANDLE_VIEW_ROTATION = registerEvent("HANDLE_VIEW_ROTATION", ComponentEventInfo::Type::Explicit);
	EVENT_ON_JUMP = registerEvent("ON_JUMP", ComponentEventInfo::Type::Broadcast);
}

BaseCharacterComponent::BaseCharacterComponent(BaseEntity &ent) : BaseActorComponent(ent), m_slopeLimit(util::FloatProperty::Create(CFloat(umath::cos(umath::deg_to_rad(45.0f))))), m_stepOffset(util::FloatProperty::Create(2.f)), m_jumpPower(util::FloatProperty::Create(0.f)) {}

const util::PFloatProperty &BaseCharacterComponent::GetSlopeLimitProperty() const { return m_slopeLimit; }
const util::PFloatProperty &BaseCharacterComponent::GetStepOffsetProperty() const { return m_stepOffset; }

void BaseCharacterComponent::InitializePhysObj(PhysObj *)
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->AddCollisionFilter(CollisionMask::Character);
}

void BaseCharacterComponent::Initialize()
{
	BaseActorComponent::Initialize();
	m_netEvSetActiveWeapon = SetupNetEvent("set_active_weapon");
	m_netEvSetAmmoCount = SetupNetEvent("set_ammo_count");

	BindEvent(BaseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { return HandleAnimationEvent(static_cast<CEHandleAnimationEvent &>(evData.get()).animationEvent) ? util::EventReply::Handled : util::EventReply::Unhandled; });
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto t = GetEntity().GetNetworkState()->GetGameState()->CurTime();
		if(t >= m_tDetachFromGround)
			return;
		auto &ent = GetEntity();
		auto whPhysComponent = ent.GetPhysicsComponent();
		auto *phys = whPhysComponent ? whPhysComponent->GetPhysicsObject() : nullptr;
		if(phys == nullptr || phys->IsController() == false)
			return;
		auto *physController = static_cast<ControllerPhysObj *>(phys);
		auto contactNormal = physController->GetController()->GetGroundTouchNormal();
		if(contactNormal.has_value() == false)
			return;
		auto whVelComponent = ent.GetComponent<pragma::VelocityComponent>();
		if(whVelComponent.valid()) {
			auto vel = whVelComponent->GetVelocity();
			uvec::normalize(&vel);
			auto dot = uvec::dot(*contactNormal, vel);
			if(dot <= 0.01f) {
				// Velocity leads towards the ground; This probably means we should allow attaching to the ground again.
				m_tDetachFromGround = 0.f;
				return;
			}
		}
	});
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateNeckControllers(); });
	auto &ent = GetEntity();
	ent.AddComponent("sound_emitter");
	ent.AddComponent("physics");
	ent.AddComponent("movement");
	ent.AddComponent("orientation");

	SetTickPolicy(TickPolicy::WhenVisible);
}

float BaseCharacterComponent::GetSlopeLimit() const { return *m_slopeLimit; }
void BaseCharacterComponent::SetSlopeLimit(float limit)
{
	*m_slopeLimit = limit;

	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys != NULL && phys->IsController()) {
		ControllerPhysObj *physController = static_cast<ControllerPhysObj *>(phys);
		physController->SetSlopeLimit(limit);
	}
}
float BaseCharacterComponent::GetStepOffset() const { return *m_stepOffset; }
void BaseCharacterComponent::SetStepOffset(float offset)
{
	*m_stepOffset = offset;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys != NULL && phys->IsController()) {
		ControllerPhysObj *physController = static_cast<ControllerPhysObj *>(phys);
		physController->SetStepOffset(offset);
	}
}

const Quat &BaseCharacterComponent::GetViewOrientation() const { return m_angView; }

float BaseCharacterComponent::GetTurnSpeed() const { return m_turnSpeed; }
void BaseCharacterComponent::SetTurnSpeed(float speed) { m_turnSpeed = speed; }

void BaseCharacterComponent::SetNeckControllers(const std::string &yawController, const std::string &pitchController)
{
	m_yawControllerName = yawController;
	m_pitchControllerName = pitchController;
	UpdateNeckControllers();
}

int32_t BaseCharacterComponent::GetNeckYawBlendController() const { return m_yawController; }
int32_t BaseCharacterComponent::GetNeckPitchBlendController() const { return m_pitchController; }

void BaseCharacterComponent::UpdateNeckControllers()
{
	m_yawController = -1;
	m_pitchController = -1;
	auto &ent = GetEntity();
	auto &hMdl = GetEntity().GetModel();
	if(hMdl == nullptr)
		return;
	m_yawController = hMdl->LookupBlendController(m_yawControllerName);
	m_pitchController = hMdl->LookupBlendController(m_pitchControllerName);
}

void BaseCharacterComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(MovementComponent))
		m_movementComponent = &static_cast<MovementComponent &>(component);
	else if(typeid(component) == typeid(OrientationComponent))
		m_orientationComponent = &static_cast<OrientationComponent &>(component);
}
void BaseCharacterComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(MovementComponent))
		m_movementComponent = nullptr;
	else if(typeid(component) == typeid(OrientationComponent))
		m_orientationComponent = nullptr;
}

void BaseCharacterComponent::NormalizeViewOrientation(Quat &rot)
{
	auto rotRel = GetOrientationAxesRotation();
	auto rotDst = rotRel * rot;
	auto ang = EulerAngles {rotDst};
	ang.r = 0.f;
	Quat rotCurNoRoll;
	if(ang.p < -135.f || ang.p > 135.f)
		rotCurNoRoll = uquat::create(ang) * uquat::create(EulerAngles(0.f, 0.f, 180.f));
	else {
		if(ang.p < -90.f)
			ang.p = -90.f;
		else if(ang.p > 90.f)
			ang.p = 90.f;
		rotCurNoRoll = uquat::create(ang);
	}
	rot = uquat::get_inverse(rotRel) * rotCurNoRoll;
}

const Quat &BaseCharacterComponent::NormalizeViewOrientation()
{
	auto rot = GetViewOrientation();
	NormalizeViewOrientation(rot);
	SetViewOrientation(rot);
	return GetViewOrientation();
}

void BaseCharacterComponent::SetViewOrientation(const Quat &orientation)
{
	/*if(
		fabsf(orientation.w -m_angView.w) <= ENT_EPSILON &&
		fabsf(orientation.x -m_angView.x) <= ENT_EPSILON &&
		fabsf(orientation.y -m_angView.y) <= ENT_EPSILON &&
		fabsf(orientation.z -m_angView.z) <= ENT_EPSILON
	)
		return;*/
	m_angView.w = orientation.w;
	m_angView.x = orientation.x;
	m_angView.y = orientation.y;
	m_angView.z = orientation.z;

	CEViewRotation evData {orientation};
	if(InvokeEventCallbacks(EVENT_HANDLE_VIEW_ROTATION, evData) == util::EventReply::Handled)
		return;

	auto &ent = GetEntity();
	if(ent.IsNPC())
		return; // NPCs are handled separately (See BaseNPC::LookAtStep).
	// TODO: This should probably go in player-specific code
	Quat rotRef = GetOrientationAxesRotation();
	auto rot = rotRef * orientation;
	EulerAngles angView(rot);
	auto animComponent = ent.GetAnimatedComponent();
	auto &hMdl = ent.GetModel();
	if(animComponent.valid() && hMdl != nullptr) {
		auto pTrComponent = ent.GetTransformComponent();
		auto rotCur = pTrComponent ? pTrComponent->GetRotation() : uquat::identity();
		auto angCur = EulerAngles(rotRef * rotCur);
		float pitchDelta = umath::get_angle_difference(angView.p, angCur.p);
		if(m_pitchController != -1)
			animComponent->SetBlendController(m_pitchController, pitchDelta);
		BlendController *yaw = hMdl->GetBlendController(m_yawController);
		if(yaw != NULL) {
			float yawDelta = umath::get_angle_difference(angView.y, angCur.y);
			if(yawDelta >= yaw->min && yawDelta <= yaw->max) {
				animComponent->SetBlendController(m_yawController, yawDelta);
				m_turnYaw = nullptr;
				return;
			}
		}
	}
	if(m_turnYaw == nullptr)
		m_turnYaw = std::make_unique<float>(angView.y);
	else
		*m_turnYaw = angView.y;
}

util::EventReply BaseCharacterComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseTransformComponent::EVENT_ON_TELEPORT) {
		auto &te = static_cast<CETeleport &>(evData);
		auto rot = GetViewOrientation();
		rot = te.deltaPose * rot;
		auto plC = GetEntity().GetPlayerComponent();
		if(plC.valid()) // TODO: This is pretty messy, handle this another way?
		{
			// Player rotation needs to be handled differently, since the client usually
			// has precedence for controlling the view angles.
			plC->SetViewRotation(rot);
		}
		else
			SetViewOrientation(rot);
	}
	return util::EventReply::Unhandled;
}

Quat BaseCharacterComponent::GetOrientationAxesRotation() const
{
	if(!m_orientationComponent)
		return uquat::identity();
	return m_orientationComponent->GetOrientationAxesRotation();
}

EulerAngles BaseCharacterComponent::GetLocalOrientationAngles() const { return EulerAngles(GetLocalOrientationRotation()); }
Quat BaseCharacterComponent::GetLocalOrientationRotation() const
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	return GetOrientationAxesRotation() * (pTrComponent ? pTrComponent->GetRotation() : uquat::identity());
}
EulerAngles BaseCharacterComponent::GetLocalOrientationViewAngles() const { return EulerAngles(GetLocalOrientationViewRotation()); }
Quat BaseCharacterComponent::GetLocalOrientationViewRotation() const { return GetOrientationAxesRotation() * GetViewOrientation(); }

Quat BaseCharacterComponent::WorldToLocalOrientation(const Quat &rot) { return GetOrientationAxesRotation() * rot; }
EulerAngles BaseCharacterComponent::WorldToLocalOrientation(const EulerAngles &ang) { return EulerAngles(WorldToLocalOrientation(uquat::create(ang))); }
Quat BaseCharacterComponent::LocalOrientationToWorld(const Quat &rot) { return uquat::get_inverse(GetOrientationAxesRotation()) * rot; }
EulerAngles BaseCharacterComponent::LocalOrientationToWorld(const EulerAngles &ang) { return EulerAngles(LocalOrientationToWorld(uquat::create(ang))); }

bool BaseCharacterComponent::HandleAnimationEvent(const AnimationEvent &ev)
{
	switch(ev.eventID) {
	case AnimationEvent::Type::FootstepLeft:
	case AnimationEvent::Type::FootstepRight:
		{
			if(ev.eventID == AnimationEvent::Type::FootstepLeft)
				FootStep(BaseCharacterComponent::FootType::Left);
			else
				FootStep(BaseCharacterComponent::FootType::Right);
			return true;
		}
	};
	return false;
}

UInt16 BaseCharacterComponent::GetAmmoCount(const std::string &ammoType) const
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	UInt32 ammoId = 0;
	auto *ammo = game->GetAmmoType(ammoType, &ammoId);
	if(ammo == nullptr)
		return 0;
	return GetAmmoCount(ammoId);
}
UInt16 BaseCharacterComponent::GetAmmoCount(UInt32 ammoType) const
{
	auto it = m_ammoCount.find(ammoType);
	if(it == m_ammoCount.end())
		return 0;
	return it->second;
}
void BaseCharacterComponent::SetAmmoCount(const std::string &ammoType, UInt16 count)
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	UInt32 ammoId = 0;
	auto *ammo = game->GetAmmoType(ammoType, &ammoId);
	if(ammo == nullptr)
		return;
	SetAmmoCount(ammoId, count);
}
void BaseCharacterComponent::SetAmmoCount(UInt32 ammoType, UInt16 count)
{
	auto it = m_ammoCount.find(ammoType);
	if(it != m_ammoCount.end()) {
		it->second = count;
		return;
	}
	m_ammoCount.insert(std::unordered_map<UInt32, UInt16>::value_type(ammoType, count));
}
void BaseCharacterComponent::AddAmmo(const std::string &ammoType, int16_t count)
{
	if(count < 0) {
		RemoveAmmo(ammoType, -count);
		return;
	}
	SetAmmoCount(ammoType, GetAmmoCount(ammoType) + count);
}
void BaseCharacterComponent::AddAmmo(UInt32 ammoType, int16_t count)
{
	if(count < 0) {
		RemoveAmmo(ammoType, -count);
		return;
	}
	SetAmmoCount(ammoType, GetAmmoCount(ammoType) + count);
}
void BaseCharacterComponent::RemoveAmmo(const std::string &ammoType, int16_t count)
{
	if(count < 0) {
		AddAmmo(ammoType, -count);
		return;
	}
	SetAmmoCount(ammoType, umath::max(static_cast<int32_t>(GetAmmoCount(ammoType)) - static_cast<int32_t>(count), 0));
}
void BaseCharacterComponent::RemoveAmmo(UInt32 ammoType, int16_t count)
{
	if(count < 0) {
		AddAmmo(ammoType, -count);
		return;
	}
	SetAmmoCount(ammoType, umath::max(static_cast<int32_t>(GetAmmoCount(ammoType)) - static_cast<int32_t>(count), 0));
}

void BaseCharacterComponent::PlayFootStepSound(FootType foot, const SurfaceMaterial &surfMat, float scale)
{
	CEPlayFootstepSound footStepInfo {foot, surfMat, scale};
	if(BroadcastEvent(EVENT_PLAY_FOOTSTEP_SOUND, footStepInfo) == util::EventReply::Handled)
		return;
	auto pSoundEmitterComponent = static_cast<pragma::BaseSoundEmitterComponent *>(GetEntity().FindComponent("sound_emitter").get());
	if(pSoundEmitterComponent == nullptr)
		return;
	const auto maxGain = 0.5f;
	auto &ent = GetEntity();
	auto soundType = ALSoundType::Effect;
	soundType |= (ent.IsPlayer() == true) ? ALSoundType::Player : ALSoundType::NPC;
	auto pSubmergibleComponent = ent.GetComponent<SubmergibleComponent>();
	if(pSubmergibleComponent.valid() && pSubmergibleComponent->IsSubmerged() == true)
		return; // Don't play footsteps when underwater
	pragma::BaseSoundEmitterComponent::SoundInfo sndInfo {};
	sndInfo.transmit = false;
	if(pSubmergibleComponent.valid() == false || pSubmergibleComponent->GetSubmergedFraction() == 0.f) {
		sndInfo.gain = maxGain * scale;
		pSoundEmitterComponent->EmitSound(surfMat.GetFootstepType(), soundType, sndInfo);
	}
	else {
		// Play water footstep sounds if we're knee-deep in water
		auto *pSurfWaterMat = ent.GetNetworkState()->GetGameState()->GetSurfaceMaterial("water");
		if(pSurfWaterMat != nullptr) {
			sndInfo.gain = maxGain * scale;
			pSoundEmitterComponent->EmitSound(pSurfWaterMat->GetFootstepType(), soundType, sndInfo);
		}
	}
}
MovementComponent *BaseCharacterComponent::GetMovementComponent() { return m_movementComponent; }
OrientationComponent *BaseCharacterComponent::GetOrientationComponent() { return m_orientationComponent; }

bool BaseCharacterComponent::IsCharacter() const { return true; }
bool BaseCharacterComponent::IsMoving() const
{
	CEIsMoving evData {};
	InvokeEventCallbacks(EVENT_IS_MOVING, evData);
	return evData.moving;
}

void BaseCharacterComponent::FootStep(FootType foot)
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(!pPhysComponent || pPhysComponent->IsOnGround() == false)
		return;

	CEOnFootStep footStepInfo {foot};
	BroadcastEvent(EVENT_ON_FOOT_STEP, footStepInfo);

	auto moveScale = 1.f;
	if(IsMoving() == true) {
		if(m_movementComponent)
			moveScale = 1.f - m_movementComponent->GetMovementBlendScale();
		if(moveScale < 0.1f) //0.25f)
			return;
	}
	auto *phys = static_cast<ControllerPhysObj *>(pPhysComponent->GetPhysicsObject());
	if(phys == nullptr || !phys->IsController())
		return;
	auto id = phys->GetGroundSurfaceMaterial();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto *mat = game->GetSurfaceMaterial(id);
	if(mat == nullptr)
		return;
	PlayFootStepSound(foot, *mat, moveScale); // TODO: Is Moving -> Blend move scale -> Same as player!
}
TraceData BaseCharacterComponent::GetAimTraceData(std::optional<float> maxDist) const
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(!pTrComponent)
		return {};
	auto trData = util::get_entity_trace_data(*pTrComponent);
	auto origin = GetEyePosition();
	auto dir = GetViewForward();
	trData.SetSource(origin);
	trData.SetTarget(origin + dir * (maxDist.has_value() ? *maxDist : static_cast<float>(GameLimits::MaxRayCastRange)));
	// See also: ControllerPhysObj::PostSimulate
	return trData;
}

EulerAngles BaseCharacterComponent::GetViewAngles() const { return EulerAngles(m_angView); }
void BaseCharacterComponent::SetViewAngles(const EulerAngles &ang) { SetViewOrientation(uquat::create(ang)); }

void BaseCharacterComponent::OnTick(double tDelta)
{
	UpdateOrientation();
	if(m_turnYaw != nullptr) {
		auto &ent = GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		if(pTrComponent) {
			auto pPhysComponent = ent.GetPhysicsComponent();
			auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
			if(phys == nullptr || phys->IsController()) {
				auto rotRef = GetOrientationAxesRotation();
				auto &rotCur = pTrComponent->GetRotation();
				auto rot = rotRef * rotCur;
				auto angCur = EulerAngles(rot);
				angCur.y = umath::approach_angle(angCur.y, *m_turnYaw, m_turnSpeed * CFloat(tDelta));
				rot = uquat::get_inverse(rotRef) * uquat::create(angCur);
				pTrComponent->SetRotation(rot);
			}
		}
	}
	UpdateMoveController();
}

Vector3 BaseCharacterComponent::GetViewForward() const { return uquat::forward(m_angView); }
Vector3 BaseCharacterComponent::GetViewRight() const { return uquat::right(m_angView); }
Vector3 BaseCharacterComponent::GetViewUp() const { return uquat::up(m_angView); }

void BaseCharacterComponent::GetViewOrientation(Vector3 *forward, Vector3 *right, Vector3 *up) const { uquat::get_orientation(m_angView, forward, right, up); }

Vector3 BaseCharacterComponent::GetEyePosition() const
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(!pTrComponent)
		return Vector3 {};
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto physType = pPhysComponent ? pPhysComponent->GetPhysicsType() : PHYSICSTYPE::NONE;
	if(physType != PHYSICSTYPE::BOXCONTROLLER && physType != PHYSICSTYPE::CAPSULECONTROLLER)
		return pTrComponent->GetPosition();
	Vector3 eyeOffset = pTrComponent->GetEyeOffset();
	Vector3 forward, right, up;
	GetViewOrientation(&forward, &right);
	up = pTrComponent->GetUp();
	Vector3 eyePos = pTrComponent->GetPosition();
	eyePos = eyePos + forward * eyeOffset.x + up * eyeOffset.y + right * eyeOffset.z;
	return eyePos;
}

Vector3 BaseCharacterComponent::GetShootPosition() const { return GetEyePosition(); }

bool BaseCharacterComponent::Jump(const Vector3 &velocity)
{
	if(CanJump() == false)
		return false;
	DetachFromGround();
	auto &ent = GetEntity();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(pVelComponent.valid())
		pVelComponent->AddVelocity(velocity);
	CEOnJump footStepInfo {velocity};
	if(BroadcastEvent(EVENT_ON_JUMP, footStepInfo) == util::EventReply::Handled)
		return true;
	return true;
}
bool BaseCharacterComponent::Jump()
{
	if(GetJumpPower() == 0.f)
		return false;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto upDir = m_orientationComponent ? m_orientationComponent->GetUpDirection() : uvec::UP;
	auto vel = upDir * (m_jumpPower->GetValue() * (pTrComponent ? pTrComponent->GetScale() : Vector3 {1.f, 1.f, 1.f}));
	return Jump(vel);
}

const util::PFloatProperty &BaseCharacterComponent::GetJumpPowerProperty() const { return m_jumpPower; }
float BaseCharacterComponent::GetJumpPower() const { return *m_jumpPower; }
void BaseCharacterComponent::SetJumpPower(float power) { *m_jumpPower = power; }
bool BaseCharacterComponent::CanJump() const
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	return pPhysComponent && pPhysComponent->IsOnGround() == true && pVelComponent.valid();
}
void BaseCharacterComponent::DetachFromGround(float duration) { m_tDetachFromGround = GetEntity().GetNetworkState()->GetGameState()->CurTime() + duration; }

//////////////////

CEOnDeployWeapon::CEOnDeployWeapon(BaseEntity &entWeapon) : weapon {entWeapon} {}
void CEOnDeployWeapon::PushArguments(lua_State *l) { weapon.GetLuaObject().push(l); }

//////////////////

CEOnSetActiveWeapon::CEOnSetActiveWeapon(BaseEntity *entWeapon) : weapon {entWeapon} {}
void CEOnSetActiveWeapon::PushArguments(lua_State *l)
{
	if(weapon != nullptr)
		weapon->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}

//////////////////

CEOnSetCharacterOrientation::CEOnSetCharacterOrientation(const Vector3 &up) : up {up} {}
void CEOnSetCharacterOrientation::PushArguments(lua_State *l) { Lua::Push<Vector3>(l, up); }

//////////////////

CEPlayFootstepSound::CEPlayFootstepSound(BaseCharacterComponent::FootType footType, const SurfaceMaterial &surfaceMaterial, float scale) : footType(footType), surfaceMaterial(surfaceMaterial), scale(scale) {}
void CEPlayFootstepSound::PushArguments(lua_State *l)
{
	Lua::PushInt(l, umath::to_integral(footType));
	Lua::Push<SurfaceMaterial *>(l, const_cast<SurfaceMaterial *>(&surfaceMaterial));
	Lua::PushNumber(l, scale);
}

//////////////////

CEOnFootStep::CEOnFootStep(BaseCharacterComponent::FootType footType) : footType {footType} {}
void CEOnFootStep::PushArguments(lua_State *l) { Lua::PushInt(l, umath::to_integral(footType)); }

//////////////////

CEOnJump::CEOnJump(const Vector3 &velocity) : velocity {velocity} {}
void CEOnJump::PushArguments(lua_State *l) { Lua::Push<Vector3>(l, velocity); }

//////////////////

CEIsMoving::CEIsMoving() {}
void CEIsMoving::PushArguments(lua_State *l) { Lua::PushBool(l, moving); }
uint32_t CEIsMoving::GetReturnCount() { return 1; }
void CEIsMoving::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		moving = Lua::CheckBool(l, -1);
}

//////////////////

CEViewRotation::CEViewRotation(const Quat &rotation) : rotation {rotation} {}
void CEViewRotation::PushArguments(lua_State *l) { Lua::Push<Quat>(l, rotation); }
