/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/game/game_limits.h"
#include "pragma/util/bulletinfo.h"
#include "pragma/physics/environment.hpp"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/model/model.h"
#include "pragma/physics/raytraces.h"
#include "pragma/logging.hpp"
#include <sharedutils/netpacket.hpp>
#include <pragma/physics/movetypes.h>
#include <pragma/physics/collisiontypes.h>
#include <udm.hpp>
#include <panima/bone.hpp>
#include <panima/skeleton.hpp>

using namespace pragma;

ComponentEventId BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED = INVALID_COMPONENT_ID;
ComponentEventId BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED = INVALID_COMPONENT_ID;
ComponentEventId BasePhysicsComponent::EVENT_ON_PHYSICS_UPDATED = INVALID_COMPONENT_ID;
ComponentEventId BasePhysicsComponent::EVENT_ON_DYNAMIC_PHYSICS_UPDATED = INVALID_COMPONENT_ID;
ComponentEventId BasePhysicsComponent::EVENT_ON_PRE_PHYSICS_SIMULATE = INVALID_COMPONENT_ID;
ComponentEventId BasePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE = INVALID_COMPONENT_ID;
ComponentEventId BasePhysicsComponent::EVENT_ON_SLEEP = INVALID_COMPONENT_ID;
ComponentEventId BasePhysicsComponent::EVENT_ON_WAKE = INVALID_COMPONENT_ID;
ComponentEventId BasePhysicsComponent::EVENT_HANDLE_RAYCAST = INVALID_COMPONENT_ID;
ComponentEventId BasePhysicsComponent::EVENT_INITIALIZE_PHYSICS = INVALID_COMPONENT_ID;

void BasePhysicsComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_PHYSICS_INITIALIZED = registerEvent("ON_PHYSICS_INITIALIZED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_PHYSICS_DESTROYED = registerEvent("ON_PHYSICS_DESTROYED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_PHYSICS_UPDATED = registerEvent("ON_PHYSICS_UPDATED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_DYNAMIC_PHYSICS_UPDATED = registerEvent("ON_DYNAMIC_PHYSICS_UPDATED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_PRE_PHYSICS_SIMULATE = registerEvent("ON_PRE_PHYSICS_SIMULATE", ComponentEventInfo::Type::Explicit);
	EVENT_ON_POST_PHYSICS_SIMULATE = registerEvent("ON_POST_PHYSICS_SIMULATE", ComponentEventInfo::Type::Explicit);
	EVENT_ON_SLEEP = registerEvent("EVENT_ON_SLEEP", ComponentEventInfo::Type::Explicit);
	EVENT_ON_WAKE = registerEvent("EVENT_ON_WAKE", ComponentEventInfo::Type::Explicit);
	EVENT_HANDLE_RAYCAST = registerEvent("HANDLE_RAYCAST", ComponentEventInfo::Type::Explicit);
	EVENT_INITIALIZE_PHYSICS = registerEvent("INITIALIZE_PHYSICS", ComponentEventInfo::Type::Broadcast);
}
BasePhysicsComponent::BasePhysicsComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_collisionType(COLLISIONTYPE::NONE), m_moveType(MOVETYPE::NONE) {}
void BasePhysicsComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	DestroyPhysicsObject();
	ClearAwakeStatus();
}
const std::vector<BasePhysicsComponent::PhysJoint> &BasePhysicsComponent::GetJoints() const { return const_cast<BasePhysicsComponent *>(this)->GetJoints(); }
std::vector<BasePhysicsComponent::PhysJoint> &BasePhysicsComponent::GetJoints() { return m_joints; }
void BasePhysicsComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetCollisionsEnabled = SetupNetEvent("set_collisions_enabled");
	m_netEvSetSimEnabled = SetupNetEvent("set_simulation_enabled");

	BindEvent(BaseAnimatedComponent::EVENT_SHOULD_UPDATE_BONES, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		if(IsRagdoll()) {
			static_cast<CEShouldUpdateBones &>(evData.get()).shouldUpdate = true;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(BaseAnimatedComponent::EVENT_ON_BONE_TRANSFORM_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &evDataTransform = static_cast<CEOnBoneTransformChanged &>(evData.get());
		UpdateBoneCollisionObject(evDataTransform.boneId, evDataTransform.pos != nullptr, evDataTransform.rot != nullptr);
	});
	BindEvent(BaseAnimatedComponent::EVENT_MAINTAIN_ANIMATIONS, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		return IsRagdoll() ? util::EventReply::Handled : util::EventReply::Unhandled; // Don't process animations if we're in ragdoll mode
	});
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		uvec::zero(&m_colMin);
		uvec::zero(&m_colMax);
		auto &mdl = static_cast<CEOnModelChanged &>(evData.get()).model;
		if(mdl.get() == nullptr)
			return;
		Vector3 cMin, cMax;
		mdl.get()->GetCollisionBounds(cMin, cMax);
		SetCollisionBounds(cMin, cMax);
	});

	auto &ent = GetEntity();
	ent.AddComponent("transform");
}
Vector3 BasePhysicsComponent::GetCenter() const
{
	auto trComponent = GetEntity().GetTransformComponent();
	if(!trComponent)
		return uvec::ORIGIN;
	auto &pos = trComponent->GetPosition();
	auto colCenter = GetCollisionCenter();
	uvec::rotate(&colCenter, trComponent->GetRotation());
	return pos + colCenter;
}
void BasePhysicsComponent::Sweep(const Vector3 &, float) const {}
float BasePhysicsComponent::GetAABBDistance(const Vector3 &p) const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto origin = pTrComponent ? pTrComponent->GetPosition() : Vector3 {};
	Vector3 min, max;
	GetCollisionBounds(&min, &max);
	min += origin;
	max += origin;

	Vector3 r {};
	umath::geometry::closest_point_on_aabb_to_point(min, max, p, &r);
	return uvec::distance(r, p);
}
float BasePhysicsComponent::GetAABBDistance(const BaseEntity &ent) const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto origin0 = pTrComponent ? pTrComponent->GetPosition() : Vector3 {};

	auto pTrComponentEnt = ent.GetTransformComponent();
	auto origin1 = pTrComponentEnt ? pTrComponentEnt->GetPosition() : Vector3 {};

	Vector3 min0, max0;
	GetCollisionBounds(&min0, &max0);
	min0 += origin0;
	max0 += origin0;

	auto pPhysComponentEnt = ent.GetPhysicsComponent();
	Vector3 min1 {};
	Vector3 max1 {};
	if(pPhysComponentEnt)
		pPhysComponentEnt->GetCollisionBounds(&min1, &max1);
	min1 += origin1;
	max1 += origin1;

	Vector3 r0 {};
	umath::geometry::closest_point_on_aabb_to_point(min0, max0, origin1, &r0);

	Vector3 r1 {};
	umath::geometry::closest_point_on_aabb_to_point(min1, max1, origin0, &r1);

	return uvec::distance(r0, r1);
}

