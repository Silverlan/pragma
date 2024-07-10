/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/game/game_limits.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_observable_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/components/component_member_flags.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/lua/luacallback.h"
#include "pragma/lua/luafunction_call.h"
#include "pragma/model/model.h"
#include <sharedutils/datastream.h>
#include "pragma/physics/raytraces.h"
#include "pragma/entities/baseentity_trace.hpp"
#include <udm.hpp>

using namespace pragma;

ComponentEventId BaseTransformComponent::EVENT_ON_POSE_CHANGED = pragma::INVALID_COMPONENT_ID;
ComponentEventId BaseTransformComponent::EVENT_ON_TELEPORT = pragma::INVALID_COMPONENT_ID;
void BaseTransformComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_POSE_CHANGED = registerEvent("ON_POSE_CHANGED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_TELEPORT = registerEvent("ON_TELEPORT", ComponentEventInfo::Type::Broadcast);
}
void BaseTransformComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseTransformComponent;

	auto poseMetaData = std::make_shared<ents::PoseTypeMetaData>();
	poseMetaData->posProperty = "position";
	poseMetaData->rotProperty = "rotation";
	poseMetaData->scaleProperty = "scale";

	using TPose = umath::ScaledTransform;
	constexpr auto *posePathName = "pose";
	auto memberInfoPose = create_component_member_info<T, TPose, static_cast<void (T::*)(const TPose &)>(&T::SetPose), static_cast<const TPose &(T::*)() const>(&T::GetPose)>(posePathName, TPose {});
	memberInfoPose.SetFlag(pragma::ComponentMemberFlags::HideInInterface);
	memberInfoPose.AddTypeMetaData(poseMetaData);
	registerMember(std::move(memberInfoPose));

	auto poseComponentMetaData = std::make_shared<ents::PoseComponentTypeMetaData>();
	poseComponentMetaData->poseProperty = posePathName;

	using TPosition = Vector3;
	auto memberInfoPos = create_component_member_info<T, TPosition, static_cast<void (T::*)(const TPosition &)>(&T::SetPosition), static_cast<const TPosition &(T::*)() const>(&T::GetPosition)>("position", TPosition {});
	memberInfoPos.AddTypeMetaData(poseComponentMetaData);
	registerMember(std::move(memberInfoPos));

	using TRotation = Quat;
	auto memberInfoRot = create_component_member_info<T, TRotation, static_cast<void (T::*)(const TRotation &)>(&T::SetRotation), static_cast<const TRotation &(T::*)() const>(&T::GetRotation)>("rotation", uquat::identity());
	memberInfoRot.AddTypeMetaData(poseComponentMetaData);
	registerMember(std::move(memberInfoRot));

	using TAngles = EulerAngles;
	auto memberInfoAng = create_component_member_info<T, TAngles, static_cast<void (T::*)(const TAngles &)>(&T::SetAngles), static_cast<TAngles (T::*)() const>(&T::GetAngles)>("angles", TAngles {});
	memberInfoAng.AddTypeMetaData(poseComponentMetaData);
	registerMember(std::move(memberInfoAng));

	using TScale = Vector3;
	auto memberInfoScale = create_component_member_info<T, TScale, static_cast<void (T::*)(const TScale &)>(&T::SetScale), static_cast<const TScale &(T::*)() const>(&T::GetScale)>("scale", TScale {1.f, 1.f, 1.f});
	memberInfoScale.AddTypeMetaData(poseComponentMetaData);
	registerMember(std::move(memberInfoScale));
}
BaseTransformComponent::BaseTransformComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseTransformComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetScale = SetupNetEvent("set_scale");

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &mdl = static_cast<CEOnModelChanged &>(evData.get()).model;
		if(mdl.get() == nullptr) {
			SetEyeOffset({});
			return util::EventReply::Unhandled;
		}
		SetEyeOffset(mdl.get()->GetEyeOffset());
		return util::EventReply::Unhandled;
	});
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		/*if(ustring::compare(kvData.key,"origin",false))
			SetPosition(uvec::create(kvData.value));
		else */
		if(ustring::compare<std::string>(kvData.key, "angles", false)) {
			EulerAngles ang;
			ustring::string_to_array<float, double>(kvData.value, &ang.p, atof, 3);
			SetAngles(ang);
		}
		else if(ustring::compare<std::string>(kvData.key, "scale", false)) {
			Vector3 scale {1.f, 1.f, 1.f};
			auto n = ustring::string_to_array<float, double>(kvData.value, &scale.x, atof, 3);
			if(n == 1) {
				scale.y = scale.x;
				scale.z = scale.x;
			}
			SetScale(scale);
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
void BaseTransformComponent::Teleport(const umath::Transform &targetPose)
{
	umath::Transform curPose = GetPose();
	auto deltaPose = targetPose * curPose.GetInverse();
	CETeleport evData {curPose, targetPose, deltaPose};
	if(BroadcastEvent(EVENT_ON_TELEPORT, evData) == util::EventReply::Handled)
		return;
	SetPose(targetPose);
}
void BaseTransformComponent::OnPoseChanged(TransformChangeFlags changeFlags, bool updatePhysics)
{
	auto &ent = GetEntity();
	if(umath::is_flag_set(changeFlags, TransformChangeFlags::PositionChanged))
		ent.SetStateFlag(BaseEntity::StateFlags::PositionChanged);
	if(umath::is_flag_set(changeFlags, TransformChangeFlags::RotationChanged))
		ent.SetStateFlag(BaseEntity::StateFlags::RotationChanged);
	m_tLastMoved = ent.GetNetworkState()->GetGameState()->CurTime();
	if(updatePhysics) {
		auto pPhysComponent = ent.GetPhysicsComponent();
		auto *pPhys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
		if(pPhys) {
			if(umath::is_flag_set(changeFlags, TransformChangeFlags::PositionChanged) && umath::is_flag_set(pPhysComponent->GetStateFlags(), BasePhysicsComponent::StateFlags::ApplyingPhysicsPosition) == false)
				pPhys->SetPosition(GetPosition());
			if(umath::is_flag_set(changeFlags, TransformChangeFlags::RotationChanged) && umath::is_flag_set(pPhysComponent->GetStateFlags(), BasePhysicsComponent::StateFlags::ApplyingPhysicsRotation) == false)
				pPhys->SetOrientation(GetRotation());
		}
	}
	InvokeEventCallbacks(EVENT_ON_POSE_CHANGED, CEOnPoseChanged {changeFlags});
}
void BaseTransformComponent::SetPose(const umath::ScaledTransform &pose) { SetPose(pose, pragma::CoordinateSpace::World); }
void BaseTransformComponent::SetPose(const umath::Transform &pose) { SetPose(pose, pragma::CoordinateSpace::World); }
void BaseTransformComponent::SetPose(const umath::ScaledTransform &pose, pragma::CoordinateSpace space)
{
	if(space == pragma::CoordinateSpace::Local) {
		auto *parent = GetEntity().GetParent();
		if(parent) {
			SetPose(parent->GetPose() * pose);
			return;
		}
	}
	m_pose = pose;
	OnPoseChanged(TransformChangeFlags::PositionChanged | TransformChangeFlags::RotationChanged | TransformChangeFlags::ScaleChanged);
}
void BaseTransformComponent::SetPose(const umath::Transform &pose, pragma::CoordinateSpace space)
{
	if(space == pragma::CoordinateSpace::Local) {
		auto *parent = GetEntity().GetParent();
		if(parent) {
			SetPose(parent->GetPose() * pose);
			return;
		}
	}
	m_pose.SetOrigin(pose.GetOrigin());
	m_pose.SetRotation(pose.GetRotation());
	OnPoseChanged(TransformChangeFlags::PositionChanged | TransformChangeFlags::RotationChanged);
}
const umath::ScaledTransform &BaseTransformComponent::GetPose() const { return m_pose; }
umath::ScaledTransform BaseTransformComponent::GetPose(pragma::CoordinateSpace space) const
{
	if(space == pragma::CoordinateSpace::Local) {
		auto *parent = GetEntity().GetParent();
		if(parent)
			return parent->GetPose().GetInverse() * m_pose;
	}
	return m_pose;
}

Vector3 BaseTransformComponent::GetEyePosition() const
{
	auto eyeOffset = GetEyeOffset();
	Vector3 forward, right, up;
	GetOrientation(&forward, &right, &up);
	auto eyePos = GetPosition();
	eyePos = eyePos + forward * eyeOffset.x + up * eyeOffset.y + right * eyeOffset.z;
	return eyePos;
}
Vector3 BaseTransformComponent::GetEyeOffset() const { return m_eyeOffset * GetScale(); }
void BaseTransformComponent::SetEyeOffset(const Vector3 &offset) { m_eyeOffset = offset; }

float BaseTransformComponent::GetMaxAxisScale() const
{
	auto &scale = m_pose.GetScale();
	auto r = scale.x;
	if(umath::abs(scale.y) > umath::abs(r))
		r = scale.y;
	if(umath::abs(scale.z) > umath::abs(r))
		r = scale.z;
	return r;
}
float BaseTransformComponent::GetAbsMaxAxisScale() const { return umath::abs(GetMaxAxisScale()); }
Vector3 BaseTransformComponent::GetScale(pragma::CoordinateSpace space) const { return GetPose(space).GetScale(); }
const Vector3 &BaseTransformComponent::GetScale() const { return m_pose.GetScale(); }
void BaseTransformComponent::SetScale(float scale) { SetScale({scale, scale, scale}); }
void BaseTransformComponent::SetScale(float scale, pragma::CoordinateSpace space) { SetScale(Vector3 {scale, scale, scale}, space); }
void BaseTransformComponent::SetScale(const Vector3 &scale)
{
	m_pose.SetScale(scale);
	OnPoseChanged(TransformChangeFlags::ScaleChanged);
}
void BaseTransformComponent::SetScale(const Vector3 &scale, pragma::CoordinateSpace space)
{
	if(space == pragma::CoordinateSpace::Local) {
		auto *parent = GetEntity().GetParent();
		if(parent) {
			{
				umath::ScaledTransform worldPose {Vector3 {}, uquat::identity(), scale};
				worldPose = parent->GetPose() * worldPose;
				SetScale(worldPose.GetScale());
				return;
			}
		}
	}
}

float BaseTransformComponent::GetDistance(const Vector3 &p) const { return uvec::distance(GetPosition(), p); }
float BaseTransformComponent::GetDistance(const BaseEntity &ent) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return uvec::distance(GetPosition(), pTrComponent ? pTrComponent->GetPosition() : Vector3 {});
}

