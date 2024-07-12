/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/movement_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/component_member_flags.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/orientation_component.hpp"
#include "pragma/physics/phys_material.hpp"
#include "pragma/physics/controller.hpp"
#include "pragma/lua/classes/ldef_vector.h"
#include <pragma/physics/movetypes.h>

using namespace pragma;

ComponentEventId MovementComponent::EVENT_CALC_MOVEMENT_SPEED = INVALID_COMPONENT_ID;
ComponentEventId MovementComponent::EVENT_CALC_AIR_MOVEMENT_MODIFIER = INVALID_COMPONENT_ID;
ComponentEventId MovementComponent::EVENT_CALC_MOVEMENT_ACCELERATION = INVALID_COMPONENT_ID;
ComponentEventId MovementComponent::EVENT_CALC_MOVEMENT_DIRECTION = INVALID_COMPONENT_ID;
void MovementComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseEntityComponent::RegisterEvents(componentManager, registerEvent);
	EVENT_CALC_MOVEMENT_SPEED = registerEvent("CALC_MOVEMENT_SPEED", ComponentEventInfo::Type::Explicit);
	EVENT_CALC_AIR_MOVEMENT_MODIFIER = registerEvent("CALC_AIR_MOVEMENT_MODIFIER", ComponentEventInfo::Type::Explicit);
	EVENT_CALC_MOVEMENT_ACCELERATION = registerEvent("CALC_MOVEMENT_ACCELERATION", ComponentEventInfo::Type::Explicit);
	EVENT_CALC_MOVEMENT_DIRECTION = registerEvent("CALC_MOVEMENT_DIRECTION", ComponentEventInfo::Type::Explicit);
}
MovementComponent::MovementComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void MovementComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PRE_PHYSICS_SIMULATE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateMovement(); });
}
void MovementComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void MovementComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

Vector3 MovementComponent::GetLocalVelocity() const
{
	auto &ent = GetEntity();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(pVelComponent.expired())
		return Vector3 {};
	auto vel = pVelComponent->GetVelocity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys != nullptr && phys->IsController()) {
		auto *physController = static_cast<ControllerPhysObj *>(phys);
		vel -= physController->GetGroundVelocity();
	}
	return vel;
}

float MovementComponent::GetMovementBlendScale() const
{
	auto blendScale = 0.f;
	auto vel = GetLocalVelocity();
	float speed = uvec::length(vel);
	auto mvSpeed = CalcMovementSpeed();
	auto speedMax = umath::max(mvSpeed.x, mvSpeed.y);
	if(speedMax == 0.f)
		blendScale = 0.f;
	else {
		blendScale = 1.f - speed / speedMax;
		if(blendScale > 1.f)
			blendScale = 1.f;
		else if(blendScale < 0.f)
			blendScale = 0.f;
	}
	return blendScale;
}

Vector2 MovementComponent::CalcMovementSpeed() const
{
	CECalcMovementSpeed evData {};
	if(InvokeEventCallbacks(EVENT_CALC_MOVEMENT_SPEED, evData) == util::EventReply::Handled)
		return evData.speed;
	return Vector2 {};
}
float MovementComponent::CalcAirMovementModifier() const
{
	CECalcAirMovementModifier evData {};
	if(InvokeEventCallbacks(EVENT_CALC_AIR_MOVEMENT_MODIFIER, evData) == util::EventReply::Handled)
		return evData.airMovementModifier;
	return 0.f;
}
float MovementComponent::CalcMovementAcceleration(float &optOutRampUpTime) const
{
	CECalcMovementAcceleration evData {};
	if(InvokeEventCallbacks(EVENT_CALC_MOVEMENT_ACCELERATION, evData) == util::EventReply::Handled) {
		optOutRampUpTime = evData.rampUpTime;
		return evData.acceleration;
	}
	return 0.f;
}

Vector3 MovementComponent::CalcMovementDirection(const Vector3 &forward, const Vector3 &right) const
{
	CECalcMovementDirection evData {forward, right};
	if(InvokeEventCallbacks(EVENT_CALC_MOVEMENT_DIRECTION, evData) == util::EventReply::Handled)
		return evData.direction;
	return Vector3 {};
}

