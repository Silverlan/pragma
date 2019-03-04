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
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_sound_emitter_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/model/model.h"
#include "pragma/entities/baseentity_trace.hpp"
#include "pragma/physics/physenvironment.h"

using namespace pragma;

extern DLLENGINE Engine *engine;

//////////////////

#pragma optimize("",off)
#ifdef PHYS_ENGINE_BULLET
void BaseCharacterComponent::InitializeController()
{
	//auto &ent = GetEntity();
	//PhysObj *phys = ent.GetPhysicsObject();
	//if(phys == NULL || !phys->IsController())
	//	return;
	
}
void BaseCharacterComponent::SetUpDirection(const Vector3 &direction)
{
	auto ndirection = direction;
	uvec::normalize(&ndirection);
	*m_upDirection = ndirection;

	//m_axRot = uquat::create(uvec::FORWARD,uvec::RIGHT,m_upDirection);//uvec::UP);

	m_axRot = uvec::get_rotation(Vector3(0.f,1.f,0.f),*m_upDirection);
	uquat::normalize(m_axRot);
	m_axForward = Vector3(0.f,0.f,1.f) *m_axRot;
	m_axRight = Vector3(-1.f,0.f,0.f) *m_axRot;

	//auto &ent = GetEntity();
	//PhysObj *phys = ent.GetPhysicsObject();
	//if(phys == NULL || !phys->IsController())
	//	return;
	
}
#elif PHYS_ENGINE_PHYSX
void BaseCharacterComponent::SetUpDirection(const Vector3 &direction)
{
	m_upDirection = direction;
	Vector3::normalize(&m_upDirection);

	m_axRot = Vector3::getRotation(Vector3(0.f,1.f,0.f),m_upDirection);
	m_axRot.Normalize();
	m_axForward = Vector3(0.f,0.f,1.f) *m_axRot;
	m_axRight = Vector3(-1.f,0.f,0.f) *m_axRot;

	PhysObj *phys = m_entity->GetPhysicsObject();
	if(phys == NULL || !phys->IsController())
		return;
	ControllerPhysObj *physController = static_cast<ControllerPhysObj*>(phys);
	physx::PxController *controller = physController->GetController();
	controller->setUpDirection(physx::PxVec3(m_upDirection.x,m_upDirection.y,m_upDirection.z));
}
void BaseCharacterComponent::InitializeController()
{
	PhysObj *phys = m_entity->GetPhysicsObject();
	if(phys == NULL || !phys->IsController())
		return;
	ControllerPhysObj *physController = static_cast<ControllerPhysObj*>(phys);
	physController->SetSlopeLimit(GetSlopeLimit());
	physController->SetStepOffset(GetStepOffset());
	physx::PxController *controller = physController->GetController();
	controller->setNonWalkableMode(physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING);
	Vector3 &upDirection = m_upDirection;
	controller->setUpDirection(physx::PxVec3(upDirection.x,upDirection.y,upDirection.z));
}
#endif

ComponentEventId BaseCharacterComponent::EVENT_ON_FOOT_STEP = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_ON_CHARACTER_ORIENTATION_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_ON_DEPLOY_WEAPON = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_ON_SET_ACTIVE_WEAPON = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_PLAY_FOOTSTEP_SOUND = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_CALC_MOVEMENT_SPEED = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_CALC_AIR_MOVEMENT_MODIFIER = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_CALC_MOVEMENT_ACCELERATION = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_CALC_MOVEMENT_DIRECTION = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_IS_MOVING = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_HANDLE_VIEW_ROTATION = INVALID_COMPONENT_ID;
ComponentEventId BaseCharacterComponent::EVENT_ON_JUMP = INVALID_COMPONENT_ID;
void BaseCharacterComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	BaseActorComponent::RegisterEvents(componentManager);
	auto componentType = std::type_index(typeid(BaseCharacterComponent));
	EVENT_ON_FOOT_STEP = componentManager.RegisterEvent("ON_FOOT_STEP");
	EVENT_ON_CHARACTER_ORIENTATION_CHANGED = componentManager.RegisterEvent("ON_CHARACTER_ORIENTATION_CHANGED");
	EVENT_ON_DEPLOY_WEAPON = componentManager.RegisterEvent("ON_DEPLOY_WEAPON");
	EVENT_ON_SET_ACTIVE_WEAPON = componentManager.RegisterEvent("ON_SET_ACTIVE_WEAPON");
	EVENT_PLAY_FOOTSTEP_SOUND = componentManager.RegisterEvent("PLAY_FOOTSTEP_SOUND");
	EVENT_CALC_MOVEMENT_SPEED = componentManager.RegisterEvent("CALC_MOVEMENT_SPEED",componentType);
	EVENT_CALC_AIR_MOVEMENT_MODIFIER = componentManager.RegisterEvent("CALC_AIR_MOVEMENT_MODIFIER",componentType);
	EVENT_CALC_MOVEMENT_ACCELERATION = componentManager.RegisterEvent("CALC_MOVEMENT_ACCELERATION",componentType);
	EVENT_CALC_MOVEMENT_DIRECTION = componentManager.RegisterEvent("CALC_MOVEMENT_DIRECTION",componentType);
	EVENT_IS_MOVING = componentManager.RegisterEvent("IS_MOVING",componentType);
	EVENT_HANDLE_VIEW_ROTATION = componentManager.RegisterEvent("HANDLE_VIEW_ROTATION",componentType);
	EVENT_ON_JUMP = componentManager.RegisterEvent("ON_JUMP");
}