void BasePhysicsComponent::UpdatePhysicsData()
{
	PHYSICSTYPE type = GetPhysicsType();
	if(type == PHYSICSTYPE::NONE)
		return;
	PhysObj *phys = GetPhysicsObject();
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(phys == nullptr) {
		if(pVelComponent.valid())
			pVelComponent->SetVelocity({});
		return;
	}
	//if(phys->IsSleeping())
	//	return;
	auto *o = phys->GetCollisionObject();
	if(o == NULL)
		return;
	phys->UpdateVelocity();
	if(pVelComponent.valid()) {
		auto linVel = phys->GetLinearVelocity();
		// Sanity check
		if(std::isnan(linVel.x) || std::isnan(linVel.y) || std::isnan(linVel.z)) {
			spdlog::warn("UpdatePhysicsData: NaN linear velocity ({},{},{}) for entity {}! Forcing to 0.", linVel.x, linVel.y, linVel.z, ent.ToString());
			phys->SetLinearVelocity({});
			linVel = {};
		}
		umath::set_flag(m_stateFlags, StateFlags::ApplyingLinearVelocity);
		pVelComponent->SetRawVelocity(linVel);
		umath::set_flag(m_stateFlags, StateFlags::ApplyingLinearVelocity, false);
	}

	bool bStatic = phys->IsStatic();
	bool bSnapshot = false;
	auto t = o->GetWorldTransform();
	Vector3 pos = phys->GetPosition();
	Quat rot = t.GetRotation();

	auto transformChangeFlags = TransformChangeFlags::None;
	if(!m_physObject->IsController() && pTrComponent) // TODO
	{
		auto &rotCur = pTrComponent->GetRotation();
		if(fabsf(rot.w - rotCur.w) > ENT_EPSILON || fabsf(rot.x - rotCur.x) > ENT_EPSILON || fabsf(rot.y - rotCur.y) > ENT_EPSILON || fabsf(rot.z - rotCur.z) > ENT_EPSILON) {
			ent.SetStateFlag(BaseEntity::StateFlags::RotationChanged);
			bSnapshot = true;

			// Sanity check
			if(std::isnan(rot.w) || std::isnan(rot.x) || std::isnan(rot.y) || std::isnan(rot.z)) {
				std::stringstream ss {};
				ss << "UpdatePhysicsData: NaN rotation (" << rot.w << "," << rot.x << "," << rot.y << "," << rot.z << ") for entity " << ent.GetClass() << "!";
				spdlog::error(ss.str());
				throw std::runtime_error(ss.str());
			}
			umath::set_flag(m_stateFlags, StateFlags::ApplyingPhysicsRotation);
			pTrComponent->SetRawRotation(rot);
			transformChangeFlags |= TransformChangeFlags::RotationChanged;
			umath::set_flag(m_stateFlags, StateFlags::ApplyingPhysicsRotation, false);

			if(!bStatic && pVelComponent.valid()) {
				auto angVel = phys->GetAngularVelocity();
				// Sanity check
				if(std::isnan(angVel.x) || std::isnan(angVel.y) || std::isnan(angVel.z)) {
					std::stringstream ss {};
					ss << "UpdatePhysicsData: NaN angular velocity (" << angVel.x << "," << angVel.y << "," << angVel.z << ") for entity " << ent.GetClass() << "!";
					spdlog::error(ss.str());
					throw std::runtime_error(ss.str());
				}
				umath::set_flag(m_stateFlags, StateFlags::ApplyingAngularVelocity);
				pVelComponent->SetRawAngularVelocity(angVel);
				umath::set_flag(m_stateFlags, StateFlags::ApplyingAngularVelocity, false);
			}
		}
	}
	if(pTrComponent) {
		auto &posCur = pTrComponent->GetPosition();
		if(fabsf(pos.x - posCur.x) > ENT_EPSILON || fabsf(pos.y - posCur.y) > ENT_EPSILON || fabsf(pos.z - posCur.z) > ENT_EPSILON) {
			ent.SetStateFlag(BaseEntity::StateFlags::PositionChanged);
			pTrComponent->UpdateLastMovedTime();
			bSnapshot = true;

			// Sanity check
			if(std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z)) {
				std::stringstream ss {};
				ss << "UpdatePhysicsData: NaN position (" << pos.x << "," << pos.y << "," << pos.z << ") for entity " << ent.GetClass() << "!";
				spdlog::error(ss.str());
				throw std::runtime_error(ss.str());
			}
			umath::set_flag(m_stateFlags, StateFlags::ApplyingPhysicsPosition);
			pTrComponent->SetRawPosition(pos);
			transformChangeFlags |= TransformChangeFlags::PositionChanged;
			umath::set_flag(m_stateFlags, StateFlags::ApplyingPhysicsPosition, false);
		}
	}
	if(type == PHYSICSTYPE::DYNAMIC) {
		/*RigidPhysObj *phys = static_cast<RigidPhysObj*>(m_physObject);
		std::vector<DynamicActorInfo> &actorInfo = phys->GetActorInfo();
		Model *mdl = GetModel();
		Frame *frame;
		Skeleton *skeleton;
		if(mdl != NULL)
		{
			Animation *ref = mdl->GetAnimation(0);
			frame = ref->GetFrame(0);
			skeleton = mdl->GetSkeleton();
		}*/
#pragma message("TODO: Do this properly!")
#pragma message("TODOBULLET")
		/*for(unsigned int i=1;i<actorInfo.size();i++)
		{
			DynamicActorInfo &info = actorInfo[i];
			int boneID = info.GetBoneID();
			physx::PxRigidDynamic *actor = info.GetActor();
			physx::PxTransform t = actor->getGlobalPose();
			Vector3 posActor = Vector3(t.p.x,t.p.y,t.p.z);
			Quat rotActor = Quat(t.q.w,t.q.x,t.q.y,t.q.z);

			if(mdl != NULL)
			{
				Vector3 posBind = *frame->GetBonePosition(boneID);
				Quat rotBind = *frame->GetBoneOrientation(boneID);

				posActor -= rotActor *Quat(Angle(-Vector3::getNormal(posBind))).Forward() *glm::length(posBind);
				posActor = (posActor -(*m_pos)) *(*m_orientation);

				rotActor = rotActor *rotBind;
				rotActor = glm::inverse(*m_orientation) *rotActor;

				SetBonePosition(boneID,posActor,rotActor);
			}
		}*/
	}
	if(bSnapshot)
		ent.MarkForSnapshot(true);
	if(transformChangeFlags != TransformChangeFlags::None)
		pTrComponent->OnPoseChanged(transformChangeFlags, false);
}