bool MovementComponent::CanMove() const
{
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	auto mvType = pPhysComponent ? pPhysComponent->GetMoveType() : MOVETYPE::NONE;
	if(!pPhysComponent || mvType == MOVETYPE::NONE)
		return false;
	auto *pPhysObj = pPhysComponent->GetPhysicsObject();
	return pPhysObj != nullptr && pPhysObj->IsController();
}

void MovementComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(OrientationComponent))
		m_orientationComponent = &static_cast<OrientationComponent &>(component);
	else {
		auto *charC = dynamic_cast<BaseCharacterComponent *>(&component);
		if(m_charComponent)
			m_charComponent = charC;
	}
}
void MovementComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(OrientationComponent))
		m_orientationComponent = nullptr;
	else {
		auto *charC = dynamic_cast<BaseCharacterComponent *>(&component);
		if(m_charComponent)
			m_charComponent = nullptr;
	}
}

bool MovementComponent::UpdateMovement()
{
	//if(m_entity->IsPlayer() == true)
	//	m_entity->GetNetworkState()->GetGameState()->HandlePlayerMovement();
	//if(true)
	//	return;
	if(CanMove() == false)
		return false;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || phys->IsController() == false)
		return false;
	auto mv = pPhysComponent->GetMoveType();
	if(mv == MOVETYPE::NONE || mv == MOVETYPE::PHYSICS)
		return false;
	auto *physController = static_cast<ControllerPhysObj *>(phys);
	auto pTrComponent = ent.GetTransformComponent();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	auto pos = pTrComponent ? pTrComponent->GetPosition() : Vector3 {};
	auto vel = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {};

	auto bOnGround = pPhysComponent->IsOnGround();
	auto groundVelocity = physController->GetGroundVelocity();
	vel -= groundVelocity; // We only care about the local velocity; The ground velocity will be re-added later

	auto pSubmergibleComponent = ent.GetComponent<pragma::SubmergibleComponent>();
	auto bSubmerged = (pSubmergibleComponent.valid() && pSubmergibleComponent->GetSubmergedFraction() > 0.5f) ? true : false;
	Vector3 axisForward = uvec::FORWARD;
	Vector3 axisRight = uvec::RIGHT;
	Vector3 axisUp = uvec::UP;
	auto viewRot = uquat::identity();
	if(m_orientationComponent) {
		m_orientationComponent->GetOrientationAxes(&axisForward, &axisRight, &axisUp);
		viewRot = m_orientationComponent->GetOrientationAxesRotation();
	}

	Quat rot;
	if(m_charComponent)
		rot = m_charComponent->GetViewOrientation();
	else
		rot = ent.GetRotation();

	Quat invViewRot = uquat::get_inverse(viewRot);

	Vector3 forward = uquat::forward(rot);
	Vector3 right = uquat::right(rot);
	if(mv == MOVETYPE::WALK && bSubmerged == false) {
		// No movement on up-axis
		auto upDir = uvec::UP;
		if(m_orientationComponent)
			upDir = m_orientationComponent->GetUpDirection();
		auto angle = uvec::dot(forward, upDir);
		if(umath::abs(angle) < 0.99f)
			forward = uvec::project_to_plane(forward, upDir, 0.f);
		else // Looking straight up or down; Use camera up-direction as forward-direction instead
		{
			auto rotUpDir = uquat::up(rot);
			if(angle > 0.f)
				rotUpDir = -rotUpDir;
			forward = uvec::project_to_plane(rotUpDir, upDir, 0.f);
		}
		right = uvec::project_to_plane(right, upDir, 0.f);

		uvec::normalize(&forward);
		uvec::normalize(&right);

		// Deprecated
		/*// TODO: Does this work with custom player up directions?
		// TODO: There should be a better way of doing this
		if(forward.y <= -0.999999f || forward.y >= 0.999999f)
		{
			auto ang = GetViewAngles();
			ang.p = 0.f;
			forward = ang.Forward();
			right = ang.Right();
		}
		else
		{
			// No movement on up-axis
			forward = forward *viewRot;
			right = right *viewRot;

			forward.y = 0.f;
			right.y = 0.f;

			uvec::normalize(&forward);
			uvec::normalize(&right);

			forward = forward *invViewRot;
			right = right *invViewRot;
		}*/
	}

	auto pTimeScaleComponent = ent.GetTimeScaleComponent();
	auto ts = pTimeScaleComponent.valid() ? CFloat(pTimeScaleComponent->GetTimeScale()) : 1.f;
	auto scale = pTrComponent ? pTrComponent->GetScale() : Vector3 {1.f, 1.f, 1.f};
	auto speed = CalcMovementSpeed() * ts * umath::abs_max(scale.x, scale.y, scale.z);

	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	float timeToReachFullAcc = 0.f;
	auto acceleration = CalcMovementAcceleration(timeToReachFullAcc);
	auto tDelta = CFloat(game->DeltaTickTime()) * ts;

	{
		//if(phys != nullptr && phys->IsController())
		{
			auto *groundObject = physController->GetGroundPhysCollisionObject();
			if(groundObject != nullptr && groundObject->IsRigid()) {
				//if(owner->GetNetworkState()->IsClient())
				if(ent.IsNPC() || ent.GetNetworkState()->IsClient()) {
					// TODO: Rotate with ground object?
					//auto angVel = static_cast<PhysRigidBody*>(groundObject)->GetAngularVelocity() *static_cast<float>(tDelta);
					//auto ang = EulerAngles(umath::rad_to_deg(angVel.x),umath::rad_to_deg(angVel.y),umath::rad_to_deg(angVel.z));
					//owner->SetAngles(owner->GetAngles() +ang);
					//SetViewAngles(GetViewAngles() +ang);
				}
			}
		}
	}

	if(pPhysComponent->IsGroundWalkable() || mv != MOVETYPE::WALK || bSubmerged == true) {
		auto friction = 0.8f;
		std::optional<Vector3> contactNormal = {};
		if(phys != nullptr && phys->IsController()) {
			auto *controller = static_cast<ControllerPhysObj *>(phys);
			auto *surfMat = controller->GetGroundMaterial();
			friction = surfMat ? surfMat->GetDynamicFriction() : 1.f;
			contactNormal = controller->GetController()->GetGroundTouchNormal();
		}
		auto frictionVel = vel;
		if(contactNormal.has_value()) // Only apply friction to the component of the velocity which is parallel to the ground (i.e. jumping and such remain unaffected)
			frictionVel = uvec::project_to_plane(frictionVel, *contactNormal, 0.f);
		auto frictionForce = -frictionVel * friction;
		//if(ent.IsPlayer())
		//	Con::cout<<"Player friction: "<<friction<<Con::endl;
		//else
		//	Con::cout<<"NPC friction: "<<friction<<Con::endl;

		vel += frictionForce * umath::min(tDelta * acceleration, 1.f);
	}
	else
		speed *= CalcAirMovementModifier();

	Vector3 dir = Vector3(0, 0, 0);
	auto isFrozen = false;
	if(m_charComponent)
		isFrozen = m_charComponent->IsFrozen();
	if(!isFrozen)
		dir = CalcMovementDirection(forward, right);
	auto l = uvec::length(dir);
	if(l > 0.f)
		dir /= l;

	if(l == 0.f)
		m_timeSinceMovementStart = 0.f;
	else
		m_timeSinceMovementStart = umath::min(m_timeSinceMovementStart + tDelta, timeToReachFullAcc);

	auto speedDir = glm::dot(dir, vel); // The speed in the movement direction of the current velocity
	if(speedDir < umath::abs(speed.x)) {
		auto speedDelta = speed.x - speedDir;

		auto addSpeed = speedDelta * tDelta * acceleration;
		auto f = (timeToReachFullAcc > 0.f) ? (m_timeSinceMovementStart / timeToReachFullAcc) : 1.f;
		addSpeed *= f;

		vel += dir * umath::min(addSpeed, speedDelta);
	}

	// Calculate sideways movement speed (NPC animation movement only)
	if(speed.y != 0.f) {
		auto dirRight = (uvec::length_sqr(dir) > 0.99f) ? uvec::cross(dir, pTrComponent ? pTrComponent->GetUp() : uvec::UP) : (pTrComponent ? pTrComponent->GetRight() : uvec::RIGHT);
		auto speedDir = glm::dot(dirRight, vel);
		if(speedDir < umath::abs(speed.y)) {
			auto speedDelta = speed.y - speedDir;
			vel += dirRight * umath::min(speedDelta * tDelta * acceleration, speedDelta);
		}
	}
	//static PhysContactInfo lastGroundContact = {btManifoldPoint{}};
	m_moveVelocity = vel;

	//static auto forceTest = 100.f;
	//vel -= uvec::create(lastGroundContact.contactPoint.m_normalWorldOnB) *forceTest;
	//auto *pGroundContactInfo = physController->GetGroundContactInfo();
	//if(pGroundContactInfo != nullptr)
	//	lastGroundContact = *pGroundContactInfo;

	if(pVelComponent.valid()) {
		vel += groundVelocity; // Re-add ground velocity
		pVelComponent->SetVelocity(vel);
	}
	//return controller->Move(disp,elapsedTime,minDist);