BaseCharacterComponent::BaseCharacterComponent(BaseEntity &ent)
	: BaseActorComponent(ent),m_upDirection(util::Vector3Property::Create({0.f,1.f,0.f})),
	m_slopeLimit(util::FloatProperty::Create(CFloat(umath::cos(umath::deg_to_rad(45.0f))))),m_stepOffset(util::FloatProperty::Create(2.f)),
	m_jumpPower(util::FloatProperty::Create(0.f))
{}

const util::PFloatProperty &BaseCharacterComponent::GetSlopeLimitProperty() const {return m_slopeLimit;}
const util::PFloatProperty &BaseCharacterComponent::GetStepOffsetProperty() const {return m_stepOffset;}
const util::PVector3Property &BaseCharacterComponent::GetUpDirectionProperty() const {return m_upDirection;}

void BaseCharacterComponent::InitializePhysObj(PhysObj*)
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->AddCollisionFilter(CollisionMask::Character);
}

void BaseCharacterComponent::Initialize()
{
	BaseActorComponent::Initialize();
	m_netEvSetActiveWeapon = SetupNetEvent("set_active_weapon");
	m_netEvSetAmmoCount = SetupNetEvent("set_ammo_count");

	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		Think(static_cast<CEOnTick&>(evData.get()).deltaTime);
	});
	BindEventUnhandled(BaseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		return HandleAnimationEvent(static_cast<CEHandleAnimationEvent&>(evData.get()).animationEvent) ? util::EventReply::Handled : util::EventReply::Unhandled;
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PRE_PHYSICS_SIMULATE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateMovement();
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto t = GetEntity().GetNetworkState()->GetGameState()->CurTime();
		if(t >= m_tDetachFromGround)
			return;
		auto &ent = GetEntity();
		auto whPhysComponent = ent.GetPhysicsComponent();
		auto *phys = whPhysComponent.valid() ? whPhysComponent->GetPhysicsObject() : nullptr;
		if(phys == nullptr || phys->IsController() == false)
			return;
		auto *physController = static_cast<ControllerPhysObj*>(phys);
		auto *pInfo = physController->GetGroundContactInfo();
		if(pInfo == nullptr)
			return;
		auto whVelComponent = ent.GetComponent<pragma::VelocityComponent>();
		if(whVelComponent.valid())
		{
			auto vel = whVelComponent->GetVelocity();
			uvec::normalize(&vel);
			auto dot = uvec::dot(pInfo->GetContactNormal(),vel);
			if(dot <= 0.01f)
			{
				// Velocity leads towards the ground; This probably means we should allow attaching to the ground again.
				m_tDetachFromGround = 0.f;
				return;
			}
		}
		physController->ClearGroundContactPoint();
	});
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateNeckControllers();
	});
	auto &ent = GetEntity();
	ent.AddComponent("sound_emitter");
	ent.AddComponent("physics");
	ent.AddComponent<LogicComponent>();

#ifdef PHYS_ENGINE_PHYSX
	m_entity->SetCollisionCallbacksEnabled(true);
	m_entity->SetCollisionContactReportEnabled(true);
#endif
}

const Vector3 &BaseCharacterComponent::GetUpDirection() const {return *m_upDirection;}

void BaseCharacterComponent::GetOrientationAxes(Vector3 **forward,Vector3 **right,Vector3 **up)
{
	if(forward != NULL)
		*forward = &m_axForward;
	if(right != NULL)
		*right = &m_axRight;
	if(up != NULL)
		*up = &(**m_upDirection);
}
const Quat &BaseCharacterComponent::GetOrientationAxesRotation() const {return m_axRot;}