BaseEntity *BasePhysicsComponent::GetGroundEntity() const { return nullptr; }

void BasePhysicsComponent::SetCollisionCallbacksEnabled(bool b) { m_bColCallbacksEnabled = b; }
void BasePhysicsComponent::SetCollisionContactReportEnabled(bool b)
{
	m_bColContactReportEnabled = b;
	auto *phys = GetPhysicsObject();
	for(auto &hColObj : phys->GetCollisionObjects()) {
		if(hColObj.IsValid() == false)
			continue;
		hColObj->SetContactReportEnabled(b);
	}
}
bool BasePhysicsComponent::GetCollisionCallbacksEnabled() const { return m_bColCallbacksEnabled; }

bool BasePhysicsComponent::GetCollisionContactReportEnabled() const { return m_bColContactReportEnabled; }

void BasePhysicsComponent::SetCollisionsEnabled(bool b)
{
	if(b == GetCollisionsEnabled())
		return;
	umath::set_flag(m_stateFlags, StateFlags::CollisionsEnabled, b);
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	auto &hColObjs = phys->GetCollisionObjects();
	for(auto &hCol : hColObjs) {
		if(!hCol.IsValid())
			continue;
		hCol->SetCollisionsEnabled(b);
	}
}
bool BasePhysicsComponent::GetCollisionsEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::CollisionsEnabled); }
void BasePhysicsComponent::SetSimulationEnabled(bool b)
{
	if(b == GetSimulationEnabled())
		return;
	umath::set_flag(m_stateFlags, StateFlags::SimulationEnabled, b);
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	auto &hColObjs = phys->GetCollisionObjects();
	for(auto &hCol : hColObjs) {
		if(!hCol.IsValid())
			continue;
		hCol->SetSimulationEnabled(b);
	}
}
bool BasePhysicsComponent::GetSimulationEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::SimulationEnabled); }
bool BasePhysicsComponent::IsTrigger() const
{
	PhysObj *phys = GetPhysicsObject();
	if(phys == NULL)
		return false;
	return phys->IsTrigger();
}