void BaseTransformComponent::SetPosition(const Vector3 &pos, Bool bForceUpdate, pragma::CoordinateSpace space)
{
	if(space == pragma::CoordinateSpace::Local) {
		auto *parent = GetEntity().GetParent();
		if(parent) {
			{
				SetPosition(parent->GetPose() * pos, bForceUpdate);
				return;
			}
		}
	}
	auto &posCur = m_pose.GetOrigin();
	if(bForceUpdate == false && fabsf(pos.x - posCur.x) <= ENT_EPSILON && fabsf(pos.y - posCur.y) <= ENT_EPSILON && fabsf(pos.z - posCur.z) <= ENT_EPSILON)
		return;
	auto &ent = GetEntity();
	if(std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z)) {
		Con::cwar << "NaN position (" << pos.x << "," << pos.y << "," << pos.z << ") for entity ";
		ent.print(Con::cout);
		Con::cwar << "! Ignoring..." << Con::endl;
		return;
	}
	if(std::isinf(pos.x) || std::isinf(pos.y) || std::isinf(pos.z)) {
		Con::cwar << "inf position (" << pos.x << "," << pos.y << "," << pos.z << ") for entity ";
		ent.print(Con::cout);
		Con::cwar << "! Ignoring..." << Con::endl;
		return;
	}
	m_pose.SetOrigin(pos);
	OnPoseChanged(TransformChangeFlags::PositionChanged);
	ent.MarkForSnapshot();
}