float BaseCharacterComponent::GetSlopeLimit() const {return *m_slopeLimit;}
void BaseCharacterComponent::SetSlopeLimit(float limit)
{
	*m_slopeLimit = limit;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys != NULL && phys->IsController())
	{
		ControllerPhysObj *physController = static_cast<ControllerPhysObj*>(phys);
		physController->SetMaxSlope(limit);
	}
}
float BaseCharacterComponent::GetStepOffset() const {return *m_stepOffset;}
void BaseCharacterComponent::SetStepOffset(float offset)
{
	*m_stepOffset = offset;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys != NULL && phys->IsController())
	{
		ControllerPhysObj *physController = static_cast<ControllerPhysObj*>(phys);
		physController->SetStepOffset(offset);
	}
}

const Quat &BaseCharacterComponent::GetViewOrientation() const {return m_angView;}

float BaseCharacterComponent::GetTurnSpeed() const {return m_turnSpeed;}
void BaseCharacterComponent::SetTurnSpeed(float speed) {m_turnSpeed = speed;}

void BaseCharacterComponent::SetNeckControllers(const std::string &yawController,const std::string &pitchController)
{
	m_yawControllerName = yawController;
	m_pitchControllerName = pitchController;
	UpdateNeckControllers();
}

int32_t BaseCharacterComponent::GetNeckYawBlendController() const {return m_yawController;}
int32_t BaseCharacterComponent::GetNeckPitchBlendController() const {return m_pitchController;}

void BaseCharacterComponent::UpdateNeckControllers()
{
	m_yawController = -1;
	m_pitchController = -1;
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return;
	m_yawController = hMdl->LookupBlendController(m_yawControllerName);
	m_pitchController = hMdl->LookupBlendController(m_pitchControllerName);
}