void BasePhysicsComponent::SetCollisionFilter(CollisionMask filterGroup, CollisionMask filterMask)
{
	m_collisionFilterGroup = filterGroup;
	m_collisionFilterMask = filterMask;
	PhysObj *phys = GetPhysicsObject();
	if(phys == NULL)
		return;
	phys->SetCollisionFilter(filterGroup, filterMask);
}
void BasePhysicsComponent::AddCollisionFilter(CollisionMask filter)
{
	CollisionMask filterGroup;
	CollisionMask filterMask;
	GetCollisionFilter(&filterGroup, &filterMask);
	SetCollisionFilter(filterGroup | filter, filterMask | filter);
}
void BasePhysicsComponent::RemoveCollisionFilter(CollisionMask filter)
{
	CollisionMask filterGroup;
	CollisionMask filterMask;
	GetCollisionFilter(&filterGroup, &filterMask);
	SetCollisionFilter(filterGroup & ~filter, filterMask & ~filter);
}
void BasePhysicsComponent::SetCollisionFilterMask(CollisionMask filterMask) { SetCollisionFilter(GetCollisionFilter(), filterMask); }
void BasePhysicsComponent::SetCollisionFilterGroup(CollisionMask filterGroup) { SetCollisionFilter(filterGroup, GetCollisionFilterMask()); }
void BasePhysicsComponent::SetCollisionFilter(CollisionMask filterGroup) { SetCollisionFilter(filterGroup, filterGroup); }
CollisionMask BasePhysicsComponent::GetCollisionFilter() const { return m_collisionFilterGroup; }
CollisionMask BasePhysicsComponent::GetCollisionFilterMask() const { return m_collisionFilterMask; }
void BasePhysicsComponent::GetCollisionFilter(CollisionMask *filterGroup, CollisionMask *filterMask) const
{
	*filterGroup = m_collisionFilterGroup;
	*filterMask = m_collisionFilterMask;
}
float BasePhysicsComponent::GetCollisionRadius(Vector3 *center) const
{
	if(center != NULL)
		*center = GetCollisionCenter();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(!pTrComponent)
		return m_colRadius;
	auto &scale = pTrComponent->GetScale();
	return m_colRadius * umath::abs_max(scale.x, scale.y, scale.z);
}

void BasePhysicsComponent::SetRayResultCallbackEnabled(bool b) { m_bRayResultCallbackEnabled = b; }
bool BasePhysicsComponent::IsRayResultCallbackEnabled() const { return m_bRayResultCallbackEnabled; }

bool BasePhysicsComponent::RayResultCallback(CollisionMask rayCollisionGroup, CollisionMask rayCollisionMask)
{
	CEHandleRaycast evData {rayCollisionGroup, rayCollisionMask};
	InvokeEventCallbacks(EVENT_HANDLE_RAYCAST, evData);
	return evData.hit;
}

PHYSICSTYPE BasePhysicsComponent::GetPhysicsType() const { return m_physicsType; }

COLLISIONTYPE BasePhysicsComponent::GetCollisionType() const { return m_collisionType; }

void BasePhysicsComponent::SetCollisionType(COLLISIONTYPE collisiontype) { m_collisionType = collisiontype; }

MOVETYPE BasePhysicsComponent::GetMoveType() const { return m_moveType; }

void BasePhysicsComponent::SetMoveType(MOVETYPE movetype) { m_moveType = movetype; }
bool BasePhysicsComponent::IsOnGround() const
{
	PhysObj *phys = GetPhysicsObject();
	if(phys == NULL || !phys->IsController())
		return false;
	ControllerPhysObj *physController = static_cast<ControllerPhysObj *>(phys);
	return physController->IsOnGround();
}
bool BasePhysicsComponent::IsGroundWalkable() const
{
	PhysObj *phys = GetPhysicsObject();
	if(phys == NULL || !phys->IsController())
		return false;
	ControllerPhysObj *physController = static_cast<ControllerPhysObj *>(phys);
	return physController->IsGroundWalkable();
}

const Vector3 &BasePhysicsComponent::GetLocalOrigin() const
{
	auto physType = GetPhysicsType();
	auto *phys = GetPhysicsObject();
	if(phys == nullptr || (physType != PHYSICSTYPE::DYNAMIC && physType != PHYSICSTYPE::STATIC))
		return uvec::ORIGIN;
	auto *o = phys->GetCollisionObject();
	if(o == nullptr)
		return uvec::ORIGIN;
	return o->GetOrigin();
}

