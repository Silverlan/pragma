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
#pragma optimize("",off)
ComponentEventId BaseTransformComponent::EVENT_ON_POSE_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseTransformComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_POSE_CHANGED = componentManager.RegisterEvent("ON_POSE_CHANGED",std::type_index(typeid(BaseTransformComponent)));
}
void BaseTransformComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,const std::function<ComponentMemberIndex(ComponentMemberInfo&&)> &registerMember)
{
	using T = BaseTransformComponent;

	using TPosition = Vector3;
	registerMember(create_component_member_info<
		T,TPosition,
		static_cast<void(T::*)(const TPosition&)>(&T::SetPosition),
		static_cast<const TPosition&(T::*)() const>(&T::GetPosition)
	>("position"));

	using TRotation = Quat;
	registerMember(create_component_member_info<
		T,TRotation,
		static_cast<void(T::*)(const TRotation&)>(&T::SetRotation),
		static_cast<const TRotation&(T::*)() const>(&T::GetRotation)
	>("rotation"));

	using TAngles = EulerAngles;
	registerMember(create_component_member_info<
		T,TAngles,
		static_cast<void(T::*)(const TAngles&)>(&T::SetAngles),
		static_cast<TAngles(T::*)() const>(&T::GetAngles)
	>("angles"));

	using TScale = Vector3;
	registerMember(create_component_member_info<
		T,TScale,
		static_cast<void(T::*)(const TScale&)>(&T::SetScale),
		static_cast<const TScale&(T::*)() const>(&T::GetScale)
	>("scale"));
}
BaseTransformComponent::BaseTransformComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void BaseTransformComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetScale = SetupNetEvent("set_scale");

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &mdl = static_cast<CEOnModelChanged&>(evData.get()).model;
		if(mdl.get() == nullptr)
		{
			SetEyeOffset({});
			return util::EventReply::Unhandled;
		}
		SetEyeOffset(mdl.get()->GetEyeOffset());
		return util::EventReply::Unhandled;
	});
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		/*if(ustring::compare(kvData.key,"origin",false))
			SetPosition(uvec::create(kvData.value));
		else */if(ustring::compare<std::string>(kvData.key,"angles",false))
		{
			EulerAngles ang;
			ustring::string_to_array<float,double>(kvData.value,&ang.p,atof,3);
			SetAngles(ang);
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
void BaseTransformComponent::OnPoseChanged(TransformChangeFlags changeFlags,bool updatePhysics)
{
	auto &ent = GetEntity();
	if(umath::is_flag_set(changeFlags,TransformChangeFlags::PositionChanged))
		ent.SetStateFlag(BaseEntity::StateFlags::PositionChanged);
	if(umath::is_flag_set(changeFlags,TransformChangeFlags::RotationChanged))
		ent.SetStateFlag(BaseEntity::StateFlags::RotationChanged);
	m_tLastMoved = ent.GetNetworkState()->GetGameState()->CurTime();
	if(updatePhysics)
	{
		auto pPhysComponent = ent.GetPhysicsComponent();
		auto *pPhys = pPhysComponent ? pPhysComponent->GetPhysicsObject() : nullptr;
		if(pPhys)
		{
			if(umath::is_flag_set(changeFlags,TransformChangeFlags::PositionChanged) && umath::is_flag_set(pPhysComponent->GetStateFlags(),BasePhysicsComponent::StateFlags::ApplyingPhysicsPosition) == false)
				pPhys->SetPosition(GetPosition());
			if(umath::is_flag_set(changeFlags,TransformChangeFlags::RotationChanged) && umath::is_flag_set(pPhysComponent->GetStateFlags(),BasePhysicsComponent::StateFlags::ApplyingPhysicsRotation) == false)
				pPhys->SetOrientation(GetRotation());
		}
	}
	InvokeEventCallbacks(EVENT_ON_POSE_CHANGED,CEOnPoseChanged{changeFlags});
}
void BaseTransformComponent::SetPose(const umath::ScaledTransform &pose)
{
	m_pose = pose;
	OnPoseChanged(TransformChangeFlags::PositionChanged | TransformChangeFlags::RotationChanged | TransformChangeFlags::ScaleChanged);
}
void BaseTransformComponent::SetPose(const umath::Transform &pose)
{
	m_pose.SetOrigin(pose.GetOrigin());
	m_pose.SetRotation(pose.GetRotation());
	OnPoseChanged(TransformChangeFlags::PositionChanged | TransformChangeFlags::RotationChanged);
}
const umath::ScaledTransform &BaseTransformComponent::GetPose() const {return m_pose;}

Vector3 BaseTransformComponent::GetEyePosition() const
{
	auto eyeOffset = GetEyeOffset();
	Vector3 forward,right,up;
	GetOrientation(&forward,&right,&up);
	auto eyePos = GetPosition();
	eyePos = eyePos +forward *eyeOffset.x +up *eyeOffset.y +right *eyeOffset.z;
	return eyePos;
}
Vector3 BaseTransformComponent::GetEyeOffset() const {return m_eyeOffset *GetScale();}
void BaseTransformComponent::SetEyeOffset(const Vector3 &offset)
{
	m_eyeOffset = offset;
}

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
float BaseTransformComponent::GetAbsMaxAxisScale() const {return umath::abs(GetMaxAxisScale());}
const Vector3 &BaseTransformComponent::GetScale() const {return m_pose.GetScale();}
void BaseTransformComponent::SetScale(float scale) {SetScale({scale,scale,scale});}
void BaseTransformComponent::SetScale(const Vector3 &scale)
{
	m_pose.SetScale(scale);
	OnPoseChanged(TransformChangeFlags::ScaleChanged);
}

float BaseTransformComponent::GetDistance(const Vector3 &p) const {return uvec::distance(GetPosition(),p);}
float BaseTransformComponent::GetDistance(const BaseEntity &ent) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return uvec::distance(GetPosition(),pTrComponent ? pTrComponent->GetPosition() : Vector3{});
}