void BaseCharacterComponent::NormalizeViewOrientation(Quat &rot)
{
	auto &rotRel = GetOrientationAxesRotation();
	auto rotDst = rotRel *rot;
	auto ang = EulerAngles{rotDst};
	ang.r = 0.f;
	Quat rotCurNoRoll;
	if(ang.p < -135.f || ang.p > 135.f)
		rotCurNoRoll = uquat::create(ang) *uquat::create(EulerAngles(0.f,0.f,180.f));
	else
	{
		if(ang.p < -90.f)
			ang.p = -90.f;
		else if(ang.p > 90.f)
			ang.p = 90.f;
		rotCurNoRoll = uquat::create(ang);
	}
	rot = uquat::get_inverse(rotRel) *rotCurNoRoll;
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
	if(InvokeEventCallbacks(EVENT_HANDLE_VIEW_ROTATION,evData) == util::EventReply::Handled)
		return;

	auto &ent = GetEntity();
	if(ent.IsNPC())
		return; // NPCs are handled separately (See BaseNPC::LookAtStep).
	// TODO: This should probably go in player-specific code
	auto &rotRef = GetOrientationAxesRotation();
	auto rot = rotRef *orientation;
	EulerAngles angView(rot);
	auto animComponent = ent.GetAnimatedComponent();
	auto mdlComponent = ent.GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(animComponent.valid() && hMdl != nullptr)
	{
		auto pTrComponent = ent.GetTransformComponent();
		auto rotCur = pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity();
		auto angCur = EulerAngles(rotRef *rotCur);
		float pitchDelta = umath::get_angle_difference(angView.p,angCur.p);
		if(m_pitchController != -1)
			animComponent->SetBlendController(m_pitchController,pitchDelta);
		BlendController *yaw = hMdl->GetBlendController(m_yawController);
		if(yaw != NULL)
		{
			float yawDelta = umath::get_angle_difference(angView.y,angCur.y);
			if(yawDelta >= yaw->min && yawDelta <= yaw->max)
			{
				animComponent->SetBlendController(m_yawController,yawDelta);
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

EulerAngles BaseCharacterComponent::GetLocalOrientationAngles() const {return EulerAngles(GetLocalOrientationRotation());}
Quat BaseCharacterComponent::GetLocalOrientationRotation() const
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	return GetOrientationAxesRotation() *(pTrComponent.valid() ? pTrComponent->GetOrientation() : uquat::identity());
}
EulerAngles BaseCharacterComponent::GetLocalOrientationViewAngles() const {return EulerAngles(GetLocalOrientationViewRotation());}
Quat BaseCharacterComponent::GetLocalOrientationViewRotation() const {return GetOrientationAxesRotation() *GetViewOrientation();}

Quat BaseCharacterComponent::WorldToLocalOrientation(const Quat &rot) {return GetOrientationAxesRotation() *rot;}
EulerAngles BaseCharacterComponent::WorldToLocalOrientation(const EulerAngles &ang) {return EulerAngles(WorldToLocalOrientation(uquat::create(ang)));}
Quat BaseCharacterComponent::LocalOrientationToWorld(const Quat &rot) {return uquat::get_inverse(GetOrientationAxesRotation()) *rot;}
EulerAngles BaseCharacterComponent::LocalOrientationToWorld(const EulerAngles &ang) {return EulerAngles(LocalOrientationToWorld(uquat::create(ang)));}

bool BaseCharacterComponent::UpdateMovement()
{
	static auto bskip = false;
	if(bskip == true)
		return false;
	//if(m_entity->IsPlayer() == true)
	//	m_entity->GetNetworkState()->GetGameState()->HandlePlayerMovement();
	//if(true)
	//	return;
	if(CanMove() == false)
		return false;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || phys->IsController() == false)
		return false;
	auto mv = pPhysComponent->GetMoveType();
	if(mv == MOVETYPE::NONE || mv == MOVETYPE::PHYSICS)
		return false;
	auto *physController = static_cast<ControllerPhysObj*>(phys);
	auto pTrComponent = ent.GetTransformComponent();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	auto pos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	auto vel = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3{};

	auto bOnGround = pPhysComponent->IsOnGround();
	auto groundVelocity = physController->GetGroundVelocity();
	vel -= groundVelocity; // We only care about the local velocity; The ground velocity will be re-added later

	auto pSubmergibleComponent = ent.GetComponent<pragma::SubmergibleComponent>();
	auto bSubmerged = (pSubmergibleComponent.valid() && pSubmergibleComponent->GetSubmergedFraction() > 0.5f) ? true : false;
	Vector3 *axisForward;
	Vector3 *axisRight;
	Vector3 *axisUp;
	GetOrientationAxes(&axisForward,&axisRight,&axisUp);
	Quat rot = GetViewOrientation();
	Quat viewRot = GetOrientationAxesRotation();
	Quat invViewRot = uquat::get_inverse(viewRot);

	Vector3 forward = uquat::forward(rot);
	Vector3 right = uquat::right(rot);
	if(mv == MOVETYPE::WALK && bSubmerged == false)
	{
		// No movement on up-axis
		auto &upDir = GetUpDirection();
		auto angle = uvec::dot(forward,upDir);
		if(umath::abs(angle) < 0.99f)
			forward = uvec::project_to_plane(forward,upDir,0.f);
		else // Looking straight up or down; Use camera up-direction as forward-direction instead
		{
			auto rotUpDir = uquat::up(rot);
			if(angle > 0.f)
				rotUpDir = -rotUpDir;
			forward = uvec::project_to_plane(rotUpDir,upDir,0.f);
		}
		right = uvec::project_to_plane(right,upDir,0.f);

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
	auto scale = pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};
	auto speed = CalcMovementSpeed() *ts *umath::abs_max(scale.x,scale.y,scale.z);
	
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto acceleration = CalcMovementAcceleration();
	auto tDelta = CFloat(game->DeltaTickTime()) *ts;

	{
		//if(phys != nullptr && phys->IsController())
		{
			auto *groundObject = physController->GetGroundPhysCollisionObject();
			if(groundObject != nullptr && groundObject->IsRigid())
			{
				//if(owner->GetNetworkState()->IsClient())
				if(ent.IsNPC() || ent.GetNetworkState()->IsClient())
				{
				auto angVel = static_cast<PhysRigidBody*>(groundObject)->GetAngularVelocity() *static_cast<float>(tDelta);
				auto ang = EulerAngles(umath::rad_to_deg(angVel.x),umath::rad_to_deg(angVel.y),umath::rad_to_deg(angVel.z));
				//owner->SetAngles(owner->GetAngles() +ang);
				//SetViewAngles(GetViewAngles() +ang);
				}
			}
		}
	}

	auto *pGroundContactInfo = physController->GetGroundContactInfo();
	if(pPhysComponent->IsGroundWalkable() || mv != MOVETYPE::WALK || bSubmerged == true)
	{
		auto friction = 0.8f;
		if(phys != nullptr && phys->IsController())
		{
			auto *controller = static_cast<ControllerPhysObj*>(phys);
			friction = controller->GetCurrentFriction();
			controller->SetCurrentFriction(1.f); // Reset
		}
		auto frictionVel = vel;
		if(pGroundContactInfo != nullptr) // Only apply friction to the component of the velocity which is parallel to the ground (i.e. jumping and such remain unaffected)
			frictionVel = uvec::project_to_plane(frictionVel,pGroundContactInfo->GetContactNormal(),0.f);
		auto frictionForce = -frictionVel *friction;
		//if(ent.IsPlayer())
		//	Con::cout<<"Player friction: "<<friction<<Con::endl;
		//else
		//	Con::cout<<"NPC friction: "<<friction<<Con::endl;

		vel += frictionForce *umath::min(tDelta *acceleration,1.f);
	}
	else
		speed *= CalcAirMovementModifier();

	Vector3 dir = Vector3(0,0,0);
	if(IsFrozen() == false)
		dir = CalcMovementDirection(forward,right);
	auto l = uvec::length(dir);
	if(l > 0.f)
		dir /= l;

	auto speedDir = glm::dot(dir,vel); // The speed in the movement direction of the current velocity
	if(speedDir < umath::abs(speed.x))
	{
		auto speedDelta = speed.x -speedDir;
		vel += dir *umath::min(speedDelta *tDelta *acceleration,speedDelta);
	}

	// Calculate sideways movement speed (NPC animation movement only)
	if(speed.y != 0.f)
	{
		auto dirRight = (uvec::length_sqr(dir) > 0.99f) ? uvec::cross(dir,pTrComponent.valid() ? pTrComponent->GetUp() : uvec::UP) : (pTrComponent.valid() ? pTrComponent->GetRight() : uvec::RIGHT);
		auto speedDir = glm::dot(dirRight,vel);
		if(speedDir < umath::abs(speed.y))
		{
			auto speedDelta = speed.y -speedDir;
			vel += dirRight *umath::min(speedDelta *tDelta *acceleration,speedDelta);
		}
	}
	//static PhysContactInfo lastGroundContact = {btManifoldPoint{}};
	m_moveVelocity = vel;


	//static auto forceTest = 100.f;
	//vel -= uvec::create(lastGroundContact.contactPoint.m_normalWorldOnB) *forceTest;
	//auto *pGroundContactInfo = physController->GetGroundContactInfo();
	//if(pGroundContactInfo != nullptr)
	//	lastGroundContact = *pGroundContactInfo;


	if(pVelComponent.valid())
	{
		vel += groundVelocity; // Re-add ground velocity
		pVelComponent->SetVelocity(vel);
	}
	//return controller->Move(disp,elapsedTime,minDist);

	const auto threshold = -0.03;
	auto localVel = vel;
	uvec::rotate(&localVel,viewRot);
	if(
		vel.y <= 0.1f && physController->IsGroundWalkable() &&
		(pGroundContactInfo != nullptr && pGroundContactInfo->contactPoint.getDistance() >= threshold) &&
		mv == MOVETYPE::WALK
	)
	{
		auto &info = *pGroundContactInfo;
		auto pos = uvec::create((info.controllerIndex == 0u ? info.contactPoint.getPositionWorldOnA() : info.contactPoint.getPositionWorldOnB()) /PhysEnv::WORLD_SCALE);
		auto n = -info.GetContactNormal();
		//ent.GetNetworkState()->GetGameState()->DrawLine(
		//	pos,pos -n *100.f,Color::Red,5.f
		//);
		
		const auto force = 100.f; // Somewhat arbitrary; The force to apply to the controller to make them stick to the ground
		physController->AddLinearVelocity(n *force);
	}

	return true;
}

const Vector3 &BaseCharacterComponent::GetMoveVelocity() const {return m_moveVelocity;}
void BaseCharacterComponent::SetMoveVelocity(const Vector3 &vel) {m_moveVelocity = vel;}
Vector3 BaseCharacterComponent::GetLocalVelocity() const
{
	auto &ent = GetEntity();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(pVelComponent.expired())
		return Vector3{};
	auto vel = pVelComponent->GetVelocity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys != nullptr && phys->IsController())
	{
		auto *physController = static_cast<ControllerPhysObj*>(phys);
		vel -= physController->GetGroundVelocity();
	}
	return vel;
}

float BaseCharacterComponent::GetMovementBlendScale() const
{
	auto blendScale = 0.f;
	auto vel = GetLocalVelocity();
	float speed = uvec::length(vel);
	auto mvSpeed = CalcMovementSpeed();
	auto speedMax = umath::max(mvSpeed.x,mvSpeed.y);
	if(speedMax == 0.f)
		blendScale = 0.f;
	else
	{
		blendScale = 1.f -speed /speedMax;
		if(blendScale > 1.f)
			blendScale = 1.f;
		else if(blendScale < 0.f)
			blendScale = 0.f;
	}
	return blendScale;
}

bool BaseCharacterComponent::HandleAnimationEvent(const AnimationEvent &ev)
{
	switch(ev.eventID)
	{
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
	auto *ammo = game->GetAmmoType(ammoType,&ammoId);
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
void BaseCharacterComponent::SetAmmoCount(const std::string &ammoType,UInt16 count)
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	UInt32 ammoId = 0;
	auto *ammo = game->GetAmmoType(ammoType,&ammoId);
	if(ammo == nullptr)
		return;
	SetAmmoCount(ammoId,count);
}
void BaseCharacterComponent::SetAmmoCount(UInt32 ammoType,UInt16 count)
{
	auto it = m_ammoCount.find(ammoType);
	if(it != m_ammoCount.end())
	{
		it->second = count;
		return;
	}
	m_ammoCount.insert(std::unordered_map<UInt32,UInt16>::value_type(ammoType,count));
}
void BaseCharacterComponent::AddAmmo(const std::string &ammoType,int16_t count)
{
	if(count < 0)
	{
		RemoveAmmo(ammoType,-count);
		return;
	}
	SetAmmoCount(ammoType,GetAmmoCount(ammoType) +count);
}
void BaseCharacterComponent::AddAmmo(UInt32 ammoType,int16_t count)
{
	if(count < 0)
	{
		RemoveAmmo(ammoType,-count);
		return;
	}
	SetAmmoCount(ammoType,GetAmmoCount(ammoType) +count);
}
void BaseCharacterComponent::RemoveAmmo(const std::string &ammoType,int16_t count)
{
	if(count < 0)
	{
		AddAmmo(ammoType,-count);
		return;
	}
	SetAmmoCount(ammoType,umath::max(static_cast<int32_t>(GetAmmoCount(ammoType)) -static_cast<int32_t>(count),0));
}
void BaseCharacterComponent::RemoveAmmo(UInt32 ammoType,int16_t count)
{
	if(count < 0)
	{
		AddAmmo(ammoType,-count);
		return;
	}
	SetAmmoCount(ammoType,umath::max(static_cast<int32_t>(GetAmmoCount(ammoType)) -static_cast<int32_t>(count),0));
}


void BaseCharacterComponent::PlayFootStepSound(FootType foot,const SurfaceMaterial &surfMat,float scale)
{
	CEPlayFootstepSound footStepInfo {foot,surfMat,scale};
	if(BroadcastEvent(EVENT_PLAY_FOOTSTEP_SOUND,footStepInfo) == util::EventReply::Handled)
		return;
	auto pSoundEmitterComponent = static_cast<pragma::BaseSoundEmitterComponent*>(GetEntity().FindComponent("sound_emitter").get());
	if(pSoundEmitterComponent == nullptr)
		return;
	const auto maxGain = 0.5f;
	auto &ent = GetEntity();
	auto soundType = ALSoundType::Effect;
	soundType |= (ent.IsPlayer() == true) ? ALSoundType::Player : ALSoundType::NPC;
	auto pSubmergibleComponent = ent.GetComponent<SubmergibleComponent>();
	if(pSubmergibleComponent.valid() && pSubmergibleComponent->IsSubmerged() == true)
		return; // Don't play footsteps when underwater
	if(pSubmergibleComponent.valid() == false || pSubmergibleComponent->GetSubmergedFraction() == 0.f)
		pSoundEmitterComponent->EmitSharedSound(surfMat.GetFootstepType(),soundType,maxGain *scale,1.f);
	else
	{
		// Play water footstep sounds if we're knee-deep in water
		auto *pSurfWaterMat = ent.GetNetworkState()->GetGameState()->GetSurfaceMaterial("water");
		if(pSurfWaterMat != nullptr)
			pSoundEmitterComponent->EmitSharedSound(pSurfWaterMat->GetFootstepType(),soundType,maxGain *scale,1.f);
	}
}

bool BaseCharacterComponent::IsCharacter() const {return true;}
bool BaseCharacterComponent::IsMoving() const
{
	CEIsMoving evData {};
	InvokeEventCallbacks(EVENT_IS_MOVING,evData);
	return evData.moving;
}

void BaseCharacterComponent::FootStep(FootType foot)
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.expired() || pPhysComponent->IsOnGround() == false)
		return;

	CEOnFootStep footStepInfo {foot};
	BroadcastEvent(EVENT_ON_FOOT_STEP,footStepInfo);

	auto moveScale = 1.f;
	if(IsMoving() == true)
	{
		moveScale = 1.f -GetMovementBlendScale();
		if(moveScale < 0.1f)//0.25f)
			return;
	}
	auto *phys = static_cast<ControllerPhysObj*>(pPhysComponent->GetPhysicsObject());
	if(phys == nullptr || !phys->IsController())
		return;
	auto id = phys->GetGroundSurfaceMaterial();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto *mat = game->GetSurfaceMaterial(id);
	if(mat == nullptr)
		return;
	PlayFootStepSound(foot,*mat,moveScale); // TODO: Is Moving -> Blend move scale -> Same as player!
}
TraceData BaseCharacterComponent::GetAimTraceData() const
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.expired())
		return {};
	auto trData = util::get_entity_trace_data(*pTrComponent);
	auto origin = GetEyePosition();
	auto dir = GetViewForward();
	trData.SetSource(origin);
	trData.SetTarget(origin +dir *static_cast<float>(GameLimits::MaxRayCastRange));
	// See also: ControllerPhysObj::PostSimulate
	return trData;
}
bool BaseCharacterComponent::CanMove() const
{
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	auto mvType = pPhysComponent.valid() ? pPhysComponent->GetMoveType() : MOVETYPE::NONE;
	if(pPhysComponent.expired() || mvType == MOVETYPE::NONE)
		return false;
	auto *pPhysObj = pPhysComponent->GetPhysicsObject();
	return pPhysObj != nullptr && pPhysObj->IsController();
}
Vector2 BaseCharacterComponent::CalcMovementSpeed() const
{
	CECalcMovementSpeed evData {};
	if(InvokeEventCallbacks(EVENT_CALC_MOVEMENT_SPEED,evData) == util::EventReply::Handled)
		return evData.speed;
	return Vector2{};
}
float BaseCharacterComponent::CalcAirMovementModifier() const
{
	CECalcAirMovementModifier evData {};
	if(InvokeEventCallbacks(EVENT_CALC_AIR_MOVEMENT_MODIFIER,evData) == util::EventReply::Handled)
		return evData.airMovementModifier;
	return 0.f;
}
float BaseCharacterComponent::CalcMovementAcceleration() const
{
	CECalcMovementAcceleration evData {};
	if(InvokeEventCallbacks(EVENT_CALC_MOVEMENT_ACCELERATION,evData) == util::EventReply::Handled)
		return evData.acceleration;
	return 0.f;
}