Vector3 BasePhysicsComponent::GetOrigin() const
{
	auto physType = GetPhysicsType();
	auto *phys = GetPhysicsObject();
	if(phys == nullptr || (physType != PHYSICSTYPE::DYNAMIC && physType != PHYSICSTYPE::STATIC)) {
		auto pTrComponent = GetEntity().GetTransformComponent();
		return pTrComponent ? pTrComponent->GetPosition() : Vector3 {};
	}
	return phys->GetOrigin();
}

void BasePhysicsComponent::GetCollisionBounds(Vector3 *min, Vector3 *max) const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto scale = pTrComponent ? pTrComponent->GetScale() : Vector3 {1.f, 1.f, 1.f};
	*min = m_colMin * scale;
	*max = m_colMax * scale;
}

void BasePhysicsComponent::SetCollisionBounds(const Vector3 &min, const Vector3 &max)
{
	if(min.x != m_colMin.x || min.y != m_colMin.y || min.z != m_colMin.z || max.x != m_colMax.x || max.y != m_colMax.y || max.z != m_colMax.z)
		GetEntity().SetStateFlag(BaseEntity::StateFlags::CollisionBoundsChanged);
	m_colMin = min;
	m_colMax = max;
	auto extents = (max - min) * 0.5f;
	m_colRadius = glm::length(extents);
	if(m_physObject != nullptr && m_physObject->IsController()) {
		auto *phys = static_cast<ControllerPhysObj *>(m_physObject.get());
		phys->SetCollisionBounds(min, max);
	}
}

void BasePhysicsComponent::GetRotatedCollisionBounds(Vector3 *min, Vector3 *max) const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	bounding_volume::AABB::GetRotatedBounds(m_colMin, m_colMax, pTrComponent ? pTrComponent->GetRotationMatrix() : umat::identity(), min, max);
}

BasePhysicsComponent::StateFlags BasePhysicsComponent::GetStateFlags() const { return m_stateFlags; }

Vector3 BasePhysicsComponent::GetCollisionExtents() const
{
	auto r = (m_colMax - m_colMin) * 0.5f;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent)
		r *= pTrComponent->GetScale();
	return r;
}

Vector3 BasePhysicsComponent::GetCollisionCenter() const
{
	auto r = m_colMin + (m_colMax - m_colMin) * 0.5f;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent)
		r *= pTrComponent->GetScale();
	return r;
}

void BasePhysicsComponent::PhysicsUpdate(double tDelta)
{
	PhysObj *phys = GetPhysicsObject();
	CEPhysicsUpdateData evData {tDelta};
	MOVETYPE movetype = GetMoveType();
	if(phys != NULL && m_physObject->IsStatic() == false) {
		m_physObject->Simulate(tDelta, (movetype != MOVETYPE::WALK && movetype != MOVETYPE::PHYSICS) ? true : false);
		InvokeEventCallbacks(EVENT_ON_DYNAMIC_PHYSICS_UPDATED, evData);
	}
	InvokeEventCallbacks(EVENT_ON_PHYSICS_UPDATED, evData);
}

void BasePhysicsComponent::PrePhysicsSimulate()
{
	PhysObj *phys = GetPhysicsObject();
	InvokeEventCallbacks(EVENT_ON_PRE_PHYSICS_SIMULATE);
	if(phys == NULL || phys->IsStatic())
		return;
	dynamic_cast<PhysObjDynamic *>(phys)->PreSimulate();
}

static void entity_space_to_bone_space(std::vector<umath::ScaledTransform> &transforms, panima::Bone &bone, Vector3 &pos, Quat &rot, Bool bSkip = true)
{
	auto parent = bone.parent.lock();
	if(parent != nullptr)
		entity_space_to_bone_space(transforms, *parent, pos, rot, false);
	if(bSkip == false) {
		auto &t = transforms[bone.ID];
		auto &posBone = t.GetOrigin();
		auto &rotBone = t.GetRotation();
		pos -= posBone;
		auto inv = uquat::get_inverse(rotBone);
		uvec::rotate(&pos, inv);
		rot = inv * rot;
	}
}
pragma::physics::ICollisionObject *BasePhysicsComponent::GetCollisionObject(UInt32 boneId) const
{
	if(m_physObject == nullptr)
		return nullptr;
	auto &objs = m_physObject->GetCollisionObjects();
	for(auto it = objs.begin(); it != objs.end(); ++it) // TODO: Implement this properly
	{
		auto &o = *it;
		if(o.IsValid()) {
			if(o->GetBoneID() == boneId)
				return const_cast<pragma::physics::ICollisionObject *>(o.Get());
		}
	}
	return nullptr;
}

std::vector<BasePhysicsComponent::PhysJoint> &BasePhysicsComponent::GetPhysConstraints() { return m_joints; }

