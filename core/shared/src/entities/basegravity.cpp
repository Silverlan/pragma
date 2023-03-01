/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/basegravity.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/util/util_ballistic.h"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/controller.hpp"
#include <pragma/physics/movetypes.h>

using namespace pragma;

void BaseGravity::SetGravityScale(float scale) { m_gravityScale = scale; }
void BaseGravity::SetGravityOverride(const Vector3 &dir)
{
	if(m_gravityDir == nullptr)
		m_gravityDir = std::make_shared<Vector3>();
	m_gravityDir->x = dir.x;
	m_gravityDir->y = dir.y;
	m_gravityDir->z = dir.z;
}
void BaseGravity::SetGravityOverride(const Vector3 &dir, float gravity)
{
	SetGravityOverride(dir);
	SetGravityOverride(gravity);
}
void BaseGravity::SetGravityOverride(float gravity)
{
	if(m_gravity == nullptr)
		m_gravity = std::make_shared<float>();
	*m_gravity = gravity;
}
void BaseGravity::SetGravityOverride()
{
	m_gravity = nullptr;
	m_gravityDir = nullptr;
}
bool BaseGravity::HasGravityForceOverride() const { return (m_gravity != nullptr) ? true : false; }
bool BaseGravity::HasGravityDirectionOverride() const { return (m_gravityDir != nullptr) ? true : false; }

//~#PDOC f-ents-gravitycomponent-getgravityscale
float BaseGravity::GetGravityScale() const { return m_gravityScale; }

Vector3 BaseGravity::GetGravityDirection(NetworkState *state) const
{
	if(m_gravityDir != nullptr)
		return *m_gravityDir;
	if(state == nullptr)
		return uvec::ORIGIN;
	Game *game = state->GetGameState();
	return glm::normalize(game->GetGravity());
}
float BaseGravity::GetGravity(NetworkState *state) const
{
	if(m_gravity != nullptr)
		return *m_gravity;
	if(state == nullptr)
		return 0.f;
	Game *game = state->GetGameState();
	return glm::length(game->GetGravity());
}
Vector3 BaseGravity::GetGravityForce(NetworkState *state) const
{
	if(state == nullptr)
		return Vector3(0, 0, 0);
	Game *game = state->GetGameState();
	Vector3 force(0, 0, 0);
	if(m_gravityDir != nullptr) {
		force.x = m_gravityDir->x;
		force.y = m_gravityDir->y;
		force.z = m_gravityDir->z;
		if(m_gravity != nullptr)
			force *= *m_gravity;
		else // Keep world force
		{
			Vector3 gravity = game->GetGravity();
			float len = glm::length(gravity);
			force *= len;
		}
	}
	else if(m_gravity != nullptr) // Keep world gravity direction
	{
		Vector3 gravity = game->GetGravity();
		gravity = glm::normalize(gravity) * (*m_gravity);
		force.x = gravity.x;
		force.y = gravity.y;
		force.z = gravity.z;
	}
	else // Use world gravity
	{
		Vector3 gravity = game->GetGravity();
		force.x = gravity.x;
		force.y = gravity.y;
		force.z = gravity.z;
	}
	force *= m_gravityScale;
	return force;
}

/////////////////////

GravityComponent::GravityComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void GravityComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnPhysicsInitialized(); });
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_DYNAMIC_PHYSICS_UPDATED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ApplyGravity(static_cast<CEPhysicsUpdateData &>(evData.get()).deltaTime); });
}