Vector3 BaseCharacterComponent::CalcMovementDirection(const Vector3 &forward,const Vector3 &right) const
{
	CECalcMovementDirection evData {forward,right};
	if(InvokeEventCallbacks(EVENT_CALC_MOVEMENT_DIRECTION,evData) == util::EventReply::Handled)
		return evData.direction;
	return Vector3{};
}

EulerAngles BaseCharacterComponent::GetViewAngles() const {return EulerAngles(m_angView);}
void BaseCharacterComponent::SetViewAngles(const EulerAngles &ang) {SetViewOrientation(uquat::create(ang));}

void BaseCharacterComponent::Think(double tDelta)
{
	UpdateOrientation();
	if(m_turnYaw != nullptr)
	{
		auto &ent = GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		if(pTrComponent.valid())
		{
			auto pPhysComponent = ent.GetPhysicsComponent();
			auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
			if(phys == nullptr || phys->IsController())
			{
				auto &rotRef = GetOrientationAxesRotation();
				auto &rotCur = pTrComponent->GetOrientation();
				auto rot = rotRef *rotCur;
				auto angCur = EulerAngles(rot);
				angCur.y = umath::approach_angle(angCur.y,*m_turnYaw,m_turnSpeed *CFloat(tDelta));
				rot = uquat::get_inverse(rotRef) *uquat::create(angCur);
				pTrComponent->SetOrientation(rot);
			}
		}
	}
	UpdateMoveController();
}