void BasePhysicsComponent::UpdatePhysicsBone(Frame &reference, const std::shared_ptr<panima::Bone> &bone, Quat &invRot, const Vector3 *)
{
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	if(animComponent.expired())
		return;
	auto boneId = bone->ID;
	auto *o = GetCollisionObject(boneId);
	if(o == nullptr)
		return;
	auto *posRef = reference.GetBonePosition(boneId);
	auto *rotRef = reference.GetBoneOrientation(boneId);
	if(posRef == nullptr || rotRef == nullptr)
		return;
	/*auto &origin = o->GetOrigin();
	auto rotConstraint = invRot *o->GetRotation() *(*rotRef);

	auto offset = *posRef +origin;
	uvec::rotate(&offset,o->GetRotation());
	auto posConstraint = o->GetPos() -GetPosition() +offset;

	auto localOffset = posConstraint;
	auto localRot = rotConstraint;

	uvec::rotate(&localOffset,invRot);
	if(mvOffset != nullptr)
		localOffset -= *mvOffset;*/ // Deprecated?
	auto boneOffset = o->GetOrigin() + (*posRef);
	uvec::rotate(&boneOffset, o->GetRotation());
	auto boneWorldPos = o->GetPos() + boneOffset;
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent)
		uvec::world_to_local(GetOrigin(), pTrComponent->GetRotation(), boneWorldPos);
	auto localOffset = boneWorldPos;

	auto rotConstraint = invRot * o->GetRotation() * (*rotRef);
	auto localRot = rotConstraint;
	entity_space_to_bone_space(animComponent->GetBoneTransforms(), *bone, localOffset, localRot);
	animComponent->SetBonePosition(boneId, localOffset, localRot, nullptr, false);
}

void BasePhysicsComponent::PostPhysicsSimulate(Frame &reference, std::unordered_map<uint32_t, std::shared_ptr<panima::Bone>> &bones, Vector3 &moveOffset, Quat &invRot, UInt32 physRootBoneId)
{
	// Linear iteration; Causes jittering, depending on how far the physics object's bone is down the skeleton hierarchy
	/*auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	auto mdl = GetModel();
	if(!mdl.IsValid())
		return;
	auto &skeleton = *mdl->GetSkeleton();
	auto &colObjs = phys->GetCollisionObjects();
	for(auto &hColObj : colObjs)
	{
		if(hColObj.IsValid())
		{
			auto *o = hColObj.get();
			auto boneId = o->GetBoneID();
			if(boneId != physRootBoneId)
			{
				auto *bone = skeleton.GetBone(boneId);
				auto *posRef = reference.GetBonePosition(boneId);
				auto *rotRef = reference.GetBoneOrientation(boneId);
				if(posRef != nullptr && rotRef != nullptr)
				{
					auto posOffset = -*posRef +o->GetOrigin();
					auto rotConstraint = invRot *o->GetRotation() *(*rotRef);

					auto offset = *posRef +o->GetOrigin();
					uvec::rotate(&offset,o->GetRotation());
					auto posConstraint = o->GetPos() -GetPosition() +offset;

					auto localOffset = posConstraint;
					auto localRot = rotConstraint;

					uvec::rotate(&localOffset,invRot);
					localOffset -= moveOffset;
					entity_space_to_bone_space(m_bones,*bone,localOffset,localRot);
					SetBonePosition(boneId,localOffset,localRot,false);
				}
			}
		}
	}*/
	//
	// Hierarchical Iteration; Slower, but no jiterring
	for(auto it = bones.begin(); it != bones.end(); ++it) {
		auto &bone = it->second;
		//auto boneId = it->first;
		//if(boneId != physRootBoneId)
		UpdatePhysicsBone(reference, bone, invRot, &moveOffset);
		PostPhysicsSimulate(reference, bone->children, moveOffset, invRot, physRootBoneId);
	}
	//
}

float BasePhysicsComponent::GetPhysicsMass() const
{
	auto &hMdl = GetEntity().GetModel();
	return (hMdl != nullptr) ? hMdl->GetMass() : 0.f;
}

void BasePhysicsComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["moveType"] = m_moveType;
	udm["collisionType"] = m_collisionType;
}
void BasePhysicsComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	auto moveType = GetMoveType();
	udm["moveType"](moveType);
	SetMoveType(moveType);

	auto collisionType = GetCollisionType();
	udm["collisionType"](collisionType);
	SetCollisionType(collisionType);
}

void BasePhysicsComponent::SetSleepReportEnabled(bool reportEnabled)
{
	umath::set_flag(m_stateFlags, StateFlags::SleepReportEnabled, reportEnabled);
	auto *physObj = GetPhysicsObject();
	if(physObj == nullptr)
		return;
	for(auto &hCol : physObj->GetCollisionObjects()) {
		if(hCol.IsValid() == false)
			continue;
		hCol->SetSleepReportEnabled(reportEnabled);
	}
}
bool BasePhysicsComponent::IsSleepReportEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::SleepReportEnabled); }
void BasePhysicsComponent::OnWake() { InvokeEventCallbacks(EVENT_ON_WAKE); }
void BasePhysicsComponent::OnSleep() { InvokeEventCallbacks(EVENT_ON_SLEEP); }

bool BasePhysicsComponent::IsRagdoll() const { return umath::is_flag_set(m_stateFlags, StateFlags::Ragdoll); }