void BaseTransformComponent::SetPosition(const Vector3 &pos, pragma::CoordinateSpace space) { SetPosition(pos, false, space); }
void BaseTransformComponent::SetPosition(const Vector3 &pos) { SetPosition(pos, false); }

Vector3 BaseTransformComponent::GetPosition(pragma::CoordinateSpace space) const { return GetPose(space).GetOrigin(); }
const Vector3 &BaseTransformComponent::GetPosition() const { return m_pose.GetOrigin(); }

void BaseTransformComponent::SetRotation(const Quat &q) { SetRotation(q, pragma::CoordinateSpace::World); }
void BaseTransformComponent::SetRotation(const Quat &q, pragma::CoordinateSpace space)
{
	if(space == pragma::CoordinateSpace::Local) {
		auto *parent = GetEntity().GetParent();
		if(parent) {
			{
				SetRotation(parent->GetPose() * q);
				return;
			}
		}
	}
	auto &rotCur = GetRotation();
	if(fabsf(q.w - rotCur.w) <= ENT_EPSILON && fabsf(q.x - rotCur.x) <= ENT_EPSILON && fabsf(q.y - rotCur.y) <= ENT_EPSILON && fabsf(q.z - rotCur.z) <= ENT_EPSILON)
		return;
	auto &ent = GetEntity();
	if(std::isnan(q.w) || std::isnan(q.x) || std::isnan(q.y) || std::isnan(q.z)) {
		Con::cwar << "NaN rotation (" << q.w << "," << q.x << "," << q.y << "," << q.z << ") for entity ";
		ent.print(Con::cout);
		Con::cwar << "! Ignoring..." << Con::endl;
		return;
	}
	m_pose.SetRotation(q);
	OnPoseChanged(TransformChangeFlags::RotationChanged);
	ent.MarkForSnapshot();
}

void BaseTransformComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["pose"] = m_pose;
	udm["eyeOffset"] = m_eyeOffset;
}

void BaseTransformComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	auto pose = m_pose;
	udm["pose"](pose);
	SetPose(pose);

	auto eyeOffset = m_eyeOffset;
	udm["eyeOffset"](eyeOffset);
	SetEyeOffset(eyeOffset);
}

Quat BaseTransformComponent::GetRotation(pragma::CoordinateSpace space) const { return GetPose(space).GetRotation(); }
const Quat &BaseTransformComponent::GetRotation() const { return m_pose.GetRotation(); }
void BaseTransformComponent::SetAngles(const EulerAngles &ang) { SetAngles(ang, pragma::CoordinateSpace::World); }
void BaseTransformComponent::SetAngles(const EulerAngles &ang, pragma::CoordinateSpace space) { SetRotation(uquat::create(ang), space); }

void BaseTransformComponent::SetPitch(float pitch, pragma::CoordinateSpace space)
{
	EulerAngles angles = GetAngles(space);
	angles.p = pitch;
	SetAngles(angles, space);
}
void BaseTransformComponent::SetYaw(float yaw, pragma::CoordinateSpace space)
{
	EulerAngles angles = GetAngles(space);
	angles.y = yaw;
	SetAngles(angles, space);
}
void BaseTransformComponent::SetRoll(float roll, pragma::CoordinateSpace space)
{
	EulerAngles angles = GetAngles(space);
	angles.r = roll;
	SetAngles(angles, space);
}

void BaseTransformComponent::LocalToWorld(Vector3 *origin) const { *origin = m_pose * *origin; }
void BaseTransformComponent::LocalToWorld(Quat *rot) const { *rot = m_pose * *rot; }
void BaseTransformComponent::LocalToWorld(Vector3 *origin, Quat *rot) const
{
	LocalToWorld(origin);
	LocalToWorld(rot);
}

void BaseTransformComponent::WorldToLocal(Vector3 *origin) const
{
	auto inv = m_pose.GetInverse();
	*origin = inv * *origin;
}
void BaseTransformComponent::WorldToLocal(Quat *rot) const
{
	auto inv = m_pose.GetInverse();
	*rot = inv * *rot;
}
void BaseTransformComponent::WorldToLocal(Vector3 *origin, Quat *rot) const
{
	WorldToLocal(origin);
	WorldToLocal(rot);
}

EulerAngles BaseTransformComponent::GetAngles() const { return EulerAngles {GetRotation()}; }
EulerAngles BaseTransformComponent::GetAngles(pragma::CoordinateSpace space) const { return EulerAngles {GetRotation(space)}; }
float BaseTransformComponent::GetPitch(pragma::CoordinateSpace space) const { return GetAngles(space).p; }
float BaseTransformComponent::GetYaw(pragma::CoordinateSpace space) const { return GetAngles(space).y; }
float BaseTransformComponent::GetRoll(pragma::CoordinateSpace space) const { return GetAngles(space).r; }
Vector3 BaseTransformComponent::GetForward(pragma::CoordinateSpace space) const { return uquat::forward(GetRotation(space)); }
Vector3 BaseTransformComponent::GetUp(pragma::CoordinateSpace space) const { return uquat::up(GetRotation(space)); }
Vector3 BaseTransformComponent::GetRight(pragma::CoordinateSpace space) const { return uquat::right(GetRotation(space)); }
void BaseTransformComponent::GetOrientation(Vector3 *forward, Vector3 *right, Vector3 *up) const { uquat::get_orientation(m_pose.GetRotation(), forward, right, up); }
Mat4 BaseTransformComponent::GetRotationMatrix(pragma::CoordinateSpace space) const { return umat::create(GetRotation(space)); }

void BaseTransformComponent::UpdateLastMovedTime() { m_tLastMoved = GetEntity().GetNetworkState()->GetGameState()->CurTime(); }