Vector3 BaseCharacterComponent::GetViewForward() const {return uquat::forward(m_angView);}
Vector3 BaseCharacterComponent::GetViewRight() const {return uquat::right(m_angView);}
Vector3 BaseCharacterComponent::GetViewUp() const {return uquat::up(m_angView);}

void BaseCharacterComponent::GetViewOrientation(Vector3 *forward,Vector3 *right,Vector3 *up) const {uquat::get_orientation(m_angView,forward,right,up);}

Vector3 BaseCharacterComponent::GetEyePosition() const
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.expired())
		return Vector3{};
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto physType = pPhysComponent.valid() ? pPhysComponent->GetPhysicsType() : PHYSICSTYPE::NONE;
	if(physType != PHYSICSTYPE::BOXCONTROLLER && physType != PHYSICSTYPE::CAPSULECONTROLLER)
		return pTrComponent->GetPosition();
	Vector3 eyeOffset = pTrComponent->GetEyeOffset();
	Vector3 forward,right,up;
	GetViewOrientation(&forward,&right);
	up = pTrComponent->GetUp();
	Vector3 eyePos = pTrComponent->GetPosition();
	eyePos = eyePos +forward *eyeOffset.x +up *eyeOffset.y +right *eyeOffset.z;
	return eyePos;
}