void BaseTransformComponent::SetPosition(const Vector3 &pos,Bool bForceUpdate)
{
	auto &posCur = m_pose.GetOrigin();
	if(
		bForceUpdate == false &&
		fabsf(pos.x -posCur.x) <= ENT_EPSILON &&
		fabsf(pos.y -posCur.y) <= ENT_EPSILON &&
		fabsf(pos.z -posCur.z) <= ENT_EPSILON
	)
		return;
	auto &ent = GetEntity();
	if(std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z))
	{
		Con::cwar<<"WARNING: NaN position ("<<pos.x<<","<<pos.y<<","<<pos.z<<") for entity ";
		ent.print(Con::cout);
		Con::cwar<<"! Ignoring..."<<Con::endl;
		return;
	}
	if(std::isinf(pos.x) || std::isinf(pos.y) || std::isinf(pos.z))
	{
		Con::cwar<<"WARNING: inf position ("<<pos.x<<","<<pos.y<<","<<pos.z<<") for entity ";
		ent.print(Con::cout);
		Con::cwar<<"! Ignoring..."<<Con::endl;
		return;
	}
	m_pose.SetOrigin(pos);
	OnPoseChanged(TransformChangeFlags::PositionChanged);
	ent.MarkForSnapshot();
}

void BaseTransformComponent::SetPosition(const Vector3 &pos) {SetPosition(pos,false);}

const Vector3 &BaseTransformComponent::GetPosition() const {return m_pose.GetOrigin();}