void BasePhysicsComponent::SetForcePhysicsAwakeCallbacksEnabled(bool enabled, bool apply, std::optional<bool> isAwakeOverride)
{
	umath::set_flag(m_stateFlags, StateFlags::ForcePhysicsAwakeCallbacksEnabled, enabled);
	if(apply == false)
		return;
	if(enabled) {
		if(!m_physObject->IsSleeping() || (isAwakeOverride.has_value() && *isAwakeOverride))
			OnPhysicsWake(m_physObject.get());
	}
	else if(m_physObject->IsSleeping() || (isAwakeOverride.has_value() && !*isAwakeOverride))
		OnPhysicsSleep(m_physObject.get());
}
bool BasePhysicsComponent::AreForcePhysicsAwakeCallbacksEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::ForcePhysicsAwakeCallbacksEnabled); }

void BasePhysicsComponent::UpdateRagdollPose()
{
	auto &ent = GetEntity();
	auto animatedComponent = ent.GetAnimatedComponent();
	auto mdlComponent = ent.GetModelComponent();
	if(!mdlComponent || animatedComponent.expired() || IsRagdoll() == false)
		return;
	auto *phys = GetPhysicsObject();
	if(phys == nullptr || phys->IsStatic() == true)
		return;
	auto mdl = mdlComponent->GetModel();
	if(mdl == nullptr)
		return;
	auto &reference = mdl->GetReference();
	auto &skeleton = mdl->GetSkeleton();
	auto &rootBones = skeleton.GetRootBones();

	auto *physRoot = phys->GetCollisionObject();
	if(physRoot == nullptr)
		return;
	auto physRootBoneId = physRoot->GetBoneID();
	auto pTrComponent = ent.GetTransformComponent();
	auto invRot = pTrComponent ? uquat::get_inverse(pTrComponent->GetRotation()) : uquat::identity();

	auto physRootBone = skeleton.GetBone(physRootBoneId).lock();
	if(physRootBone != nullptr)
		UpdatePhysicsBone(reference, physRootBone, invRot);

	Vector3 posRoot;
	animatedComponent->GetLocalBonePosition(physRootBoneId, posRoot);

	auto moveOffset = -posRoot;
	PostPhysicsSimulate(reference, rootBones, moveOffset, invRot, physRootBoneId);
}

bool BasePhysicsComponent::PostPhysicsSimulate()
{
	PhysObj *phys = GetPhysicsObject();
	CEPostPhysicsSimulate evData {};
	InvokeEventCallbacks(EVENT_ON_POST_PHYSICS_SIMULATE, evData);
	if(phys == NULL || phys->IsStatic())
		return evData.keepAwake;
	dynamic_cast<PhysObjDynamic *>(phys)->PostSimulate();
	UpdateRagdollPose();
	return evData.keepAwake;
}
#if PHYS_KEEP_SIMULATION_TRANSFORM != 0
Vector3 BasePhysicsComponent::GetPhysicsSimulationOffset()
{
	PhysObj *phys = GetPhysicsObject();
	if(phys == NULL || phys->IsStatic())
		return Vector3(0, 0, 0);
	return dynamic_cast<PhysObjDynamic *>(phys)->GetSimulationOffset();
}
Quat BasePhysicsComponent::GetPhysicsSimulationRotation()
{
	PhysObj *phys = GetPhysicsObject();
	if(phys == NULL || phys->IsStatic())
		return uquat::identity();
	return dynamic_cast<PhysObjDynamic *>(phys)->GetSimulationRotation();
}
#endif

void BasePhysicsComponent::DropToFloor()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(!pTrComponent)
		return;
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto origin = ent.GetCenter();
	auto extents = GetCollisionExtents();
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return;
	auto shape = physEnv->CreateBoxShape(extents, physEnv->GetGenericMaterial()); // TODO: Cache this shape?
	auto pGravity = ent.GetComponent<pragma::GravityComponent>();
	auto dir = pGravity.valid() ? pGravity->GetGravityDirection() : -uvec::UP;
	auto dest = origin + dir * static_cast<float>(GameLimits::MaxRayCastRange);

	TraceData trace;
	trace.SetFilter(GetEntity());
	trace.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
	trace.SetSource(origin);
	trace.SetShape(*shape);
	trace.SetTarget(dest);
	auto result = game->Sweep(trace);
	if(result.hitType == RayCastHitType::None || result.distance == 0.f)
		return;
	auto pos = pTrComponent->GetPosition();
	auto rot = uvec::get_rotation(uvec::UP, -dir);
	uquat::normalize(rot);

	uvec::rotate(&pos, rot);
	uvec::rotate(&result.position, rot);
	pos.y = result.position.y;
	uvec::rotate(&pos, uquat::get_inverse(rot));
	pTrComponent->SetPosition(pos);
}

void BasePhysicsComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(pragma::VelocityComponent)) {
		FlagCallbackForRemoval(static_cast<pragma::VelocityComponent &>(component).GetVelocityProperty()->AddCallback([this](std::reference_wrapper<const Vector3> oldVel, std::reference_wrapper<const Vector3> vel) {
			if(umath::is_flag_set(m_stateFlags, StateFlags::ApplyingLinearVelocity))
				return;
			if(m_physObject != NULL)
				m_physObject->SetLinearVelocity(vel);
		}),
		  CallbackType::Component, &component);
		FlagCallbackForRemoval(static_cast<pragma::VelocityComponent &>(component).GetAngularVelocityProperty()->AddCallback([this](std::reference_wrapper<const Vector3> oldVel, std::reference_wrapper<const Vector3> vel) {
			if(umath::is_flag_set(m_stateFlags, StateFlags::ApplyingAngularVelocity))
				return;
			if(m_physObject != NULL)
				m_physObject->SetAngularVelocity(vel);
		}),
		  CallbackType::Component, &component);
	}
}
void BasePhysicsComponent::OriginToWorld(Vector3 *origin) const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	uvec::local_to_world(GetOrigin(), pTrComponent ? pTrComponent->GetRotation() : uquat::identity(), *origin);
}
void BasePhysicsComponent::OriginToWorld(Vector3 *origin, Quat *rot) const
{
	OriginToWorld(origin);
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent)
		pTrComponent->LocalToWorld(rot);
}

void BasePhysicsComponent::WorldToOrigin(Vector3 *origin) const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	uvec::world_to_local(GetOrigin(), pTrComponent ? pTrComponent->GetRotation() : uquat::identity(), *origin);
}
void BasePhysicsComponent::WorldToOrigin(Vector3 *origin, Quat *rot) const
{
	WorldToOrigin(origin);
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent)
		pTrComponent->WorldToLocal(rot);
}

void BasePhysicsComponent::UpdateBoneCollisionObject(UInt32 boneId, Bool updatePos, Bool updateRot)
{
	// TODO: This function might be deprecated?
	// Check if this works correctly!
	// If not, use code from animation to ragdoll transition from function BaseEntity::InitializeModelPhysics!
	auto *phys = GetPhysicsObject();
	if(phys == nullptr)
		return;
	auto &ent = GetEntity();
	auto animatedComponent = ent.GetAnimatedComponent();
	auto &hMdl = GetEntity().GetModel();
	if(animatedComponent.expired() || hMdl == nullptr)
		return;
	auto &reference = hMdl->GetReference();
	auto &objs = phys->GetCollisionObjects();
	auto *physRoot = phys->GetCollisionObject();
	if(physRoot == nullptr)
		return;
	auto physRootBoneId = physRoot->GetBoneID();
	if(physRootBoneId == boneId)
		return;
	auto posRef = reference.GetBonePosition(boneId);
	auto rotRef = reference.GetBoneOrientation(boneId);
	if(posRef == nullptr || rotRef == nullptr)
		return;
	Vector3 pos;
	Quat rot;
	animatedComponent->GetLocalBonePosition(boneId, pos, rot);
	rot *= uquat::get_inverse(*rotRef);
	Vector3 posRoot;
	animatedComponent->GetLocalBonePosition(physRootBoneId, posRoot);
	auto offsetRoot = -(physRoot->GetOrigin() * physRoot->GetRotation()) - posRoot;
	auto pTrComponent = ent.GetTransformComponent();
	for(auto it = objs.begin(); it != objs.end(); ++it) {
		auto &o = *it;
		if(o.IsValid() && o->GetBoneID() == boneId) {
			Vector3 oPos = pos;
			Quat oRot = rot;

			if(updatePos == true) {
				auto offset = *posRef + o->GetOrigin();
				uvec::rotate(&offset, oRot);
				oPos += -offset + offsetRoot;
				if(updateRot == true) {
					if(pTrComponent)
						pTrComponent->LocalToWorld(&oPos, &oRot);
					o->SetRotation(oRot);
				}
				else if(pTrComponent)
					pTrComponent->LocalToWorld(&oPos);
				o->SetPos(oPos);
			}
			else {
				if(pTrComponent)
					pTrComponent->LocalToWorld(&oRot);
				o->SetRotation(oRot);
			}
		}
	}
}

///////////////

CEPhysicsUpdateData::CEPhysicsUpdateData(double dt) : deltaTime {dt} {}
void CEPhysicsUpdateData::PushArguments(lua_State *l) { Lua::PushNumber(l, deltaTime); }

///////////////

CEHandleRaycast::CEHandleRaycast(CollisionMask rayCollisionGroup, CollisionMask rayCollisionMask) : rayCollisionGroup {rayCollisionGroup}, rayCollisionMask {rayCollisionMask} {}
void CEHandleRaycast::PushArguments(lua_State *l) {}

///////////////

CEInitializePhysics::CEInitializePhysics(PHYSICSTYPE type, BasePhysicsComponent::PhysFlags flags) : physicsType {type}, flags {flags} {}
void CEInitializePhysics::PushArguments(lua_State *l)
{
	Lua::PushInt(l, umath::to_integral(physicsType));
	Lua::PushInt(l, umath::to_integral(flags));
}

///////////////

CEPostPhysicsSimulate::CEPostPhysicsSimulate() {}
void CEPostPhysicsSimulate::PushArguments(lua_State *l) {}
uint32_t CEPostPhysicsSimulate::GetReturnCount() { return 1; }
void CEPostPhysicsSimulate::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -1))
		keepAwake = Lua::CheckBool(l, -1);
}