Vector3 BaseCharacterComponent::GetShootPosition() const {return GetEyePosition();}

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
	if(BroadcastEvent(EVENT_ON_JUMP,footStepInfo) == util::EventReply::Handled)
		return true;
	return true;
}
bool BaseCharacterComponent::Jump()
{
	if(GetJumpPower() == 0.f)
		return false;
	auto &ent = GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	auto upDir = (charComponent.valid()) ? charComponent->GetUpDirection() : uvec::UP;
	auto vel = upDir *(m_jumpPower->GetValue() *(pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f}));
	return Jump(vel);
}

const util::PFloatProperty &BaseCharacterComponent::GetJumpPowerProperty() const {return m_jumpPower;}
float BaseCharacterComponent::GetJumpPower() const {return *m_jumpPower;}
void BaseCharacterComponent::SetJumpPower(float power) {*m_jumpPower = power;}
bool BaseCharacterComponent::CanJump() const
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	return pPhysComponent.valid() && pPhysComponent->IsOnGround() == true && pVelComponent.valid();
}
void BaseCharacterComponent::DetachFromGround(float duration)
{
	m_tDetachFromGround = GetEntity().GetNetworkState()->GetGameState()->CurTime() +duration;
}

//////////////////

CEOnDeployWeapon::CEOnDeployWeapon(BaseEntity &entWeapon)
	: weapon{entWeapon}
{}
void CEOnDeployWeapon::PushArguments(lua_State *l)
{
	weapon.GetLuaObject()->push(l);
}