void BaseTransformComponent::SetRotation(const Quat &q)
{
	auto &rotCur = GetRotation();
	if(
		fabsf(q.w -rotCur.w) <= ENT_EPSILON &&
		fabsf(q.x -rotCur.x) <= ENT_EPSILON &&
		fabsf(q.y -rotCur.y) <= ENT_EPSILON &&
		fabsf(q.z -rotCur.z) <= ENT_EPSILON
	)
		return;
	auto &ent = GetEntity();
	if(std::isnan(q.w) || std::isnan(q.x) || std::isnan(q.y) || std::isnan(q.z))
	{
		Con::cwar<<"WARNING: NaN rotation ("<<q.w<<","<<q.x<<","<<q.y<<","<<q.z<<") for entity ";
		ent.print(Con::cout);
		Con::cwar<<"! Ignoring..."<<Con::endl;
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

void BaseTransformComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
	auto pose = m_pose;
	udm["pose"](pose);
	SetPose(pose);

	auto eyeOffset = m_eyeOffset;
	udm["eyeOffset"](eyeOffset);
	SetEyeOffset(eyeOffset);
}

const Quat &BaseTransformComponent::GetRotation() const {return m_pose.GetRotation();}
void BaseTransformComponent::SetAngles(const EulerAngles &ang) {SetRotation(uquat::create(ang));}

void BaseTransformComponent::SetPitch(float pitch)
{
	EulerAngles angles = GetAngles();
	angles.p = pitch;
	SetAngles(angles);
}
void BaseTransformComponent::SetYaw(float yaw)
{
	EulerAngles angles = GetAngles();
	angles.y = yaw;
	SetAngles(angles);
}
void BaseTransformComponent::SetRoll(float roll)
{
	EulerAngles angles = GetAngles();
	angles.r = roll;
	SetAngles(angles);
}

void BaseTransformComponent::LocalToWorld(Vector3 *origin) const {*origin = m_pose **origin;}
void BaseTransformComponent::LocalToWorld(Quat *rot) const {*rot = m_pose **rot;}
void BaseTransformComponent::LocalToWorld(Vector3 *origin,Quat *rot) const
{
	LocalToWorld(origin);
	LocalToWorld(rot);
}

void BaseTransformComponent::WorldToLocal(Vector3 *origin) const
{
	auto inv = m_pose.GetInverse();
	*origin = inv **origin;
}
void BaseTransformComponent::WorldToLocal(Quat *rot) const
{
	auto inv = m_pose.GetInverse();
	*rot = inv **rot;
}
void BaseTransformComponent::WorldToLocal(Vector3 *origin,Quat *rot) const
{
	WorldToLocal(origin);
	WorldToLocal(rot);
}

EulerAngles BaseTransformComponent::GetAngles() const {return EulerAngles(m_pose.GetRotation());}
float BaseTransformComponent::GetPitch() const {return GetAngles().p;}
float BaseTransformComponent::GetYaw() const {return GetAngles().y;}
float BaseTransformComponent::GetRoll() const {return GetAngles().r;}
Vector3 BaseTransformComponent::GetForward() const {return uquat::forward(m_pose.GetRotation());}
Vector3 BaseTransformComponent::GetUp() const {return uquat::up(m_pose.GetRotation());}
Vector3 BaseTransformComponent::GetRight() const {return uquat::right(m_pose.GetRotation());}
void BaseTransformComponent::GetOrientation(Vector3 *forward,Vector3 *right,Vector3 *up) const {uquat::get_orientation(m_pose.GetRotation(),forward,right,up);}
Mat4 BaseTransformComponent::GetRotationMatrix() const {return umat::create(m_pose.GetRotation());}

void BaseTransformComponent::UpdateLastMovedTime()
{
	m_tLastMoved = GetEntity().GetNetworkState()->GetGameState()->CurTime();
}

void BaseTransformComponent::SetRawPosition(const Vector3 &pos) {m_pose.SetOrigin(pos);}
void BaseTransformComponent::SetRawRotation(const Quat &rot) {m_pose.SetRotation(rot);}
void BaseTransformComponent::SetRawScale(const Vector3 &scale) {m_pose.SetScale(scale);}

Vector3 BaseTransformComponent::GetDirection(const BaseEntity &ent,bool bIgnoreYAxis) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return GetDirection(pTrComponent ? ent.GetCenter() : Vector3{},bIgnoreYAxis);
}
EulerAngles BaseTransformComponent::GetAngles(const BaseEntity &ent,bool bIgnoreYAxis) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return GetAngles(pTrComponent ? ent.GetCenter() : Vector3{},bIgnoreYAxis);
}
float BaseTransformComponent::GetDotProduct(const BaseEntity &ent,bool bIgnoreYAxis) const
{
	auto pTrComponent = ent.GetTransformComponent();
	return GetDotProduct(pTrComponent ? ent.GetCenter() : Vector3{},bIgnoreYAxis);
}
Vector3 BaseTransformComponent::GetDirection(const Vector3 &pos,bool bIgnoreYAxis) const
{
	auto &origin = GetPosition();
	auto dir = pos -origin;
	if(bIgnoreYAxis == true)
		dir.y = 0.f;
	uvec::normalize(&dir);
	return dir;
}
EulerAngles BaseTransformComponent::GetAngles(const Vector3 &pos,bool bIgnoreYAxis) const {return uvec::to_angle(GetDirection(pos,bIgnoreYAxis),GetUp());}
float BaseTransformComponent::GetDotProduct(const Vector3 &pos,bool bIgnoreYAxis) const {return uvec::dot(GetForward(),GetDirection(pos,bIgnoreYAxis));}

Vector3 BaseTransformComponent::GetOrigin() const
{
	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	return pPhysComponent ? pPhysComponent->GetOrigin() : GetPosition();
}

double BaseTransformComponent::GetLastMoveTime() const {return m_tLastMoved;}

/////////////////

CEOnPoseChanged::CEOnPoseChanged(TransformChangeFlags changeFlags)
	: changeFlags{changeFlags}
{}
void CEOnPoseChanged::PushArguments(lua_State *l)
{
	Lua::PushInt(l,umath::to_integral(changeFlags));
}

/////////////////

TraceData util::get_entity_trace_data(BaseTransformComponent &component)
{
	auto &origin = component.GetPosition();
	auto dir = component.GetForward();
	TraceData trData;
	trData.SetSource(origin);
	trData.SetTarget(origin +dir *static_cast<float>(GameLimits::MaxRayCastRange));
	trData.SetFilter(component.GetEntity());
	trData.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
	auto pPhysComponent = component.GetEntity().GetPhysicsComponent();
	if(pPhysComponent)
	{
		trData.SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter());
		trData.SetCollisionFilterMask(pPhysComponent->GetCollisionFilterMask() &~CollisionMask::Trigger &~CollisionMask::Water &~CollisionMask::WaterSurface);
	}
	return trData;
}
#pragma optimize("",on)