void BaseTransformComponent::SetRawPosition(const Vector3 &pos, pragma::CoordinateSpace space)
{
	if(space == pragma::CoordinateSpace::Local) {
		auto *parent = GetEntity().GetParent();
		if(parent) {
			{
				SetRawPosition(parent->GetPose() * pos);
				return;
			}
		}
	}
	m_pose.SetOrigin(pos);
}
void BaseTransformComponent::SetRawRotation(const Quat &rot, pragma::CoordinateSpace space)
{
	if(space == pragma::CoordinateSpace::Local) {
		auto *parent = GetEntity().GetParent();
		if(parent) {
			{
				SetRawRotation(parent->GetPose() * rot);
				return;
			}
		}
	}
	m_pose.SetRotation(rot);
}
void BaseTransformComponent::SetRawScale(const Vector3 &scale, pragma::CoordinateSpace space)
{
	if(space == pragma::CoordinateSpace::Local) {
		auto *parent = GetEntity().GetParent();
		if(parent) {
			{
				umath::ScaledTransform worldPose {Vector3 {}, uquat::identity(), scale};
				worldPose = parent->GetPose() * worldPose;
				SetRawScale(worldPose.GetScale());
				return;
			}
		}
	}
	m_pose.SetScale(scale);
}

Vector3 BaseTransformComponent::GetDirection(const BaseEntity &ent, bool bIgnoreYAxis) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return GetDirection(pTrComponent ? ent.GetCenter() : Vector3 {}, bIgnoreYAxis);
}
EulerAngles BaseTransformComponent::GetAngles(const BaseEntity &ent, bool bIgnoreYAxis) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return GetAngles(pTrComponent ? ent.GetCenter() : Vector3 {}, bIgnoreYAxis);
}
float BaseTransformComponent::GetDotProduct(const BaseEntity &ent, bool bIgnoreYAxis) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return GetDotProduct(pTrComponent ? ent.GetCenter() : Vector3 {}, bIgnoreYAxis);
}
Vector3 BaseTransformComponent::GetDirection(const Vector3 &pos, bool bIgnoreYAxis) const
{
	auto &origin = GetPosition();
	auto dir = pos - origin;
	if(bIgnoreYAxis == true)
		dir.y = 0.f;
	uvec::normalize(&dir);
	return dir;
}
EulerAngles BaseTransformComponent::GetAngles(const Vector3 &pos, bool bIgnoreYAxis) const { return uvec::to_angle(GetDirection(pos, bIgnoreYAxis), GetUp()); }
float BaseTransformComponent::GetDotProduct(const Vector3 &pos, bool bIgnoreYAxis) const { return uvec::dot(GetForward(), GetDirection(pos, bIgnoreYAxis)); }

Vector3 BaseTransformComponent::GetOrigin() const
{
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	return pPhysComponent ? pPhysComponent->GetOrigin() : GetPosition();
}

double BaseTransformComponent::GetLastMoveTime() const { return m_tLastMoved; }

/////////////////

CEOnPoseChanged::CEOnPoseChanged(TransformChangeFlags changeFlags) : changeFlags {changeFlags} {}
void CEOnPoseChanged::PushArguments(lua_State *l) { Lua::PushInt(l, umath::to_integral(changeFlags)); }

/////////////////

TraceData util::get_entity_trace_data(BaseTransformComponent &component)
{
	auto &origin = component.GetPosition();
	auto dir = component.GetForward();
	TraceData trData;
	trData.SetSource(origin);
	trData.SetTarget(origin + dir * static_cast<float>(GameLimits::MaxRayCastRange));
	trData.SetFilter(component.GetEntity());
	trData.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
	auto pPhysComponent = component.GetEntity().GetPhysicsComponent();
	if(pPhysComponent) {
		trData.SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter());
		trData.SetCollisionFilterMask(pPhysComponent->GetCollisionFilterMask() & ~CollisionMask::Trigger & ~CollisionMask::Water & ~CollisionMask::WaterSurface);
	}
	return trData;
}

CETeleport::CETeleport(const umath::Transform &originalPose, const umath::Transform &targetPose, const umath::Transform &deltaPose) : originalPose {originalPose}, targetPose {targetPose}, deltaPose {deltaPose} {}
void CETeleport::PushArguments(lua_State *l)
{
	Lua::Push(l, originalPose);
	Lua::Push(l, targetPose);
	Lua::Push(l, deltaPose);
}
