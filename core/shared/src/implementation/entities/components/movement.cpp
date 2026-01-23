// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.movement;

using namespace pragma;

void MovementComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseEntityComponent::RegisterEvents(componentManager, registerEvent);
	movementComponent::EVENT_ON_UPDATE_MOVEMENT = registerEvent("ON_UPDATE_MOVEMENT", ComponentEventInfo::Type::Explicit);
}
MovementComponent::MovementComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void MovementComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(basePhysicsComponent::EVENT_ON_PRE_PHYSICS_SIMULATE, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateMovement(); });
}
void MovementComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void MovementComponent::OnRemove() { BaseEntityComponent::OnRemove(); }

void MovementComponent::SetSpeed(const Vector2 &speed) { m_movementSpeed = speed; }
const Vector2 &MovementComponent::GetSpeed() const { return m_movementSpeed; }

void MovementComponent::SetAirModifier(float modifier) { m_airMovementModifier = modifier; }
float MovementComponent::GetAirModifier() const { return m_airMovementModifier; }

void MovementComponent::SetAcceleration(float acc) { m_movementAcceleration = acc; }
float MovementComponent::GetAcceleration() const { return m_movementAcceleration; }

void MovementComponent::SetAccelerationRampUpTime(float rampUpTime) { m_accelerationRampUpTime = rampUpTime; }
float MovementComponent::GetAccelerationRampUpTime() const { return m_accelerationRampUpTime; }

void MovementComponent::SetDirection(const std::optional<Vector3> &dir) { m_movementDirection = dir; }
const std::optional<Vector3> &MovementComponent::GetDirection() const { return m_movementDirection; }

void MovementComponent::SetDirectionMagnitude(MoveDirection direction, float magnitude) { m_directionMagnitude[math::to_integral(direction)] = magnitude; }
float MovementComponent::GetDirectionMagnitude(MoveDirection direction) const { return m_directionMagnitude[math::to_integral(direction)]; }

Vector3 MovementComponent::GetLocalVelocity() const
{
	auto &ent = GetEntity();
	auto pVelComponent = ent.GetComponent<VelocityComponent>();
	if(pVelComponent.expired())
		return Vector3 {};
	auto vel = pVelComponent->GetVelocity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys != nullptr && phys->IsController()) {
		auto *physController = static_cast<physics::ControllerPhysObj *>(phys);
		vel -= physController->GetGroundVelocity();
	}
	return vel;
}