#ifdef ENABLE_DEPRECATED_PHYSICS
	const auto threshold = -0.03;
	auto localVel = vel;
	uvec::rotate(&localVel, viewRot);
	auto contactNormal = physController->GetController()->GetGroundTouchNormal();
	;
	if(vel.y <= 0.1f && physController->IsGroundWalkable() && (contactNormal.has_value() && pGroundContactInfo->contactDistance >= threshold) && mv == MOVETYPE::WALK) {
		auto &info = *pGroundContactInfo;
		//auto pos = uvec::create((info.controllerIndex == 0u ? info.contactPoint.getPositionWorldOnA() : info.contactPoint.getPositionWorldOnB()) /PhysEnv::WORLD_SCALE);
		auto n = -*contactNormal;
		//ent.GetNetworkState()->GetGameState()->DrawLine(
		//	pos,pos -n *100.f,Color::Red,5.f
		//);

		const auto force = 100.f; // Somewhat arbitrary; The force to apply to the controller to make them stick to the ground
		physController->AddLinearVelocity(n * force);
	}
#endif

	return true;
}

const Vector3 &MovementComponent::GetMoveVelocity() const { return m_moveVelocity; }
void MovementComponent::SetMoveVelocity(const Vector3 &vel) { m_moveVelocity = vel; }