//////////////////

CEOnSetActiveWeapon::CEOnSetActiveWeapon(BaseEntity *entWeapon)
	: weapon{entWeapon}
{}
void CEOnSetActiveWeapon::PushArguments(lua_State *l)
{
	if(weapon != nullptr)
		weapon->GetLuaObject()->push(l);
	else
		Lua::PushNil(l);
}

//////////////////

CEOnSetCharacterOrientation::CEOnSetCharacterOrientation(const Vector3 &up)
	: up{up}
{}
void CEOnSetCharacterOrientation::PushArguments(lua_State *l)
{
	Lua::Push<Vector3>(l,up);
}

//////////////////

CEPlayFootstepSound::CEPlayFootstepSound(BaseCharacterComponent::FootType footType,const SurfaceMaterial &surfaceMaterial,float scale)
	: footType(footType),surfaceMaterial(surfaceMaterial),scale(scale)
{}
void CEPlayFootstepSound::PushArguments(lua_State *l)
{
	Lua::PushInt(l,umath::to_integral(footType));
	Lua::Push<SurfaceMaterial*>(l,const_cast<SurfaceMaterial*>(&surfaceMaterial));
	Lua::PushNumber(l,scale);
}

//////////////////

CEOnFootStep::CEOnFootStep(BaseCharacterComponent::FootType footType)
	: footType{footType}
{}
void CEOnFootStep::PushArguments(lua_State *l)
{
	Lua::PushInt(l,umath::to_integral(footType));
}

//////////////////

CEOnJump::CEOnJump(const Vector3 &velocity)
	: velocity{velocity}
{}
void CEOnJump::PushArguments(lua_State *l)
{
	Lua::Push<Vector3>(l,velocity);
}

//////////////////

CECalcMovementSpeed::CECalcMovementSpeed()
{}
void CECalcMovementSpeed::PushArguments(lua_State *l) {}
uint32_t CECalcMovementSpeed::GetReturnCount() {return 1;}
void CECalcMovementSpeed::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		speed = *Lua::CheckVector2(l,-1);
}

//////////////////

CECalcAirMovementModifier::CECalcAirMovementModifier()
{}
void CECalcAirMovementModifier::PushArguments(lua_State *l) {}
uint32_t CECalcAirMovementModifier::GetReturnCount() {return 1;}
void CECalcAirMovementModifier::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		airMovementModifier = Lua::CheckNumber(l,-1);
}

//////////////////

CECalcMovementAcceleration::CECalcMovementAcceleration()
{}
void CECalcMovementAcceleration::PushArguments(lua_State *l) {}
uint32_t CECalcMovementAcceleration::GetReturnCount() {return 1;}
void CECalcMovementAcceleration::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		acceleration = Lua::CheckNumber(l,-1);
}

//////////////////

CECalcMovementDirection::CECalcMovementDirection(const Vector3 &forward,const Vector3 &right)
	: forward(forward),right(right)
{}
void CECalcMovementDirection::PushArguments(lua_State *l)
{
	Lua::Push<Vector3>(l,forward);
	Lua::Push<Vector3>(l,right);
}
uint32_t CECalcMovementDirection::GetReturnCount() {return 1;}
void CECalcMovementDirection::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		direction = *Lua::CheckVector(l,-1);
}

//////////////////

CEIsMoving::CEIsMoving()
{}
void CEIsMoving::PushArguments(lua_State *l) {Lua::PushBool(l,moving);}
uint32_t CEIsMoving::GetReturnCount() {return 1;}
void CEIsMoving::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		moving = Lua::CheckBool(l,-1);
}

//////////////////

CEViewRotation::CEViewRotation(const Quat &rotation)
	: rotation{rotation}
{}
void CEViewRotation::PushArguments(lua_State *l)
{
	Lua::Push<Quat>(l,rotation);
}
#pragma optimize("",on)