float MovementComponent::GetMovementBlendScale() const
{
	auto blendScale = 0.f;
	auto vel = GetLocalVelocity();
	float speed = uvec::length(vel);
	auto mvSpeed = GetSpeed();
	auto speedMax = math::max(mvSpeed.x, mvSpeed.y);
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

bool MovementComponent::CanMove() const
{
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	auto mvType = pPhysComponent ? pPhysComponent->GetMoveType() : physics::MoveType::None;
	if(!pPhysComponent || mvType == physics::MoveType::None)
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
		if(charC)
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
		if(charC)
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
	if(mv == physics::MoveType::None || mv == physics::MoveType::Physics)
		return false;
	InvokeEventCallbacks(movementComponent::EVENT_ON_UPDATE_MOVEMENT);
	auto *physController = static_cast<physics::ControllerPhysObj *>(phys);
	auto pTrComponent = ent.GetTransformComponent();
	auto pVelComponent = ent.GetComponent<VelocityComponent>();
	auto pos = pTrComponent ? pTrComponent->GetPosition() : Vector3 {};
	auto vel = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {};

	auto bOnGround = pPhysComponent->IsOnGround();
	auto groundVelocity = physController->GetGroundVelocity();
	vel -= groundVelocity; // We only care about the local velocity; The ground velocity will be re-added later

	auto pSubmergibleComponent = ent.GetComponent<SubmergibleComponent>();
	auto bSubmerged = (pSubmergibleComponent.valid() && pSubmergibleComponent->GetSubmergedFraction() > 0.5f) ? true : false;
	Vector3 axisForward = uvec::PRM_FORWARD;
	Vector3 axisRight = uvec::PRM_RIGHT;
	Vector3 axisUp = uvec::PRM_UP;
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
	if(mv == physics::MoveType::Walk && bSubmerged == false) {
		// No movement on up-axis
		auto upDir = uvec::PRM_UP;
		if(m_orientationComponent)
			upDir = m_orientationComponent->GetUpDirection();
		auto angle = uvec::dot(forward, upDir);
		if(math::abs(angle) < 0.99f)
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
	auto speed = GetSpeed() * ts * math::abs_max(scale.x, scale.y, scale.z);

	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	float timeToReachFullAcc = GetAccelerationRampUpTime();
	auto acceleration = GetAcceleration();
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
					//auto ang = EulerAngles(pragma::math::rad_to_deg(angVel.x),pragma::math::rad_to_deg(angVel.y),pragma::math::rad_to_deg(angVel.z));
					//owner->SetAngles(owner->GetAngles() +ang);
					//SetViewAngles(GetViewAngles() +ang);
				}
			}
		}
	}

	if(pPhysComponent->IsGroundWalkable() || mv != physics::MoveType::Walk || bSubmerged == true) {
		auto friction = 0.8f;
		std::optional<Vector3> contactNormal = {};
		if(phys != nullptr && phys->IsController()) {
			auto *controller = static_cast<physics::ControllerPhysObj *>(phys);
			auto *surfMat = controller->GetGroundMaterial();
			friction = surfMat ? surfMat->GetDynamicFriction() : 1.f;
			contactNormal = controller->GetController()->GetGroundTouchNormal();
		}
		auto frictionVel = vel;
		if(contactNormal.has_value()) // Only apply friction to the component of the velocity which is parallel to the ground (i.e. jumping and such remain unaffected)
			frictionVel = uvec::project_to_plane(frictionVel, *contactNormal, 0.f);
		auto frictionForce = -frictionVel * friction;
		//if(ent.IsPlayer())
		//	Con::COUT<<"Player friction: "<<friction<<Con::endl;
		//else
		//	Con::COUT<<"NPC friction: "<<friction<<Con::endl;

		vel += frictionForce * math::min(tDelta * acceleration, 1.f);
	}
	else
		speed *= GetAirModifier();

	Vector3 dir = Vector3(0, 0, 0);
	auto isFrozen = false;
	if(m_charComponent)
		isFrozen = m_charComponent->IsFrozen();
	if(!isFrozen) {
		if(m_movementDirection)
			dir = *m_movementDirection;
		else {
			dir += forward * GetDirectionMagnitude(MoveDirection::Forward);
			dir -= forward * GetDirectionMagnitude(MoveDirection::Backward);
			dir += right * GetDirectionMagnitude(MoveDirection::Right);
			dir -= right * GetDirectionMagnitude(MoveDirection::Left);
		}
	}
	auto l = uvec::length(dir);
	if(l > 0.f)
		dir /= l;

	if(l == 0.f)
		m_timeSinceMovementStart = 0.f;
	else
		m_timeSinceMovementStart = math::min(m_timeSinceMovementStart + tDelta, timeToReachFullAcc);

	auto speedDir = glm::dot(dir, vel); // The speed in the movement direction of the current velocity
	if(speedDir < math::abs(speed.x)) {
		auto speedDelta = speed.x - speedDir;

		auto addSpeed = speedDelta * tDelta * acceleration;
		auto f = (timeToReachFullAcc > 0.f) ? (m_timeSinceMovementStart / timeToReachFullAcc) : 1.f;
		addSpeed *= f;

		vel += dir * math::min(addSpeed, speedDelta);
	}

	// Calculate sideways movement speed (NPC animation movement only)
	if(speed.y != 0.f) {
		auto dirRight = (uvec::length_sqr(dir) > 0.99f) ? uvec::cross(dir, pTrComponent ? pTrComponent->GetUp() : uvec::PRM_UP) : (pTrComponent ? pTrComponent->GetRight() : uvec::PRM_RIGHT);
		auto speedDir = glm::dot(dirRight, vel);
		if(speedDir < math::abs(speed.y)) {
			auto speedDelta = speed.y - speedDir;
			vel += dirRight * math::min(speedDelta * tDelta * acceleration, speedDelta);
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
	if(vel.y <= 0.1f && physController->IsGroundWalkable() && (contactNormal.has_value() && pGroundContactInfo->contactDistance >= threshold) && mv == pragma::physics::MoveType::Walk) {
		auto &info = *pGroundContactInfo;
		//auto pos = uvec::create((info.controllerIndex == 0u ? info.contactPoint.getPositionWorldOnA() : info.contactPoint.getPositionWorldOnB()) /PhysEnv::WORLD_SCALE);
		auto n = -*contactNormal;
		//ent.GetNetworkState()->GetGameState()->DrawLine(
		//	pos,pos -n *100.f,colors::Red,5.f
		//);

		const auto force = 100.f; // Somewhat arbitrary; The force to apply to the controller to make them stick to the ground
		physController->AddLinearVelocity(n * force);
	}
#endif

	return true;
}

const Vector3 &MovementComponent::GetMoveVelocity() const { return m_moveVelocity; }
void MovementComponent::SetMoveVelocity(const Vector3 &vel) { m_moveVelocity = vel; }