void GravityComponent::ApplyGravity(double dt)
{
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	auto *pPhys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(pPhys == nullptr || pPhys->IsDisabled() == true)
		return;
	auto moveType = pPhysComponent->GetMoveType();
	if(moveType != MOVETYPE::WALK && moveType != MOVETYPE::PHYSICS)
		return;
	if(pPhys->IsRigid()) {
		auto *pPhysRigid = static_cast<RigidPhysObj *>(pPhys);
		if(pPhysRigid->IsKinematic() || pPhysRigid->IsStatic())
			return;
		auto f = GetGravityForce();
		f *= pPhysRigid->GetMass();

		for(auto hObj : pPhysRigid->GetCollisionObjects()) {
			if(hObj.IsValid() == false || hObj->IsAsleep())
				continue;
			hObj->GetRigidBody()->ApplyForce(f, false);
		}
	}
	else if(pPhys->IsController()) {
		auto *pPhysObjController = static_cast<ControllerPhysObj *>(pPhys);
		auto *pCollisionObject = pPhysObjController->GetCollisionObject();
		if(pCollisionObject == nullptr)
			return;
		auto t = pCollisionObject->GetWorldTransform();
		//m_originLast = t.GetOrigin();
		Vector3 disp {};
		if(pPhysObjController->IsGroundWalkable() == false) //!m_controller->onGround())
		{
			Vector3 f = GetGravityForce() * CFloat(dt);
			disp += Vector3(f.x, f.y, f.z);
		}
		else {
			//static Vector3 oldVelocity = {};

			// TODO: Code below doesn't work with moving kinematic platforms. What was this for?
#if 0
			auto *physColGround = pPhysObjController->GetGroundPhysCollisionObject();
			if(physColGround != nullptr && physColGround->IsRigid())
			{
				auto &ent = GetEntity();
				auto charComponent = ent.GetCharacterComponent();
				if(charComponent.valid())
				{
					auto &moveVel = charComponent->GetMoveVelocity();
					auto v = pPhysObjController->GetGroundVelocity();

					auto speed = uvec::length(v);
					auto dir = v;
					if(speed > 0.f)
						dir /= speed;
					auto speedDir = glm::dot(dir,disp -moveVel); // Speed in direction of ground velocity
					if(speedDir < umath::abs(speed))
					{
						auto speedDelta = speed -speedDir;
						disp += dir *speedDelta;
					}
				}
				/*Con::cout<<"Ground Velocity: ("<<v.x<<","<<v.y<<","<<v.z<<") ("<<disp.x<<","<<disp.y<<","<<disp.z<<")"<<Con::endl;
				disp += (v -oldVelocity);
				oldVelocity = v;*/

			}
#endif
			//else
			//	oldVelocity = {};
		}
#pragma message("TODO: Use Verlet Gravity")
		//

		disp *= dt;
		auto *pPhysController = pPhysObjController->GetController();
#ifndef TEST_PHYSX
		//auto walkDir = pPhysController->GetLastMoveDisplacement() +disp;
		//pPhysController->Move(walkDir);
		pPhysController->AddMoveVelocity(disp);
#endif
	}
	else if(pPhys->IsSoftBody()) {
		auto *pPhysObjSoftBody = static_cast<SoftBodyPhysObj *>(pPhys);
		auto f = GetGravityForce() * CFloat(dt);
		for(auto &hSoftBody : pPhysObjSoftBody->GetSoftBodies()) {
			if(hSoftBody.IsValid() == false)
				continue;
			hSoftBody->AddVelocity(f);
		}
	}
}

void GravityComponent::OnPhysicsInitialized() {}
void GravityComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Vector3 GravityComponent::GetGravityDirection() const { return BaseGravity::GetGravityDirection(GetEntity().GetNetworkState()); }
float GravityComponent::GetGravity() const { return BaseGravity::GetGravity(GetEntity().GetNetworkState()); }
Vector3 GravityComponent::GetGravityForce() const { return BaseGravity::GetGravityForce(GetEntity().GetNetworkState()); }

void GravityComponent::SetGravityOverride(const Vector3 &dir) { BaseGravity::SetGravityOverride(dir); }
void GravityComponent::SetGravityOverride(float gravity) { BaseGravity::SetGravityOverride(gravity); }
void GravityComponent::SetGravityOverride() { BaseGravity::SetGravityOverride(); }
void GravityComponent::SetGravityScale(float scale) { BaseGravity::SetGravityScale(scale); }
bool GravityComponent::CalcBallisticVelocity(const Vector3 &origin, const Vector3 &destPos, float fireAngle, float maxSpeed, float spread, float maxPitch, float maxYaw, Vector3 &vel) const
{
	auto gravity = -GetGravityForce().y;
	auto b = umath::calc_ballistic_velocity(origin, destPos, fireAngle, gravity, vel);
	if(b == false)
		return false;

	// Apply random spread
	auto spreadAngles = EulerAngles(umath::random(-spread, spread), umath::random(-spread, spread), 0.f);
	uvec::rotate(&vel, spreadAngles);
	//

	// Clamp to max speed
	auto l = uvec::length(vel);
	if(l > maxSpeed)
		l = maxSpeed;
	//

	// Clamp to maximum angles
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent) {
		auto velAng = uvec::to_angle(vel);
		auto ang = pTrComponent->GetAngles();
		ang.p = umath::clamp_angle(velAng.p, ang.p - maxPitch, ang.p + maxPitch);
		ang.y = umath::clamp_angle(velAng.y, ang.y - maxYaw, ang.y + maxYaw);
		vel = ang.Forward() * l;
	}
	else {
		uvec::normalize(&vel);
		vel = vel * l;
	}
	//

	return true;
}