//////////////////

CECalcMovementSpeed::CECalcMovementSpeed() {}
void CECalcMovementSpeed::PushArguments(lua_State *l) {}
uint32_t CECalcMovementSpeed::GetReturnCount() { return 1; }
void CECalcMovementSpeed::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		speed = *Lua::CheckVector2(l, -1);
}

//////////////////

CECalcAirMovementModifier::CECalcAirMovementModifier() {}
void CECalcAirMovementModifier::PushArguments(lua_State *l) {}
uint32_t CECalcAirMovementModifier::GetReturnCount() { return 1; }
void CECalcAirMovementModifier::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		airMovementModifier = Lua::CheckNumber(l, -1);
}

//////////////////

CECalcMovementAcceleration::CECalcMovementAcceleration() {}
void CECalcMovementAcceleration::PushArguments(lua_State *l) {}
uint32_t CECalcMovementAcceleration::GetReturnCount() { return 2; }
void CECalcMovementAcceleration::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -2))
		acceleration = Lua::CheckNumber(l, -2);
	if(Lua::IsSet(l, -1))
		rampUpTime = Lua::CheckNumber(l, -1);
}

//////////////////

CECalcMovementDirection::CECalcMovementDirection(const Vector3 &forward, const Vector3 &right) : forward(forward), right(right) {}
void CECalcMovementDirection::PushArguments(lua_State *l)
{
	Lua::Push<Vector3>(l, forward);
	Lua::Push<Vector3>(l, right);
}
uint32_t CECalcMovementDirection::GetReturnCount() { return 1; }
void CECalcMovementDirection::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		direction = *Lua::CheckVector(l, -1);
}
